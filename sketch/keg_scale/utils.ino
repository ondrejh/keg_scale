// other utils

void set_conf_default(conf_t *cnf) {
  cnf->ssid[0] = '\0';
  cnf->wpwd[0] = '\0';
  /*cnf->dkey[0] = '\0';
  cnf->pin[0] = '\0';*/
}

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

// set default keg list
void set_keglist_default(keglist_t *keglist) {
  sprintf(keglist->kegs, DEFAULT_KEG_LIST, strlen(DEFAULT_KEG_LIST));
}

// add keg to list
void add_keg_to_list(char *keg, keglist_t *keglist) {
  int pbeg=0, p=0;
  char newlist[KEG_LIST_MAX_LENGTH];
  p = sprintf(newlist, keg, strlen(keg));
  p += sprintf(&newlist[p], "\n", 1);
  for (int i=1; i<KEG_LIST_MAX_LENGTH; i++) {
    char c = keglist->kegs[i];
    if ((c == '\n') || (c == '\0')) {
      int klen = i-pbeg;
      char kegn[KEG_LABEL_MAX + 1];
      memcpy(kegn, &keglist->kegs[pbeg], klen);
      kegn[klen] = '\0';
      if (strncmp(keg, kegn, KEG_LABEL_MAX) == 0) {
        return; // keg found, no need to add
      }
      if ((p + klen) < (KEG_LIST_MAX_LENGTH - 1)) {
        p += sprintf(&newlist[p], kegn, strlen(kegn));
        p += sprintf(&newlist[p], "\n", 1);
      }
      pbeg = i+1;
    }
    if (c == '\0')
      break;
  }
  newlist[p-1] = '\0';
  memcpy(keglist->kegs, newlist, p);
  int ret = eesave(EEPROM_KEGLIST_ADDR, keglist, sizeof(keglist_t));
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
