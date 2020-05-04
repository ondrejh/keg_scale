#include "HX711.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = D3;
const int LOADCELL_SCK_PIN = D4;

HX711 scale;

#define MESS 16
int32_t scale_mess[MESS];
uint32_t mess_ptr = 0;
int32_t scale_avg = 0;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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


void setup() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();

  Serial.begin(1000000);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {

  if (scale.is_ready()) {
    int32_t reading = scale.read();
    if (reading != -1) {
      scale_avg -= scale_mess[mess_ptr];
      scale_mess[mess_ptr] = reading;
      scale_avg += scale_mess[mess_ptr++];
      mess_ptr %= MESS;
    }
    Serial.println(scale_avg);
    display_raw_scale();
  } else {
    Serial.println("---");
  }
  delay(100);
}
