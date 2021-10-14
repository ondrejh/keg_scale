#include <Adafruit_NeoPixel.h>

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

#define DELAYVAL 100 // Time (in milliseconds) to pause between pixels

void setup() {
  Serial.begin(115200);
  
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();   // Send the updated pixel colors to the hardware.
  clear_buffer(&screen_buffer);
  delay(DELAYVAL); // Pause before next pass through loop
}

void loop() {
  static int x = 0;
  static int y = 0;

  set_pixel(&screen_buffer, x, y, 255, 255, 255);
  screen_buffer_to_pixels(&screen_buffer);
  pixels.show();   // Send the updated pixel colors to the hardware.

  x += 1;
  if (x >= SIZEX) {
    x = 0;
    y += 1;
    y %= SIZEY;
  }

  delay(DELAYVAL); // Pause before next pass through loop
}
