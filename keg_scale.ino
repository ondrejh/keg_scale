/**
 * keg_scale
 * 
 * todo:
 *   interpolation test
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

// eeprom calibration data
#define EEPROM_CALIB_ADDR 0
#define CLBLEN 5
union {
  struct {
    uint8_t p;
    int32_t x[CLBLEN];
    float y[CLBLEN];
    uint8_t chsum;
  } calib;
  uint8_t d[sizeof(calib)];
} clb_eeprom;

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
  EEPROM.begin(512);
  /*uint8_t a = EEPROM.read(0);
  a += 1;
  EEPROM.write(0, a);
  EEPROM.commit();
  Serial.print(a);*/
  load_calib();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

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
      display_raw_scale();
    }
    else
      Serial.println("---");
    
    digitalWrite(SLED, OFF);
  }

  // display
  
}
