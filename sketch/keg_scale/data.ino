String f2s1d(float f) {
  char s[64];
  sprintf(s, "%.01f", f);
  return String(s);
}

String f2s2d(float f) {
  char s[64];
  sprintf(s, "%.02f", f);
  return String(s);
}

String f2s3d(float f) {
  char s[64];
  sprintf(s, "%.03f", f);
  return String(s);
}

String build_data() {
  String json = "";
  
  json += "{\"devid\": \"" + String(ssid) + "\"";     // device id
  json += ", \"sw\": \"" + String(sw_version) + "\""; // sw version
  json += ", \"raw\": " + String(scale_avg);          // sensor raw readings
  json += ", \"units\": " + f2s1d(scale_units);       // weight

  if (!AP_Mode)
    json += ", \"ip\": \"" + WiFi.localIP().toString() + "\"";   // ip address

  // temperature (if available)
  if (temperature_valid) {
    json += ", \"temp\": " + f2s1d(temperature);      // temperature in deg C
    //json += ", \"traw\": " + String(raw_temp);        // raw sensor readings
  }

  // units (can't change it but who knows)
  json += ", \"primary_unit\": \"" + String(calib.uprim) + "\"";  // primary unit
  json += ", \"secondary_unit\": \"" + String(calib.usec) + "\""; // secondary unit
  //json += ", \"ratio\": " + f2s3d(calib.us);          // primary to secondary ratio

  // calibration table
  json += ", \"calib\": {";
  bool first = true;
  for (int i = 0; i < calib.p; i++) {
    if (first) first = false; else json += ", ";
    json += "\"" + f2s2d(calib.y[i]) + "\": " + String(calib.x[i]);
  }
  json += "}";

  // keg data
  if (KEG) {
    json += ", \"keg\": {";
    json += "\"name\": \"" + String(keg.label) + "\"";  // name of the keg
    //json += ", \"fullraw\": " + String(keg.fullraw);      // full keg raw sensor reading
    json += ", \"volume\": " + f2s1d(keg.volume);         // keg volume (beers)
    json += ", \"left\": " + f2s1d(keg_left);             // beers left in the keg
    json += "}";
  }

  json += "}"; // das ende
  return json;
}

String build_mqtt() {
  String json = "";
  
  json += "{\"devid\": \"" + String(ssid) + "\"";     // device id
  json += ", \"sw\": \"" + String(sw_version) + "\""; // sw version
  json += ", \"units\": " + f2s1d(scale_units);       // weight
  json += ", \"ip\": \"" + WiFi.localIP().toString() + "\"";   // ip address

  // temperature (if available)
  if (temperature_valid)
    json += ", \"temp\": " + f2s1d(temperature);      // temperature in deg C

  // keg data
  if (KEG) {
    json += ", \"keg\": {";
    json += "\"name\": \"" + String(keg.label) + "\"";  // name of the keg
    json += ", \"volume\": " + f2s1d(keg.volume);         // keg volume (beers)
    json += ", \"left\": " + f2s1d(keg_left);             // beers left in the keg
    json += "}";
  }

  json += "}"; // das ende
  return json;
}