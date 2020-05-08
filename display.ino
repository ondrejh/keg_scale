// --- display functions ---

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
