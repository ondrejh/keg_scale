// ------ BACKLIGHT -------------------------------- ///

void backlight_interpolate(float val, uint8_t *r, uint8_t *g, uint8_t *b, bool *blik) {
  if (val<0.1) *blik = true;
  else if (val>0.2) *blik = false;
  
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
  *b = 0;
}

void backlight_pull(uint8_t r, uint8_t g, uint8_t b, bool blik) {
  static uint32_t bl_timer = 0;
  static uint8_t rr = 0, gg = 0, bb = 0;
  static bool bblik = false;
  if ((millis() - bl_timer) > 40) {
    float shad = 1.0;
    bl_timer += 40;
    if (rr < r) rr++;
    if (rr > r) rr--;
    if (gg < g) gg++;
    if (gg > g) gg--;
    if (bb < b) bb--;
    if (bb < b) bb++;
    if (blik)  bblik = true;
    if (bblik) {
      static int bcnt = 0;
      if (bcnt<50)
        shad = float(50 - bcnt) / 50.0;
      else if (bcnt<100)
        shad = 0.0;
      else if (bcnt<150)
        shad = float(bcnt - 100) / 50.0;
      else
        shad = 1.0;
      bcnt += 7;
      if (bcnt >= 200) {
        bcnt = 0;
        bblik = false;
      }
      //Serial.print(bcnt);
      //Serial.print(" ");
      //Serial.println(shad);
    }
    uint8_t rrr = (int)(shad * float(rr));
    uint8_t ggg = (int)(shad * float(gg));
    uint8_t bbb = (int)(shad * float(bb));
    backlight_set_color(rrr, ggg, bbb);
  }
}

void backlight_set_color(uint8_t r, uint8_t g, uint8_t b) {
  for (int i=0; i<NUMPIXELS; i++)
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  pixels.show();
}

// ------ GAUGE ------------------------------------ ///

#define GAUGE_MAX 830
#define GAUGE_MIN 530
#define GAUGE_TUNE_1_4 25
#define GAUGE_TUNE_2_4 -5
#define GAUGE_TUNE_3_4 -25

const int gauge_steps[5] = {\
  GAUGE_MIN,\
  GAUGE_MIN + (GAUGE_MAX - GAUGE_MIN) / 4 + GAUGE_TUNE_1_4,\
  GAUGE_MIN + (GAUGE_MAX - GAUGE_MIN) / 2 + GAUGE_TUNE_2_4,\
  GAUGE_MIN + (GAUGE_MAX - GAUGE_MIN) * 3 / 4 + GAUGE_TUNE_3_4,\
  GAUGE_MAX};
  
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
