// ------------- DISPLAY -------------------------- //

void display_units(float val, char *unit) {
  char buff[32];
  int blen;
  int tsiz;
  sprintf(buff, "%0.1f %s", val, unit);
  blen = strlen(buff);
  tsiz = SCREEN_WIDTH / (blen * 6);
  if (tsiz < 1) tsiz = 1;
  if (tsiz > 3) tsiz = 3;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextSize(tsiz);
  display.setCursor(SCREEN_WIDTH / 2 - blen * 3 * tsiz, SCREEN_HEIGHT / 2 - tsiz * 4 + 1);
  display.write(buff);
  display.display();  
}

void display_wifi(const char* ssid, IPAddress ip) {
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  
  char buff[32];
  int blen;
  //IPAddress ip = WiFi.softAPIP();
  sprintf(buff, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  blen = strlen(buff);
  display.setCursor(64 - blen*3, 20);     // Start at top-left corner
  display.write(buff);

  sprintf(buff, "%s", ssid);
  blen = strlen(buff);
  display.setCursor(64 - blen*3, 4);     // Start at top-left corner
  display.write(buff);  

  display.display();
}

void display_no_signal() {
  char buff[32];
  int blen;
  int tsiz;

  sprintf(buff, "---");
  
  blen = strlen(buff);
  tsiz = SCREEN_WIDTH / (blen * 6);
  if (tsiz < 1) tsiz = 1;
  if (tsiz > 3) tsiz = 3;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextSize(tsiz);
  display.setCursor(SCREEN_WIDTH / 2 - blen * 3 * tsiz, SCREEN_HEIGHT / 2 - tsiz * 4 + 1);
  display.write(buff);
  display.display();  
}

void display_temp(String temp) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextSize(3);
  
  char buff[16];
  int blen;
  sprintf(buff, "%s C", temp);
  blen = strlen(buff);
  int xs = 64 - blen*9;
  display.setCursor(xs, 10);     // Start at top-left corner
  display.print(buff);

  display.setTextSize(2);
  display.setCursor(xs + blen*18 - 32, 5);
  display.print("o");

  display.display();
}

void display_intro() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  const char* buff = "Kegator R";
  int blen = strlen(buff);
  int xs = 64 - blen*6;
  display.setTextSize(2);
  display.setCursor(xs, 12);
  display.print(buff);
  xs = 69 + (blen - 2) * 6;
  display.drawCircle(xs, 19, 11, SSD1306_WHITE);
  display.display();
}

// ------------- JSON ----------------------------- //

bool key_exists(String where, String key) {
  String k = key;
  if (key[0] != '"')
    k = "\"" + key + "\"";
  if (where.indexOf(k) != -1)
    return true;
  return false;
}

String strip_string(String what) {
  int b = 0;
  int e = what.length();
  for (b=0; b<e; b++) {
    if (!((what[b] == ':') || (what[b] == ' ') || (what[b] == '"')))
      break;
  }
  e--;
  for (; e>b; e--) {
    if (!((what[e] == ' ') || (what[e] == '"') || (what[e] == '}')))
      break;
  }
  return what.substring(b, e+1);
}

String find_key_simple(String where, String key) {
  String k = key;
  if (key[0] != '"')
    k = "\"" + key + "\"";
  int b = where.indexOf(k);
  if (b != -1) {
    b += k.length();
    int e = where.indexOf(",", b);
    return where.substring(b, e); 
  }
  return "";
}

// ------ BACKLIGHT -------------------------------- ///

void backlight_interpolate(float val, uint8_t *r, uint8_t *g, uint8_t *b) {
  *b = 0;

  if (val<0.25) {
    *r = 255;
    *g = 0;
  }
  else if (val>0.75) {
    *r = 0;
    *g = 255;
  }
  else {
    float v = (val - 0.25) * 2;
    *g = (int)(v * 255.0);
    *r = 255 - *g;
  }
}

void backlight_set_color(uint8_t r, uint8_t g, uint8_t b) {
  for (int i=0; i<NUMPIXELS; i++)
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  pixels.show();
}

// ------ GAUGE ------------------------------------ ///

// version 3
#define GAUGE_MAX 150
#define GAUGE_MIN 30
// version 2
//#define GAUGE_MAX 700
//#define GAUGE_MIN 330
// version 1
//#define GAUGE_MAX 370
//#define GAUGE_MIN 260
#define GAUGE_TUNE_1_4 0
#define GAUGE_TUNE_2_4 0
#define GAUGE_TUNE_3_4 0
#define GAUGE_ZERO 0

const int gauge_steps[5] = {\
  GAUGE_MIN,\
  GAUGE_MIN + (GAUGE_MAX - GAUGE_MIN) / 4 + GAUGE_TUNE_1_4,\
  GAUGE_MIN + (GAUGE_MAX - GAUGE_MIN) / 2 + GAUGE_TUNE_2_4,\
  GAUGE_MIN + (GAUGE_MAX - GAUGE_MIN) * 3 / 4 + GAUGE_TUNE_3_4,\
  GAUGE_MAX};

int get_gauge_steps(int v) {
  return (v == 0) ? GAUGE_ZERO : gauge_steps[v-1];
}
  
int gauge_interpolate(float val) {
  float p;
  int a,b;
  if (val > 0.75) {
    p = (val - 0.75) * 4.0;
    a = gauge_steps[3];
    b = gauge_steps[4];
  }
  else if (val > 0.5) {
    p = (val - 0.5) * 4.0;
    a = gauge_steps[2];
    b = gauge_steps[3];
  }
  else if (val > 0.25) {
    p = (val - 0.25) * 4.0;
    a = gauge_steps[1];
    b = gauge_steps[2];
  }
  else {
    p = val * 4.0;
    a = gauge_steps[0];
    b = gauge_steps[1];
  }
  
  int ret = a + (int)((float)(b - a) * p);
  
  //Serial.print(val);
  //Serial.print(" ");
  //Serial.println(ret);
  
  return ret;
}

void gauge_set_pwm(int pwm) {
  analogWrite(GAUGE_PIN, pwm);
}

void gauge_set(float val) {
  analogWrite(GAUGE_PIN, gauge_interpolate(val));
}

void gauge_zero() {
  analogWrite(GAUGE_PIN, GAUGE_ZERO);
}
