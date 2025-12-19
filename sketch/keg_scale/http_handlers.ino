// --- web server handlers ---

void handleRoot() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("index_html");
  //Serial.print(" .. ");
  server.send(200, "text/html", index_html);
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleKegstart() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("keg_bin");
  //Serial.print(" .. ");
  server.send(200, "text/html", keg_bin);
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);  
}

void handleCalibration() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("calib_html");
  //Serial.print(" .. ");
  server.send(200, "text/html", calib_html);
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleConfiguration() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("config_html");
  //Serial.print(" .. ");
  server.send(200, "text/html", config_html);
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleStyle() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("sass_style_bin");
  //Serial.print(" .. ");
  server.send(200, "text/css", sass_style_bin, sizeof(sass_style_bin));
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleFavicon() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("favicon_bin");
  //Serial.print(" .. ");
  server.send(200, "image/x-icon", favicon_bin, sizeof(favicon_bin));
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleFavicon16() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("img_favicon_16x16_bin");
  //Serial.print(" .. ");
  server.send(200, "image/png", img_favicon_16x16_bin, sizeof(img_favicon_16x16_bin));  
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleFavicon32() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("img_favicon_32x32_bin");
  //Serial.print(" .. ");
  server.send(200, "image/png", img_favicon_32x32_bin, sizeof(img_favicon_32x32_bin));  
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleAppleIcon() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("img_apple_touch_icon_bin");
  //Serial.print(" .. ");
  server.send(200, "image/png", img_apple_touch_icon_bin, sizeof(img_apple_touch_icon_bin));  
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleAndroid512() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("img_android_chrome_512x512_bin");
  //Serial.print(" .. ");
  server.send(200, "image/png", img_android_chrome_512x512_bin, sizeof(img_android_chrome_512x512_bin));  
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleAndroid192() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("img_android_chrome_192x192_bin");
  //Serial.print(" .. ");
  server.send(200, "image/png", img_android_chrome_192x192_bin, sizeof(img_android_chrome_192x192_bin));  
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleSite() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("img_site_bin");
  //Serial.print(" .. ");
  server.send(200, "image/png", img_site_bin, sizeof(img_site_bin));  
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleBg() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("img_bg_bin");
  //Serial.print(" .. ");
  server.send(200, "image/jpeg", img_bg_bin, sizeof(img_bg_bin));
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleBarell() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("img_barell_bin");
  //Serial.print(" .. ");
  server.send(200, "image/svg+xml", img_barell_bin, sizeof(img_barell_bin));
  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void jsonHeader() {
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Content-type", "application/json");
}

void handleData() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("data");
  //Serial.print(" .. ");

  char msg[1024];
  int p = 0;
  p = sprintf(msg, "{\"devid\": \"%s\", \"sw\": \"%s\", ", ssid, sw_version);
  p += sprintf(&msg[p], "\"raw\": %d, \"units\": %0.1f, ", scale_avg, scale_units);
  if (temperature_valid)
    p += sprintf(&msg[p], "\"temp\": %0.1f, \"traw\": %d, ", temperature, raw_temp);
  p += sprintf(&msg[p], "\"primary_unit\": \"%s\", \"secondary_unit\": \"%s\", \"ratio\": %0.3f, \"calib\": {", calib.uprim, calib.usec, calib.us);
  bool first = true;
  for (int i = 0; i < calib.p; i++) {
    if (first) first = false; else p += sprintf(&msg[p], ", ");
    p += sprintf(&msg[p], "\"%0.02f\": %d", calib.y[i], calib.x[i]);
  }
  p += sprintf(&msg[p], "}");
  if (KEG) {
    p += sprintf(&msg[p], ", \"keg\": {\"name\": \"%s\", \"fullraw\": %d, \"volume\": %0.1f, \"left\": %0.1f}", keg.label, keg.fullraw, keg.volume, keg_left);
  }
  p += sprintf(&msg[p], "}");

  jsonHeader();
  server.send(200, "application/json", msg);

  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleConfData() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("config");
  //Serial.print(" .. ");

  char msg[1024];
  int p = 0;
  p = sprintf(msg, "{");
  //if (conf.dkey[0] != '\0') 
  //  p += sprintf(&msg[p], " \"dkey\" :\"%s\"", conf.dkey);
  if (conf.ssid[0] != '\0')
    p += sprintf(&msg[p], " \"ssid\" :\"%s\"", conf.ssid);
  p += sprintf(&msg[p], " }");

  jsonHeader();
  server.send(200, "application/json", msg);
  /*server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.sendHeader("Access-Control-Allow-Origin", "*");*/

  //Serial.println(millis() - now);
  digitalWrite(WLED, OFF);
}

