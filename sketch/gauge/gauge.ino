/*

*/

#include <Adafruit_NeoPixel.h>

#define LED_PIN D7
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define GAUGE_PIN D6

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // backlight initialization
  pixels.begin();
  backlight_set_color(0, 0, 0);

  // gauge initialization
  analogWriteFreq(10); // kryplítko (kvůli špatnému budíku)
  gauge_set_pwm(0);
}

// the loop function runs over and over again forever
void loop() {
  static int cnt=0;
  static bool up=true;
  static uint32_t test_t = 0;
  static uint32_t blink_t = 0;
  static bool led_on = false;
  static uint8_t r = 0, g = 0, b = 0;
  static bool blik = false;
  if ( (millis() - test_t) > 5000 ) {
    test_t += 5000;
    switch (cnt) {
      case 0:
        backlight_interpolate(0.0, &r, &g, &b, &blik);
        gauge_set(0.0);
        break;
      case 1:
        backlight_interpolate(0.25, &r, &g, &b, &blik);
        gauge_set(0.25);
        break;
      case 2:
        backlight_interpolate(0.5, &r, &g, &b, &blik);
        gauge_set(0.5);
        break;
      case 3:
        backlight_interpolate(0.75, &r, &g, &b, &blik);
        gauge_set(0.75);
        break;
      case 4:
        backlight_interpolate(1.0, &r, &g, &b, &blik);
        gauge_set(1.0);
        break;
    }
    
    if (up) {
      cnt ++;
      if (cnt >= 4) up = false;
    } else {
      cnt--;
      if (cnt <= 0) up = true;
    }
    
    digitalWrite(LED_BUILTIN, LOW);
    blink_t = test_t;
    led_on = true;
  }

  if (led_on && ((millis() - blink_t) > 500)) {
    led_on = false;
    digitalWrite(LED_BUILTIN, HIGH);
  }

  backlight_pull(r, g, b, blik);
}
