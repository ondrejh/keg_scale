// --- display functions ---

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

void display_raw_scale(int32_t raw) {
  char buff[32];
  int blen;
  sprintf(buff, "%d", raw);
  blen = strlen(buff);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextSize(2);
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
