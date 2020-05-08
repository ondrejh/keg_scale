/**
 * keg_scale
 * 
 * todo:
 *   webi: calibration input
 *   eeprom: save calibration ect.
 */

#include "HX711.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

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

#define CLBLEN 5
int32_t calibx[CLBLEN] = {-7090000, -7350000};
float caliby[CLBLEN] = {0.0, 0.5}; 
int calp = 2;

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
  digitalWrite(WLED, ON);
  server.send(200, "text/html", index_html);
  digitalWrite(WLED, OFF);
}

void handleJquery() {
  digitalWrite(WLED, ON);
  server.send(200, "application/javascript", jquery_bin);
  digitalWrite(WLED, OFF);
}

void handleData() {
  digitalWrite(WLED, ON);

  char msg[1024];
  int p = 0;
  p = sprintf(msg, "{\"raw\": %d, \"calib\": {", scale_avg);
  bool first = true;
  for (int i = 0; i < calp; i++) {
    if (first) first = false; else p += sprintf(&msg[p], ", ");
    p += sprintf(&msg[p], "\"%0.1f\": %d", caliby[i], calibx[i]);
  }
  p += sprintf(&msg[p], "}}");

  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", msg);

  digitalWrite(WLED, OFF);
}

void handleCalib() {
  digitalWrite(WLED, ON);

  char msg[1024];

  if (server.hasArg("add")) {
    char sval[16];
    server.arg("add").toCharArray(sval, 16);
    float val;
    if (sscanf(sval, "%f", &val) != 1) {
      sprintf(msg, "Error: input arg '%s' can't be converted to float", sval); 
    }
    else {
      int i;
      for (i=0; i<calp; i++) {
        if (val == caliby[i]) {
          calibx[i] = scale_avg;
          sprintf(msg, "Value %s updated to %d", sval, scale_avg);
          break;
        }
      }
      if (i == calp) {
        if (calp < CLBLEN) {
          calibx[calp] = scale_avg;
          caliby[calp++] = val;
          sprintf(msg, "Added new callibration point: %s .. %d", sval, scale_avg);
        }
        else {
          sprintf(msg, "Error: maximum number of callibration points reached");
        }
      }
    }
  }
  else {
    sprintf(msg, "Error: no 'add' arg found");
  }

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

void setup() {
  Serial.begin(115200);

  pinMode(WLED, OUTPUT);
  digitalWrite(WLED, OFF);
  pinMode(SLED, OUTPUT);
  digitalWrite(SLED, OFF);

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
