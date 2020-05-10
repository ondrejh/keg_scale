/**
 * keg_scale
 * 
 * todo:
 *   keg info (name, volume, full_raw ...)
 */

#include "HX711.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <EEPROM.h>

#include "webi.h"

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

#define SCALE_PERIOD 200 //ms

// display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// wifi ap settings
const char *ssid = "Kegator1";
const char *password = "k3Gat0rr";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(WLED, OUTPUT);
  digitalWrite(WLED, OFF);
  pinMode(SLED, OUTPUT);
  digitalWrite(SLED, OFF);

  // init eeprom data
  init_eeprom(EEPROM_SIZE);
  if (eeload(EEPROM_CALIB_ADDR, &calib, sizeof(calib)) < 0)
    set_calib_default(&calib);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))// Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));

  // show initial display buffer contents on the screen
  display.display();

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

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on(index_name, handleRoot);
  server.on(jquery_name, handleJquery);
  server.on("/data.json", handleData);
  server.on("/calib.php", HTTP_GET, handleCalib);
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
      //display_raw_scale(stale_avg);
      display_units(scale_units, calib.uprim);
    }
    else
      Serial.println("---");
    
    digitalWrite(SLED, OFF);
  }

  // display
  
}
