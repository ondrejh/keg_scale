// --- web server handlers ---

void handleRoot() {
  digitalWrite(WLED, ON);
  server.send(200, "text/html", index_html);
  digitalWrite(WLED, OFF);
}

void handleKegstart() {
  digitalWrite(WLED, ON);
  server.send(200, "text/html", keg_html);
  digitalWrite(WLED, OFF);  
}

void handleCalibration() {
  digitalWrite(WLED, ON);
  server.send(200, "text/html", calib_html);
  digitalWrite(WLED, OFF);
}

void handleConfiguration() {
  digitalWrite(WLED, ON);
  server.send(200, "text/html", config_html);
  digitalWrite(WLED, OFF);
}

void handleStyle() {
  digitalWrite(WLED, ON);
  server.send(200, "text/css", sass_style_bin, sizeof(sass_style_bin));
  digitalWrite(WLED, OFF);
}

void handleFavicon() {
  digitalWrite(WLED, ON);
  server.send(200, "image/x-icon", favicon_bin, sizeof(favicon_bin));
  digitalWrite(WLED, OFF);
}

void handleFavicon16() {
  digitalWrite(WLED, ON);
  server.send(200, "image/png", img_favicon_16x16_bin, sizeof(img_favicon_16x16_bin));  
  digitalWrite(WLED, OFF);
}

void handleFavicon32() {
  digitalWrite(WLED, ON);
  server.send(200, "image/png", img_favicon_32x32_bin, sizeof(img_favicon_32x32_bin));  
  digitalWrite(WLED, OFF);
}

void handleAppleIcon() {
  digitalWrite(WLED, ON);
  server.send(200, "image/png", img_apple_touch_icon_bin, sizeof(img_apple_touch_icon_bin));  
  digitalWrite(WLED, OFF);
}

void handleAndroid512() {
  digitalWrite(WLED, ON);
  server.send(200, "image/png", img_android_chrome_512x512_bin, sizeof(img_android_chrome_512x512_bin));  
  digitalWrite(WLED, OFF);
}

void handleAndroid192() {
  digitalWrite(WLED, ON);
  server.send(200, "image/png", img_android_chrome_192x192_bin, sizeof(img_android_chrome_192x192_bin));  
  digitalWrite(WLED, OFF);
}

void handleSite() {
  digitalWrite(WLED, ON);
  server.send(200, "image/png", img_site_bin, sizeof(img_site_bin));  
  digitalWrite(WLED, OFF);
  
}

void handleBg() {
  digitalWrite(WLED, ON);
  server.send(200, "image/jpeg", img_bg_bin, sizeof(img_bg_bin));
  digitalWrite(WLED, OFF);
}

void handleBarell() {
  digitalWrite(WLED, ON);
  server.send(200, "image/svg+xml", img_barell_bin, sizeof(img_barell_bin));
  digitalWrite(WLED, OFF);
}

void handleJquery() {
  digitalWrite(WLED, ON);
  server.send(200, "application/javascript", jquery_3_bin, sizeof(jquery_3_bin));
  digitalWrite(WLED, OFF);
}

void handleData() {
  digitalWrite(WLED, ON);

  char msg[1024];
  int p = 0;
  p = sprintf(msg, "{\"devid\": \"%s\", ", ssid);
  p += sprintf(&msg[p], "\"raw\": %d, \"units\": %0.1f, ", scale_avg, scale_units);
  if (temperature_valid)
    p += sprintf(&msg[p], "\"temp\": %0.1f, \"traw\": %d, ", temperature, raw_temp);
  p += sprintf(&msg[p], "\"primary_unit\": \"%s\", \"secondary_unit\": \"%s\", \"ratio\": %0.3f, \"calib\": {", calib.uprim, calib.usec, calib.us);
  bool first = true;
  for (int i = 0; i < calib.p; i++) {
    if (first) first = false; else p += sprintf(&msg[p], ", ");
    p += sprintf(&msg[p], "\"%0.1f\": %d", calib.y[i], calib.x[i]);
  }
  p += sprintf(&msg[p], "}");
  if (KEG) {
    p += sprintf(&msg[p], ", \"keg\": {\"name\": \"%s\", \"fullraw\": %d, \"volume\": %0.1f, \"left\": %0.1f}", keg.label, keg.fullraw, keg.volume, keg_left);
  }
  p += sprintf(&msg[p], "}");

  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", msg);

  digitalWrite(WLED, OFF);
}

