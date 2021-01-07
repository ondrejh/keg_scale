/*

*/

#include <Adafruit_NeoPixel.h>

#define LED_PIN D4
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define GAUGE_PIN D8

#define MIN 590
#define G1Q (MIN+(MAX-MIN)/4)
#define GH ((MIN+MAX)/2-15)
#define G3Q (MIN+(MAX-MIN)*3/4-20)
#define MAX 850

int interpol(float val) {
  float p;
  int a,b;
  if (val > 0.75) {
    p = (val - 0.75) * 4.0;
    a = G3Q;
    b = MAX;
  }
  else if (val > 0.5) {
    p = (val - 0.5) * 4.0;
    a = GH;
    b = G3Q;
  }
  else if (val > 0.25) {
    p = (val - 0.25) * 4.0;
    a = G1Q;
    b = GH;
  }
  else {
    p = val * 4.0;
    a = MIN;
    b = G1Q;
  }
  return a; // + (int)((float)(b - a) * p);
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pixels.begin();
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      pixels.show();
}

// the loop function runs over and over again forever
void loop() {
  static int cnt=0;
  switch (cnt) {
    case 0:
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();
      analogWrite(interpol(0.0), MIN);
      break;
    case 1:
      pixels.setPixelColor(0, pixels.Color(255, 128, 0));
      pixels.show();
      analogWrite(interpol(0.25), G1Q);
      break;
    case 2:
      pixels.setPixelColor(0, pixels.Color(255, 255, 0));
      pixels.show();
      analogWrite(interpol(0.5), GH);
      break;
    case 3:
      pixels.setPixelColor(0, pixels.Color(128, 255, 0));
      pixels.show();
      analogWrite(interpol(0.75), G3Q);
      break;
    case 4:
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.show();
      analogWrite(interpol(1.0), MAX);
      break;
  }
  //cnt--;
  //if (cnt < 0) cnt=4;
  cnt ++;
  if (cnt > 4) cnt = 0;
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
  delay(5000);                       // wait for a second
}
