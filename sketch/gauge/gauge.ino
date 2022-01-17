/*

*/


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LED_PIN D7
#define NUMPIXELS 2

#define WLED LED_BUILTIN
#define ON LOW
#define OFF HIGH

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define GAUGE_PIN D6
#define GAUGE_ZERO 400
#define GAUGE_ENABLE D5

ESP8266WiFiMulti WiFiMulti;

//#define WSSID "keg6D98FD"
//#define WSSID "keg60B784"
//#define WSSID "kegFCADEB"
//#define WSSID "kegE1BBF2"
#define WSSID "keg98A154"
#define WPWD "k3Gat0rr"

// display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// wifi ap settings
char ssid[16] = "disp123456";
const char *password = "k3Gat0rr";
bool AP_Mode = false;

ESP8266WebServer server(80);

void handleRoot() {
  digitalWrite(WLED, ON);
  char msg[1024];
  sprintf(msg, "<!DOCTYPE html><html lang='cz'><head><title>Kegator display</title></head><body>Ahoj Světe!</body></html>");
  server.send(200, "text/html", msg);
  digitalWrite(WLED, OFF);
}

void handleNotFound() {
  digitalWrite(WLED, ON);

  if (server.method() == HTTP_OPTIONS) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.send(204);
  } else {
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
    server.send(404, "text/plain", "");
  }
  digitalWrite(WLED, OFF);
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(GAUGE_ENABLE, OUTPUT);
  digitalWrite(GAUGE_ENABLE, HIGH);

  // backlight initialization
  pixels.begin();
  backlight_set_color(0, 0, 0);

  // gauge initialization
  gauge_zero();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))// Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));

  // show initial display buffer contents on the screen
  display.display();
  display_intro();

  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(ssid, "disp%02X%02X%02X", mac[3], mac[4], mac[5]);

  // wifi connect
  if (!AP_Mode) {
    AP_Mode = false;
    WiFi.mode(WIFI_STA);
    WiFi.begin(WSSID, WPWD);
    // Wait for connection
    Serial.print("Connecting WiFi ");
    int cnt = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      cnt ++;
      if (cnt > 30)
        break;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(" OK");
      Serial.println(WiFi.localIP());
    }
    else {
      Serial.println(" ERROR (timeout)");
      AP_Mode = true;
    }
  }
  if (AP_Mode) {
    // start the access point
    WiFi.mode(WIFI_AP);
    IPAddress ip(192, 168, 42, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(ip, ip, subnet);
    WiFi.softAP(ssid, password);
    Serial.print("Access Point \"");
    Serial.print(ssid);
    Serial.println("\" started");
    Serial.print("IP address:\t");
    Serial.println(WiFi.softAPIP());
  }
  
  if (MDNS.begin(ssid)) Serial.println("MDNS responder started");

  // mdns setup
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // display wifi connection
  if (AP_Mode) {
    IPAddress dip = WiFi.softAPIP();
    display_wifi(ssid, dip);
  }
}

// the loop function runs over and over again forever
void loop() {
  // http server
  server.handleClient();
  MDNS.update();

  static uint8_t r = 0, g = 0, b = 0;
  static uint32_t wget_timer = 1000;
  static int err_cnt = 0;

  static String temp;
  static bool temp_valid = false;
  static uint8_t dcnt = 0;

  static int gauge_setup = -1;
  static int gauge_setup_value = 0;
  if (Serial.available() > 0) {
    char b = Serial.read();
    bool prnt = false;
    if ((b >= '0') && (b <= '5')) {
      gauge_setup = b - '0';
      gauge_setup_value = get_gauge_steps(gauge_setup);
      prnt = true;
    }
    else if (gauge_setup >= 0) {
      if (b == '+') {
        gauge_setup_value ++;
        prnt = true;
      }
      else if (b == '-') {
        gauge_setup_value --;
        prnt = true;
      }
      else if (b == 'q') {
        gauge_setup = -1;
        Serial.print("End");
      }
      else if (b == 'w') {
        Serial.print("Write");
      }
    }
    if (prnt) {
      Serial.print(gauge_setup);
      Serial.print(": ");
      Serial.println(gauge_setup_value);
      backlight_set_color(255, 255, 255);
      gauge_set_pwm(gauge_setup_value);
    }
  }

  if ((millis() - wget_timer) > 1000) {
    if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(WLED, ON);
      WiFiClient client;
      HTTPClient http;
  
      Serial.println();
      if (http.begin(client, "http://192.168.42.1/data.json")) {
        int httpCode = http.GET();
        if (httpCode > 0) {
          if ((httpCode == HTTP_CODE_OK) || (httpCode == HTTP_CODE_MOVED_PERMANENTLY)) {
            float v;
            char u[16];
            String payload = http.getString();
            String prim = strip_string(find_key_simple(payload, "units"));
            String primu = strip_string(find_key_simple(payload, "primary_unit"));
            Serial.print(prim);
            Serial.print(" ");
            Serial.println(primu);
            if (key_exists(payload, "temp")) {
              temp = strip_string(find_key_simple(payload, "temp"));
              Serial.print(temp);
              temp_valid = true;
              Serial.println(" C");
            }
            else temp_valid = false;
            if (key_exists(payload, "keg")) {
              String kname = strip_string(find_key_simple(payload, "name"));
              String kvol = strip_string(find_key_simple(payload, "volume"));
              String kleft = strip_string(find_key_simple(payload, "left"));
              String secu = strip_string(find_key_simple(payload, "secondary_unit"));
              Serial.print("Keg: ");
              Serial.println(kname);
              Serial.print("Zbývá ");
              Serial.print(kleft);
              Serial.print(" ze ");
              Serial.print(kvol);
              Serial.print(" ");
              Serial.println(secu);
              float fvol = kvol.toFloat();
              float fleft = kleft.toFloat();
              float p;
              p = fleft / fvol;
              if (gauge_setup < 0) {
                backlight_interpolate(p, &r, &g, &b);
                backlight_set_color(r, g, b);
                gauge_set(p);
              }
              
              v = fleft;
              secu.toCharArray(u, 16);
            }
            else {
              r = 0; g = 0; b = 255;
              if (gauge_setup < 0) {
                backlight_set_color(r, g, b);
                gauge_zero();
              }
              
              v = prim.toFloat();
              primu.toCharArray(u, 16);
            }
            dcnt += 1;
            if (((dcnt & 0x1F) > 28) and (temp_valid))
              display_temp(temp);
            else
              display_units(v, u);
            err_cnt = 0;
          }
        }
      }
      digitalWrite(WLED, OFF);
    }
    else {
      err_cnt += 1;
      if (err_cnt > 5) {
        if (!AP_Mode) {
          display_no_signal();
          backlight_set_color(0, 0, 0);
        }
        err_cnt = 5;
      }
    }
    wget_timer = millis();
  }
}
