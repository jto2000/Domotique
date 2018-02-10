void PilotageConvecteur(String textMessage)
{
  char firstChar;
  int SetPoint;
  const int ThdUp1 = 0.2;
  const int ThdUp2 = 1;
  const int ThdDown1 = 0.1;
  const int ThdDown2 = 1;
  
  #if defined(DEBUG_STR)
    Serial.print("Message recu : ");
    Serial.println(textMessage);
    //Serial.print("Premier caractere fonction :");
    
    //Serial.println(textMessage.charAt(0));
    //Serial.println(textMessage);
  #endif
    
  firstChar = textMessage.charAt(0);
  
  if (firstChar == '0') {
    // 0 Mode Off 0 1
     #if defined(DEBUG_STR)
      Serial.println("Mode Off active");
     #endif
    Drive(0,1);
  }
  else if (firstChar == '1') {
     // 1 Mode eco 1 1
     #if defined(DEBUG_STR)
       Serial.println("Mode Eco active");
     #endif
     Drive(1,1);
  }
  else if (firstChar == '2') {  
     // 2 Mode HG 1 0
     #if defined(DEBUG_STR)
       Serial.println("Mode Hors Gel active");
     #endif
    Drive(1,0);
  }
  else if (firstChar == '3') {
    // 3 Mode Confort 0 0
    #if defined(DEBUG_STR)
       Serial.println("Mode Confort active");
     #endif
    Drive(0,0);
  }
  else if (firstChar == '4') {
    // 3 Mode Thermostat
    /*#if defined(DEBUG_STR)
       Serial.print("Mode Thermostat active");
       Serial.println("C est le dawa !!");
     #endif*/
     // Probleme ici : la fonction doit etre appellee regulierement
     // Sinon regulation ne se fait pas...
          
     if (isDigit(textMessage.charAt(2))) {
        SetPoint = textMessage.substring(2).toFloat();
        #if defined(DEBUG_STR)
          Serial.println("Mode Thermostat active");
          Serial.print("Thermostat : ");
          Serial.println(SetPoint);
          Serial.print("Temperature actuelle : ");
          Serial.println(ActualTemp);
        #endif
      
        if (SetPoint > ActualTemp + ThdUp1){
            if (SetPoint > ActualTemp + ThdUp2){
                Drive(0,1); // Mode Off -> Au mini
            }
            else
            {
              Drive(1,1); // Mode eco -> Chauffe douce
            }
        }
        else if (SetPoint < ActualTemp - ThdDown1){
          if (SetPoint < ActualTemp - ThdDown2){
                Drive(0,0); // Mode Confort -> Au max
            }
            else
            {
                Drive(1,1); // Mode eco -> Chauffe douce
            }
        }
          
     }
     
  }
  else {
    #if defined(DEBUG_STR)
        Serial.println("Trame non lisible");
    #endif
  }
}

void Drive(int Pos, int Neg)
{
   digitalWrite(relaisPosPin, Pos);
   digitalWrite(relaisNegPin, Neg);
}
