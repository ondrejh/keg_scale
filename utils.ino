// other utils

// set calibration defaults (if calibration missing)
void set_calib_default(calib_t *cal) {
  cal->p = 2;
  cal->x[0] = -18689192;
  cal->x[1] = -7110488;
  cal->y[0] = 30.0;
  cal->y[1] = 0.0;
  cal->us = 0.5; // 1 piv = 0.5 kg
  sprintf(cal->uprim, "kg");
  sprintf(cal->usec, "piv");
  bubble_calib();
}

// bubblesort calibration to have x axis consistent
void bubble_calib(void) {
  for (int i=1; i<calib.p; i++) {
    int cnt = 0;
    for (int j=1; j<calib.p; j++) {
      if (calib.x[j-1] > calib.x[j]) {
        int32_t x = calib.x[j];
        float y = calib.y[j];
        calib.x[j] = calib.x[j-1];
        calib.y[j] = calib.y[j-1];
        calib.x[j-1] = x;
        calib.y[j-1] = y;
        cnt ++;
      }
    }
    if (cnt == 0)
      break;
  }
}

// interpolate with callibration
float interpolate(int32_t raw) {
  int len = calib.p;
  int i;
  for (i=1; i<(len - 1); i++) {
    if (raw < calib.x[i])
      break;
  }
  int32_t x1 = calib.x[i-1];
  int32_t x2 = calib.x[i];
  float y1 = calib.y[i-1];
  float y2 = calib.y[i];
  
  return y1 + (y2 - y1) * (raw - x1) / (x2 - x1);
}
