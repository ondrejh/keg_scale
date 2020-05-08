// --- web server handlers ---

void handleRoot() {
  digitalWrite(WLED, ON);
  server.send(200, "text/html", index_html);
  digitalWrite(WLED, OFF);
}

void handleJquery() {
  digitalWrite(WLED, ON);
  server.send(200, "application/javascript", jquery_bin);
  digitalWrite(WLED, OFF);
}

void handleData() {
  digitalWrite(WLED, ON);

  char msg[1024];
  int p = 0;
  p = sprintf(msg, "{\"raw\": %d, \"calib\": {", scale_avg);
  bool first = true;
  for (int i = 0; i < clb_eeprom.calib.p; i++) {
    if (first) first = false; else p += sprintf(&msg[p], ", ");
    p += sprintf(&msg[p], "\"%0.1f\": %d", clb_eeprom.calib.y[i], clb_eeprom.calib.x[i]);
  }
  p += sprintf(&msg[p], "}}");

  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", msg);

  digitalWrite(WLED, OFF);
}

void handleCalib() {
  digitalWrite(WLED, ON);

  char msg[1024];

  if (server.hasArg("add")) {
    char sval[16];
    server.arg("add").toCharArray(sval, 16);
    float val;
    if (sscanf(sval, "%f", &val) != 1) {
      sprintf(msg, "Error: input arg '%s' can't be converted to float", sval); 
    }
    else {
      int i;
      for (i=0; i<clb_eeprom.calib.p; i++) {
        if (val == clb_eeprom.calib.y[i]) {
          clb_eeprom.calib.x[i] = scale_avg;
          sprintf(msg, "Value %s updated to %d", sval, scale_avg);
          break;
        }
      }
      if (i == clb_eeprom.calib.p) {
        if (clb_eeprom.calib.p < CLBLEN) {
          clb_eeprom.calib.x[clb_eeprom.calib.p] = scale_avg;
          clb_eeprom.calib.y[clb_eeprom.calib.p] = val;
          clb_eeprom.calib.p ++;
          bubble_calib();
          save_calib();
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
      if ((val > 0) && (val <= clb_eeprom.calib.p) && (clb_eeprom.calib.p > 2)) {
        for (int i=val - 1; i < (CLBLEN - 1); i++) {
          clb_eeprom.calib.x[i] = clb_eeprom.calib.x[i + 1];
          clb_eeprom.calib.y[i] = clb_eeprom.calib.y[i + 1];
        }
        clb_eeprom.calib.p --;
        save_calib();
        
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