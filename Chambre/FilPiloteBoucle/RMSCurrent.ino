// Fonction RMSCurrent :
// https://openenergymonitor.org/emon/buildingblocks/ac-power-arduino-maths
/*long RMSCurrent(){

long mean_square_current;
double inst_current;
double squared_current;
double sum_squared_current = 0;
const int number_of_samples = 300;
double root_mean_square_current;
int n;

for (n=0; n<number_of_samples; n++)
{
  // inst_current calculation from raw ADC input goes here.
  inst_current = getAmpVcc();

  squared_current = inst_current * inst_current;

  sum_squared_current += squared_current;
}

mean_square_current = sum_squared_current / number_of_samples;
root_mean_square_current = sqrt(mean_square_current);
return root_mean_square_current;

}*/

// Fonction getAmpVcc
// 13/01/17
// Récupère les valeurs brutes et renvoie l'intensité
// Mesure recalée avec une lecture de Vcc

double getAmpVcc() {
  int RawValue;
  int ACSoffset = 2500; // Quezaco
  double Voltage = 0;
  double Amps = 0;
  double Vcc;

  #if defined(DEBUG_AMP)
    double AmpsBis;
    double VoltageBis;
  #endif
  
  Vcc = readVcc();

  RawValue = analogRead(capteurInt);
  #if defined(DEBUG_AMP)
    Serial.print("Valeur brute : ");
    Serial.println(RawValue);
  #endif
  Voltage = (RawValue /1024.0) * Vcc;
  #if defined(DEBUG_AMP)
    Serial.print("Tension mesuree : en mV ");
    Serial.print(Voltage);
    Serial.print("Difference avec valeur fixe : en mV");
    VoltageBis = (RawValue /1024.0) * 5000;
    Serial.println(abs(Voltage - VoltageBis));
    
  #endif
  Amps = ((Voltage - ACSoffset) / mVperAmp);

  #if defined(DEBUG_AMP)
    Serial.print("Intensite lue : ");
    Serial.println(Amps);
    AmpsBis = ((VoltageBis - ACSoffset) / mVperAmp);
    Serial.print("Diff en A : ");
    Serial.println(abs(Amps - AmpsBis));
  #endif

  return Amps;
}

// Function from : 
// https://hackingmajenkoblog.wordpress.com/2016/02/01/making-accurate-adc-readings-on-the-arduino/

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}
