// other utils

// bubblesort calibration to have x axis consistent
void bubble_calib(void) {
  for (int i=1; i<clb_eeprom.calib.p; i++) {
    int cnt = 0;
    for (int j=1; j<clb_eeprom.calib.p; j++) {
      if (clb_eeprom.calib.x[j-1] > clb_eeprom.calib.x[j]) {
        int32_t x = clb_eeprom.calib.x[j];
        float y = clb_eeprom.calib.y[j];
        clb_eeprom.calib.x[j] = clb_eeprom.calib.x[j-1];
        clb_eeprom.calib.y[j] = clb_eeprom.calib.y[j-1];
        clb_eeprom.calib.x[j-1] = x;
        clb_eeprom.calib.y[j-1] = y;
        cnt ++;
      }
    }
    if (cnt == 0)
      break;
  }
}

// interpolate with callibration
float interpolate(int32_t raw) {
  int len = clb_eeprom.calib.p;
  int i;
  for (i=1; i<(len - 1); i++) {
    if (raw < clb_eeprom.calib.x[i])
      break;
  }
  int32_t x1 = clb_eeprom.calib.x[i-1];
  int32_t x2 = clb_eeprom.calib.x[i];
  float y1 = clb_eeprom.calib.y[i-1];
  float y2 = clb_eeprom.calib.y[i];
  
  return y1 + (y2 - y1) * (raw - x1) / (x2 - x1);
}
