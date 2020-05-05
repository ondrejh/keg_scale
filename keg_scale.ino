/**
 * keg_scale
 * 
 * todo:
 *   webi: calibration
 *   eeprom: save calibration ect.
 */

#include "HX711.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = D3;
const int LOADCELL_SCK_PIN = D4;

HX711 scale;

const int led = 16;

#define MESS 16
int32_t scale_mess[MESS];
uint32_t mess_ptr = 0;
int32_t scale_avg = 0;

// display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// wifi ap settings
const char *ssid = "Kegator";
const char *password = "";

ESP8266WebServer server(80);

void display_raw_scale() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  
  char buff[32];
  int blen;

  display.setTextSize(2);
  sprintf(buff, "%d", scale_avg);
  blen = strlen(buff);
  display.setCursor(64 - blen*6, 12);
  display.write(buff);
  display.display();  
}

void display_wifi() {
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  
  char buff[32];
  int blen;
  IPAddress ip = WiFi.softAPIP();
  sprintf(buff, "IP: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  blen = strlen(buff);
  display.setCursor(64 - blen*3, 20);     // Start at top-left corner
  display.write(buff);

  sprintf(buff, "SSID: %s", ssid);
  blen = strlen(buff);
  display.setCursor(64 - blen*3, 4);     // Start at top-left corner
  display.write(buff);  

  display.display();
}

void handleRoot() {
  digitalWrite(led, 1);

  char msg[1024];
  int p = 0;
  p += sprintf(&msg[p], "<!DOCTYPE html><html lang='cz'><head><meta charset='utf-8'><title>title</title></head><body>");
  p += sprintf(&msg[p], "<p id='raw'>%d</p>", scale_avg);
  p += sprintf(&msg[p], "</body></html>");

  server.send(200, "text/html", msg);

  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // show initial display buffer contents on the screen
  display.display();

  // start the access point
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
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // display wifi connection
  display_wifi();
  delay(2000);
}

void loop() {
  // http server
  server.handleClient();
  MDNS.update();
  
  // scale reading
  if (scale.is_ready()) {
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
  }

  // display
  
}
