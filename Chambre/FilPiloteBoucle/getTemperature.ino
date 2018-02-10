
void getTemperature(float *temperature, byte addr[8]) {
  // Partie 1 demande la temperature, partie 2 récupere depuis scratchpad
  byte data[9];
/*
  // Reset le bus 1-Wire et indique l'adresse de la sonde  
  ds.reset();
  ds.select(addr);

  // Lance la prise de mesure de température
  ds.write(0x44, 1);
  delay(800);*/

  // Reset le bus 1-Wire, sélectionne le capteur et envoie une demande de lecture du scratchpad 
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  
  for (byte i = 0; i < 9; i++) {           // 9 bytes nécessaires ? A vérifier pour nég
    data[i] = ds.read();
  }
  *temperature = ((data[1] << 8) | data[0]) * 0.0625; 
}

void askTemperature(byte addr[8]){
  // This function asks temperature to the dedicated bus :
  byte data[9];

  // Reset le bus 1-Wire et indique l'adresse de la sonde  
  ds.reset();
  ds.select(addr);

  // Lance la prise de mesure de température
  ds.write(0x44, 1);
}
