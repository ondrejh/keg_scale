// --- eeprom functions ---

void set_calib_default() {
  clb_eeprom.calib.p = 2;
  clb_eeprom.calib.x[0] = -7090000;
  clb_eeprom.calib.x[1] = -7350000;
  clb_eeprom.calib.y[0] = 0.0;
  clb_eeprom.calib.y[1] = 0.5;  
}

void save_calib() {
  uint32_t adr = EEPROM_CALIB_ADDR;
  uint8_t chsum = 0;
  for (int i=0; i<(sizeof(clb_eeprom.d)-1); i++) {
    EEPROM.write(adr ++, clb_eeprom.d[i]);
    chsum += clb_eeprom.d[i];
  }
  chsum = 0x00 - chsum;
  EEPROM.write(adr, chsum);
  EEPROM.commit();
}

void load_calib() {
  uint32_t adr = EEPROM_CALIB_ADDR;
  uint8_t chsum = 0;
  for (int i=0; i<sizeof(clb_eeprom.d); i++) {
    clb_eeprom.d[i] = EEPROM.read(adr + i);
    chsum += clb_eeprom.d[i];
  }
  if ((chsum!=0) || (clb_eeprom.calib.p < 2) || (clb_eeprom.calib.p >= CLBLEN))
    set_calib_default();
}
