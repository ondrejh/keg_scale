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

void display_temp(int temp) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setTextSize(3);
  
  char buff[16];
  int blen;
  int t = temp / 10;
  int dt;
  if (temp >= 0)
    dt = temp - t * 10;
  else
    dt = -temp + t * 10;
  if ((t >= 100) || (t <= -10))
    sprintf(buff, "%d C", t);
  else
    sprintf(buff, "%d.%01d C", t, dt);
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
