/*

*/
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

#include <Adafruit_NeoPixel.h>

#define LED_PIN D7
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define GAUGE_PIN D6

ESP8266WiFiMulti WiFiMulti;

#define WSSID "keg6D98FD"
#define WPWD "k3Gat0rr"

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // backlight initialization
  pixels.begin();
  backlight_set_color(0, 0, 0);

  // gauge initialization
  analogWriteFreq(20); // kryplítko (kvůli špatnému budíku)
  gauge_set_pwm(0);

  // wifi connect
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WSSID, WPWD);
}

// the loop function runs over and over again forever
void loop() {
  static uint8_t r = 0, g = 0, b = 0;
  static uint32_t wget_timer = 1000;

  if ((millis() - wget_timer) > 1000) {
    if (WiFiMulti.run() == WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, LOW);
      WiFiClient client;
      HTTPClient http;
  
      Serial.println();
      if (http.begin(client, "http://192.168.42.1/data.json")) {
        int httpCode = http.GET();
        if (httpCode > 0) {
          if ((httpCode == HTTP_CODE_OK) || (httpCode == HTTP_CODE_MOVED_PERMANENTLY)) {
            String payload = http.getString();
            String prim = strip_string(find_key_simple(payload, "units"));
            String primu = strip_string(find_key_simple(payload, "primary_unit"));
            Serial.print(prim);
            Serial.print(" ");
            Serial.println(primu);
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
              backlight_interpolate(p, &r, &g, &b);
              gauge_set(p);
            }
            else {
              r = 0; g = 0; b = 255;
              gauge_set_pwm(200);
            }
          }
        }
      }
      digitalWrite(LED_BUILTIN, HIGH);
    }
    wget_timer = millis();
  }
  
  backlight_pull(r, g, b);
}