void handleDo() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("do");
  //Serial.print(" .. ");

  char msg[256];
  char keg_name[KEG_LABEL_MAX + 1];
  char keg_vol[17];
  float keg_vol_float;
  bool res = false;

  if (server.hasArg("keg") && server.hasArg("vol")) { // new keg
    server.arg("keg").toCharArray(keg_name, KEG_LABEL_MAX);
    keg_name[KEG_LABEL_MAX] = '\0';
    server.arg("vol").toCharArray(keg_vol, 16);
    keg_vol[16] = '\0';
    if (sscanf(keg_vol, "%f", &keg_vol_float) == 1) {
      strncpy(keg.label, keg_name, KEG_LABEL_MAX);
      keg.fullraw = scale_avg;
      keg_full = interpolate(keg.fullraw);
      keg.volume = keg_vol_float;
      keg_left = keg_vol_float;
      keg.keg = true;
      // save to eeprom
      eesave(EEPROM_KEG_ADDR, &keg, sizeof(keg));
      // OK
      res = true;
      // add new keg to list (for the whispler)
      add_keg_to_list(keg_name, &keglist);
    }
  }
  
  else if (server.hasArg("del") && (server.arg("del") == "yes")) { // das ende, slus, vypito
    keg.keg = false;
    // save to eeprom
    eesave(EEPROM_KEG_ADDR, &keg, sizeof(keg));
    // OK
    res = true;
  }

  else if (server.hasArg("list")) { // add keg name to list (to restore saved list)
    server.arg("keg").toCharArray(keg_name, KEG_LABEL_MAX);
    keg_name[KEG_LABEL_MAX] = '\0';
    add_keg_to_list(keg_name, &keglist);
  }

  else if (server.hasArg("addc")) { // add calibration row
    char sval[16];
    float val;
    int32_t raw;
    server.arg("addc").toCharArray(sval, 16);

    if (server.hasArg("rawc")) { // with value (to restore saved calibration)
      char rval[16];
      server.arg("rawc").toCharArray(rval, 16);
      if (sscanf(rval, "%i", &raw) != 1) {
        raw = scale_avg;
      }
    }
    else 
      raw = scale_avg;

    if (sscanf(sval, "%f", &val) == 1) {
      bool needSave = false;
      int i;
      for (i=0; i<calib.p; i++) {
        if (val == calib.y[i]) {
          calib.x[i] = raw;
          // OK
          needSave = true;
          res = true;
          break;
        }
      }
      if (i == calib.p) {
        if (calib.p < CLBLEN) {
          calib.x[calib.p] = raw;
          calib.y[calib.p] = val;
          calib.p ++;

          // OK
          needSave = true;
          res = true;
        }
      }
      if (needSave) {
          // bubblesort
          bubble_calib();
          // save to eeprom
          eesave(EEPROM_CALIB_ADDR, &calib, sizeof(calib));
      }
    }
  }

  else if (server.hasArg("delc")) { // delete callibration row
    char sval[16];
    server.arg("delc").toCharArray(sval, 16);
    unsigned int val;
    if (sscanf(sval, "%d", &val) == 1) {
      if ((val > 0) && (val <= calib.p) && (calib.p > 2)) {
        for (int i=val - 1; i < (CLBLEN - 1); i++) {
          calib.x[i] = calib.x[i + 1];
          calib.y[i] = calib.y[i + 1];
        }
        calib.p --;

        // save to eeprom
        eesave(EEPROM_CALIB_ADDR, &calib, sizeof(calib));
        
        // OK
        res = true;
      }
    }
  }

  else if (server.hasArg("wssid")) { // set wifi name (and password)
    server.arg("wssid").toCharArray(conf.ssid, CONF_SSID_MAX);
    conf.ssid[CONF_SSID_MAX] = '\0';
    if (server.hasArg("wpwd")) {
      server.arg("wpwd").toCharArray(conf.wpwd, CONF_PWD_MAX);
      conf.wpwd[CONF_PWD_MAX] = '\0';
    }
    else
      conf.wpwd[0] = '\0';
    eesave(EEPROM_CONF_ADDR, &conf, sizeof(conf));
    res = true;
  }

  sprintf(msg, res ? "OK" : "ERROR");
  server.send(200, "text/html", msg);
  digitalWrite(WLED, OFF);
  //Serial.println(millis() - now);
}

void handleList() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("list");
  //Serial.print(" .. ");

  char msg[512];
  int p = 0;
  p += sprintf(&msg[p], keglist.kegs);
  p += sprintf(&msg[p], "\n");
  server.send(200, "text/plain", msg);
  digitalWrite(WLED, OFF);
  //Serial.println(millis() - now);
}

void handleNotFound() {
  //uint32_t now = millis();
  digitalWrite(WLED, ON);
  //Serial.print("not_found");
  //Serial.print(" .. ");

  if (server.method() == HTTP_OPTIONS) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.send(204);
  } else {
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
    server.send(404, "text/plain", "");
  }
  digitalWrite(WLED, OFF);
  //Serial.println(millis() - now);
}