/*void handleCalib() {
  digitalWrite(WLED, ON);

  char msg[1024];

  if (server.hasArg("add")) {
    char sval[16];
    server.arg("add").toCharArray(sval, 16);
    //if (server.hasArg("val")))
    float val;
    if (sscanf(sval, "%f", &val) != 1) {
      sprintf(msg, "Error: input arg '%s' can't be converted to float", sval); 
    }
    else {
      int i;
      for (i=0; i<calib.p; i++) {
        if (val == calib.y[i]) {
          calib.x[i] = scale_avg;
          sprintf(msg, "Value %s updated to %d", sval, scale_avg);
          break;
        }
      }
      if (i == calib.p) {
        if (calib.p < CLBLEN) {
          calib.x[calib.p] = scale_avg;
          calib.y[calib.p] = val;
          calib.p ++;

          // bubblesort
          bubble_calib();

          // save to eeprom
          eesave(EEPROM_CALIB_ADDR, &calib, sizeof(calib));

          sprintf(msg, "Added new callibration point: %s .. %d", sval, scale_avg);
        }
        else {
          sprintf(msg, "Error: maximum number of callibration points reached");
        }
      }
    }
  }
  else if (server.hasArg("del")) {
    char sval[16];
    server.arg("del").toCharArray(sval, 16);
    unsigned int val;
    if (sscanf(sval, "%d", &val) != 1) {
      sprintf(msg, "Error: input arg '%s' can't be converted to int", sval); 
    }
    else {
      if ((val > 0) && (val <= calib.p) && (calib.p > 2)) {
        for (int i=val - 1; i < (CLBLEN - 1); i++) {
          calib.x[i] = calib.x[i + 1];
          calib.y[i] = calib.y[i + 1];
        }
        calib.p --;

        // save to eeprom
        eesave(EEPROM_CALIB_ADDR, &calib, sizeof(calib));
        
        sprintf(msg, "Value %s deleted from callibration", sval);
      }
      else {
        sprintf(msg, "Value %s can't be deleted from callibration", sval);
      }
    }
  }
  else {
    sprintf(msg, "Error: no 'add' arg found");
  }

  server.send(200, "text/html", msg);

  
  digitalWrite(WLED, OFF);
}

void handleKeg() {
  digitalWrite(WLED, ON);
  char msg[256];
  char keg_name[KEG_LABEL_MAX + 1];
  char keg_vol[17];
  float keg_vol_float;
  if (server.hasArg("keg") && server.hasArg("vol")) {
    server.arg("keg").toCharArray(keg_name, KEG_LABEL_MAX);
    keg_name[KEG_LABEL_MAX] = '\0';
    server.arg("vol").toCharArray(keg_vol, 16);
    keg_vol[16] = '\0';
    if (sscanf(keg_vol, "%f", &keg_vol_float) != 1) {
      sprintf(msg, "Error: incorrect volume input");
    }
    else {
      sprintf(msg, "New keg %s volume %0.0f", keg_name, keg_vol_float);
      strncpy(keg.label, keg_name, KEG_LABEL_MAX);
      keg.fullraw = scale_avg;
      keg_full = interpolate(keg.fullraw);
      keg.volume = keg_vol_float;
      keg_left = keg_vol_float;
      keg.keg = true;
      // save to eeprom
      eesave(EEPROM_KEG_ADDR, &keg, sizeof(keg));
    }
  }
  else if (server.hasArg("del") && (server.arg("del") == "yes")) {
    sprintf(msg, "Kill keg %s", keg.label);
    keg.keg = false;
    // save to eeprom
    eesave(EEPROM_KEG_ADDR, &keg, sizeof(keg));
  }
  else
    sprintf(msg, "Input arguments missing");
  server.send(200, "text/html", msg);
  digitalWrite(WLED, OFF);
}*/

void handleDo() {
  char msg[256];
  char keg_name[KEG_LABEL_MAX + 1];
  char keg_vol[17];
  float keg_vol_float;
  bool res = false;
  if (server.hasArg("keg") && server.hasArg("vol")) {
    server.arg("keg").toCharArray(keg_name, KEG_LABEL_MAX);
    keg_name[KEG_LABEL_MAX] = '\0';
    server.arg("vol").toCharArray(keg_vol, 16);
    keg_vol[16] = '\0';
    if (sscanf(keg_vol, "%f", &keg_vol_float) != 1) {
      //sprintf(msg, "Error: incorrect volume input");
    }
    else {
      //sprintf(msg, "New keg %s volume %0.0f", keg_name, keg_vol_float);
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
    }
  }
  
  else if (server.hasArg("del") && (server.arg("del") == "yes")) {
    //sprintf(msg, "Kill keg %s", keg.label);
    keg.keg = false;
    // save to eeprom
    eesave(EEPROM_KEG_ADDR, &keg, sizeof(keg));
    // OK
    res = true;
  }
  else if (server.hasArg("addc")) {
    char sval[16];
    server.arg("addc").toCharArray(sval, 16);
    //if (server.hasArg("val")))
    float val;
    if (sscanf(sval, "%f", &val) != 1) {
      //sprintf(msg, "Error: input arg '%s' can't be converted to float", sval); 
    }
    else {
      bool needSave = false;
      int i;
      for (i=0; i<calib.p; i++) {
        if (val == calib.y[i]) {
          calib.x[i] = scale_avg;
          //sprintf(msg, "Value %s updated to %d", sval, scale_avg);
          // OK
          needSave = true;
          res = true;
          break;
        }
      }
      if (i == calib.p) {
        if (calib.p < CLBLEN) {
          calib.x[calib.p] = scale_avg;
          calib.y[calib.p] = val;
          calib.p ++;

          //sprintf(msg, "Added new callibration point: %s .. %d", sval, scale_avg);
          // OK
          needSave = true;
          res = true;
        }
        else {
          //sprintf(msg, "Error: maximum number of callibration points reached");
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
  else if (server.hasArg("delc")) {
    char sval[16];
    server.arg("delc").toCharArray(sval, 16);
    unsigned int val;
    if (sscanf(sval, "%d", &val) != 1) {
      //sprintf(msg, "Error: input arg '%s' can't be converted to int", sval); 
    }
    else {
      if ((val > 0) && (val <= calib.p) && (calib.p > 2)) {
        for (int i=val - 1; i < (CLBLEN - 1); i++) {
          calib.x[i] = calib.x[i + 1];
          calib.y[i] = calib.y[i + 1];
        }
        calib.p --;

        // save to eeprom
        eesave(EEPROM_CALIB_ADDR, &calib, sizeof(calib));
        
        //sprintf(msg, "Value %s deleted from callibration", sval);
        // OK
        res = true;
      }
      else {
        //sprintf(msg, "Value %s can't be deleted from callibration", sval);
      }
    }
  }

  sprintf(msg, res ? "OK" : "ERROR");
  server.send(200, "text/html", msg);
}

void handleNotFound() {
  digitalWrite(WLED, ON);
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
}
