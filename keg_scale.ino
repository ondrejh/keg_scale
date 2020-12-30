/**
 * keg_scale v0.11 alpha
 * 
 * done:
 *   individual device and ssid name
 *   new webi using post
 * 
 * todo:
 *   connect to local wifi
 *   store config data
 */

#include "HX711.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <EEPROM.h>

#include <OneWire.h>

#include "webi.h"

OneWire  ds(D8);  // on pin 10 (a 4.7K resistor is necessary)

float temperature;
int16_t raw_temp = 0;
bool temperature_valid = false;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = D5;
const int LOADCELL_SCK_PIN = D6;

HX711 scale;

const int WLED = BUILTIN_LED;
const int SLED = 16;
#define ON 0
#define OFF 1

#define MESS 16
int32_t scale_mess[MESS];
uint32_t mess_ptr = 0;
int32_t scale_avg = 0;
float scale_units = 0.0;

#define EEPROM_SIZE 512

// calibration data
#define EEPROM_CALIB_ADDR 0
#define UNIT_STR_MAX 8
#define CLBLEN 5
typedef struct {
  uint8_t p; // number of calb point used
  int32_t x[CLBLEN]; // calibration points x (raw)
  float y[CLBLEN]; // calibration points y (primary unit)
  float us; // secondary units = primary units * us
  char uprim[UNIT_STR_MAX + 1]; // primary unit name
  char usec[UNIT_STR_MAX + 1]; // secondary unit name
} calib_t;

calib_t calib;

#define EEPROM_KEG_ADDR (2+sizeof(calib_t))
#define KEG_LABEL_MAX 32
#define KEG (keg.keg)
typedef struct {
  bool keg;
  char label[KEG_LABEL_MAX + 1];
  float volume;
  int32_t fullraw;
} keg_t;

keg_t keg;
float keg_left;
float keg_full;

#define SCALE_PERIOD 200 //ms

// display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// wifi ap settings
char ssid[16] = "keg123456";
const char *password = "k3Gat0rr";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(WLED, OUTPUT);
  digitalWrite(WLED, OFF);
  pinMode(SLED, OUTPUT);
  digitalWrite(SLED, OFF);

  pinMode(D7, OUTPUT);
  digitalWrite(D7, HIGH);

  // init eeprom data
  init_eeprom(EEPROM_SIZE);
  if (eeload(EEPROM_CALIB_ADDR, &calib, sizeof(calib)) < 0)
    set_calib_default(&calib);
  if (eeload(EEPROM_KEG_ADDR, &keg, sizeof(keg)) < 0)
    keg.keg = false;

  // if keg initialize live data
  if (KEG) {
    keg_full = interpolate(keg.fullraw);
    keg_left = keg.volume;
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))// Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));

  // show initial display buffer contents on the screen
  display.display();

  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(ssid, "keg%02X%02X%02X", mac[3], mac[4], mac[5]);

  // start the access point
  IPAddress ip(192, 168, 42, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, ip, subnet);
  WiFi.softAP(ssid, password);
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());

  // init scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  pinMode(LOADCELL_DOUT_PIN, INPUT);

  if (MDNS.begin("esp8266"))
    Serial.println("MDNS responder started");

  // mdns setup
  server.on("/", handleRoot);
  server.on(index_name, handleRoot);
  server.on(keg_name, handleKegstart);
  server.on(calib_name, handleCalibration);
  server.on(config_name, handleConfiguration);
  server.on(sass_style_name, handleStyle);
  server.on(favicon_name, handleFavicon);
  server.on("img/favicon.ico", handleFavicon);
  server.on(img_bg_name, handleBg);
  server.on(img_barell_name, handleBarell);
  
  server.on(jquery_name, handleJquery);
  server.on("/data.json", handleData);
  //server.on("/calib.php", HTTP_GET, handleCalib);
  //server.on("/keg.php", HTTP_GET, handleKeg);
  server.on("/do.php", HTTP_POST, handleDo);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // display wifi connection
  display_wifi();
}

void loop() {
  // http server
  server.handleClient();
  MDNS.update();
  
  // scale reading
  static uint32_t scale_tim = 0;
  uint32_t now = millis();
  if (((now - scale_tim) >= SCALE_PERIOD) && (scale.is_ready())) {
    scale_tim = now;
    digitalWrite(SLED, ON);

    int32_t reading = scale.read();
    if (reading != -1) {
      scale_avg -= scale_mess[mess_ptr];
      scale_mess[mess_ptr] = reading;
      scale_avg += scale_mess[mess_ptr++];
      mess_ptr %= MESS;
      Serial.println(scale_avg);
      scale_units = interpolate(scale_avg);
      if (KEG) {
        keg_left = keg.volume - (keg_full - scale_units) / calib.us;
        display_units(keg_left, calib.usec);
      }
      else
        display_units(scale_units, calib.uprim);
    }
    else
      Serial.println("---");
    
    digitalWrite(SLED, OFF);
  }

  // temperature
  static uint32_t temp_tim = 0;
  static int temp_state = 0;
  switch (temp_state) {
  case 0:
    temp_tim = millis();
    ds.reset();
    ds.skip();
    ds.write(0x44,1);
    temp_state ++;
    break;
  case 1:
    if ((millis() - temp_tim) >= 1000) {
      byte present = ds.reset();
      byte data[9];
      ds.skip();    
      ds.write(0xBE); // Read Scratchpad

      if (!present) {
        temp_state = 3;
        break;
      }
      else {
        for ( int i = 0; i < 9; i++)
          data[i] = ds.read();
        if (OneWire::crc8( data, 9) == 0) {
          static bool first = true;
          if (first) {
            temp_state = 3;
            first = false;
            break;
          }
          else {
            int16_t raw = (data[1] << 8) | data[0];
            byte cfg = (data[4] & 0x60);
            if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
            else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
            else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
            temperature = (float)raw / 16.0;
            temperature_valid = true;
            raw_temp = raw;
            Serial.println(temperature);
          }
        } else {
          temp_state = 3;
          break;
        }
      }
      temp_state++;
    }
    break;
  case 2:
    if ((millis() - temp_tim) >= 30000) {
      temp_state = 0;
    }
    break;
  case 3:
    if ((millis() - temp_tim) >= 1200) {
      temp_state = 0;
    }
    break;
  }

  // display

  
}
