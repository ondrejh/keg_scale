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
