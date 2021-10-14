#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>

#ifndef STASSID
#define STASSID "Zahrada"
#define STAPSK  "Neiboo2a"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;


// display

#define PIN 2

#define SIZEX 32
#define SIZEY 8

#define NUMPIXELS (SIZEX*SIZEY)

#define MAX_POWER (35 * 3 * 255)

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

typedef struct screen_buffer {
  uint8_t r[NUMPIXELS];
  uint8_t g[NUMPIXELS];
  uint8_t b[NUMPIXELS];
} screen_buffer_t;

screen_buffer_t screen_buffer;

/* funkce clear_buffer
 *  nastaví obsah screen bufferu na samé 0
 */
void clear_buffer(screen_buffer_t *scr) {
  memset(scr, 0, sizeof(screen_buffer_t));
  /*for (int i=0; i++; i<NUMPIXELS) {
    scr->r[i] = 0;
    scr->g[i] = 0;
    scr->b[i] = 0;
  }*/
}

/* funkce set_pixel
 *  nastaví konkrétní pixel na pozici x y ve screen bufferu na zadanou hodnotu
 */
void set_pixel(screen_buffer_t *scr, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  int n = x + SIZEX * y;
  scr->r[n] = r;
  scr->g[n] = g;
  scr->b[n] = b;
}

/* funkce reduce
 *  vrací redukovanou hodnotu jasu pro zadanou výchozí hodnotu,
 *  známý celkový výkon (bez redukce) a maximální výkon
 */
uint8_t reduce(uint8_t pIn, uint32_t factor, uint32_t pMax) {
  if (factor < pMax)
    return pIn;
    
  uint32_t h = pIn;
  h *= pMax;
  h /= factor;
  return h < 0xFF ? h : 0xFF;
}

/* funkce pixel_number
 *  vrací číslo pixelu na pozici x y
 *  pokud je pixel mimo rozsah vrací NUMPIXELS (což je také mimo rozsah)
 */
int pixel_number(int x, int y) {
  if (x < 0)
    return NUMPIXELS;
  if (y < 0)
    return NUMPIXELS;
  if (x >= SIZEX)
    return NUMPIXELS;
  if (y >= SIZEY)
    return NUMPIXELS;
  return NUMPIXELS - 1 - SIZEY * x + (x&1 ? -y : 1 - SIZEY + y);
}

/* funkce screen_buffer_to_pixels
 * převádí screen buffer ve formátu x y na řadu pixelů 
 * přitom spočítá celkový výkon a v případě překročení maxima snižuje jas 
 * screen buffer tak může obsahovat i samé 0xFF
 */
void screen_buffer_to_pixels(screen_buffer_t *scr) {
  uint32_t power = 0;
  for (int i=0; i<NUMPIXELS; i++) {
    power += scr->r[i];
    power += scr->g[i];
    power += scr->b[i];
  }
  
  if (power < MAX_POWER)
    power = MAX_POWER;

  for (int x=0; x<SIZEX; x++) {
    for (int y=0; y<SIZEY; y++) {
      int n = x + SIZEX * y;
      pixels.setPixelColor(pixel_number(x, y), pixels.Color(reduce(scr->r[n], power, MAX_POWER),
                                                            reduce(scr->g[n], power, MAX_POWER),
                                                            reduce(scr->b[n], power, MAX_POWER)));
    }
  }
}

// jine

uint8_t h2u(char c) {
  if ((c >= '0') and (c <= '9'))
    return c - '0';
  if ((c >= 'A') and (c <= 'F'))
    return c - 'A' + 10;
  if ((c >= 'a') and (c <= 'f'))
    return c - 'a' + 10;
  return 0;
}

char u2h(uint8_t h) {
  if (h < 10)
    return h + '0';
  if (h < 16)
    return h - 10 + 'A';
  return '0';
}

// http handlers

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleBuffer() {
  #define BUFF_STR_MAX (6*256)
  char bufstr[BUFF_STR_MAX + 1];
  if (server.hasArg("buffer")) { // set buffer
    server.arg("buffer").toCharArray(bufstr, BUFF_STR_MAX);
    int len;
    len = strlen(bufstr);
    while (len < BUFF_STR_MAX) {
      bufstr[len++] = '0';
    }
    bufstr[len] = '\0';
    for (int i; i<BUFF_STR_MAX; i++) {
      char c = bufstr[i];
      if ((c >= '0') && (c <= '9'))
        continue;
      if ((c >= 'A') && (c <= 'F'))
        continue;
      if ((c >= 'a') && (c <= 'f')) {
        c = 'A' + c - 'a';
        bufstr[i] = c;
        continue;
      }
      bufstr[i] = '0';
    }
    //Serial.println();
    for (int y=0; y < 8; y++) {
      for (int x=0; x < 32; x++) {
        char col[7];
        int p = 6 * (x + y * 32);
        uint8_t r = h2u(bufstr[p++]) << 4 | h2u(bufstr[p++]);
        uint8_t g = h2u(bufstr[p++]) << 4 | h2u(bufstr[p++]);
        uint8_t b = h2u(bufstr[p++]) << 4 | h2u(bufstr[p++]);
        set_pixel(&screen_buffer, x, y, r, g, b);
        //strncpy(col, &bufstr[6 * (x + y * 32)], 6);
        //col[6] = '\0';
        //Serial.print(col);
        //Serial.print(" ");
      }
      //Serial.println("");
    }
    screen_buffer_to_pixels(&screen_buffer);
    pixels.show();

    server.send(200, "text/plain", "OK\n");
  }
  else {
    int p = 0;
    for (int y=0; y < 8; y++) {
      for (int x=0; x < 32; x++) {
        int n = x + y * 32;
        bufstr[p++] = u2h(screen_buffer.r[n] >> 4);
        bufstr[p++] = u2h(screen_buffer.r[n] & 0xF);
        bufstr[p++] = u2h(screen_buffer.g[n] >> 4);
        bufstr[p++] = u2h(screen_buffer.g[n] & 0xF);
        bufstr[p++] = u2h(screen_buffer.b[n] >> 4);
        bufstr[p++] = u2h(screen_buffer.b[n] & 0xF);
      }
    }
    bufstr[p] = '\0';
    server.send(200, "text/plain", bufstr);
  }
}

/*void handleBuffer() {
  char bufstr[BUFF_STR_MAX + 1];
  int p = 0;
  for (int y=0; y < 8; y++) {
    for (int x=0; x < 32; x++) {
      int n = x + y * 32;
      bufstr[p++] = u2h(screen_buffer.r[n] >> 4);
      bufstr[p++] = u2h(screen_buffer.r[n] & 0xF);
      bufstr[p++] = u2h(screen_buffer.g[n] >> 4);
      bufstr[p++] = u2h(screen_buffer.g[n] & 0xF);
      bufstr[p++] = u2h(screen_buffer.b[n] >> 4);
      bufstr[p++] = u2h(screen_buffer.b[n] & 0xF);
    }
  }
  bufstr[p] = '\0';
  server.send(200, "text/plain", bufstr);
}*/

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

void setup(void) {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();   // Send the updated pixel colors to the hardware.
  clear_buffer(&screen_buffer);

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("kegDisplay")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/buffer", HTTP_POST, handleBuffer);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
