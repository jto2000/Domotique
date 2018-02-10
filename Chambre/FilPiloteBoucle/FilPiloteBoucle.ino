
// Enable debug prints to serial monitor
#define MY_DEBUG
#define DEBUG_STR
#define DEBUG_CASE
#define DEBUG_POW
//#define DEBUG_AMP

// Enable and select radio type attached
#define MY_RADIO_NRF24

#define MY_NODE_ID 2 // Chambre du bas !

#define MY_REPEATER_FEATURE


// MySensors libraries
#include <SPI.h>
#include <MySensors.h>
#include <OneWire.h>

// Child ID's of this node
#define CHILD_TEMP  0
#define CHILD_POWER 5 //V_WATT, V_KWH
#define CHILD_TEXT  7

// Define Sketch Infos :
#define SKETCH_NAME "Convecteur Chambre Boucle" 
#define SKETCH_VERSION "1.0.1"



// Define Pins
//Addresse actuelle : 28 53 21 28 0 0 80 BC 
//28 E0 1B 28 0 0 80 DB 
byte SENSOR_1[] = { 0x28, 0xE0, 0x1B, 0x28, 0x0, 0x0, 0x80, 0xDB };
OneWire  ds(8);  // on pin 10
const int relaisPosPin = 5; // Inversé sur la carte !
const int relaisNegPin = 4;
const int capteurInt = A0;


bool RelayState ;
float ActualTemp;

// Variables for Current computation :
const int mVperAmp = 100; // Valeur pour 20A
double sum_squared_current = 0;
int nSamples = 0;

int StateMachine = 1; // Init a 1
long Pconso = 0;
double PconsoFloat = 2.9;

long Temps;


// MySensors messages of the node: 

MyMessage TempMsg(CHILD_TEMP, V_TEMP);
MyMessage PowMsg(CHILD_POWER, V_WATT);
MyMessage EnMsg(CHILD_POWER, V_KWH); // Vérif si messages necssairement différenciés

unsigned long TPS_CALCUL_PUISSANCE = 60000;

void setup()
{
  Serial.println(F("Demarrage capteur temperature et pilotage convecteur"));
  pinMode(relaisPosPin, OUTPUT);//Pilotage du Pin Relais Positif
  digitalWrite (relaisPosPin, LOW);//Mise à Low pour commencer
  pinMode(relaisNegPin, OUTPUT);//Pilotage du Pin Relais Positif
  digitalWrite (relaisNegPin, LOW);//Mise à Low pour commencer
}


void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);

  // Register the sensors to the MySensors Gateway
  present(CHILD_TEMP,S_TEMP, "Temperature");
  present(CHILD_POWER,S_POWER, "ACS 712"); // Pas de presentation supp necessaire
  present(CHILD_TEXT, S_INFO, "InfoPilotage");
  
}

void loop() {
  // Déclaration var locales :
  float temperature;
  double RMSCurrentValue;
  double PowerValue;
  double TensionSecteur = 220.0;
  
  // put your main code here, to run repeatedly:
  switch (StateMachine) {
    case 1:
      //Debut => Get kwh from gw
      //Changement de statut ssi reçu
      #if defined(DEBUG_CASE)
          Serial.println("Initialisation");
      #endif

      // Solution temporaire :
      StateMachine++;    
      if (Pconso > 0) {
        StateMachine++;
      }
      else{
        request(CHILD_POWER, V_KWH);
        delay(1000);
      }
      break;

    case 2:
      #if defined(DEBUG_CASE)
          Serial.println("Demande de la temperature");
          Serial.println("Puis debut calcul courant");
      #endif
      //Request V_TEXT
      //Demande temp
      // Ini variable temporelle
      request(CHILD_TEXT, V_TEXT);
      askTemperature(SENSOR_1);
      Temps = millis();
      nSamples = 0;
      sum_squared_current = 0;
      StateMachine++;
      break;
    case 3:
      //Calcul puissance durant temps donné
      // si temps donné écoulé : StateMachine++;
      if ((millis() - Temps) < TPS_CALCUL_PUISSANCE){
        // Calcul
        double inst_current = getAmpVcc();
        double squared_current = inst_current * inst_current;
        
        sum_squared_current += squared_current;
        nSamples++;
      }
      else {
        Temps = millis() - Temps;
        #if defined(DEBUG_CASE)
          Serial.println("Fin du calcul courant");
          Serial.print("Duree : ");
          Serial.println(Temps);
        #endif
        
        StateMachine++; 
      }
      break;
    case 4:
      //Finir calculs de puissance
      // Récupérer les données de température
      // Envoyer Tempe, Puissance, Energie
      // Retour StateMachine = 2;
      long mean_square_current;
      double root_mean_square_current;
      
      mean_square_current = sum_squared_current / nSamples;
      root_mean_square_current = sqrt(mean_square_current);
      double PowerValue;
      
      getTemperature(&temperature, SENSOR_1);
      PowerValue = root_mean_square_current*TensionSecteur;
      #if defined(DEBUG_POW)
        Serial.println("Power measured : ");
        Serial.println(PowerValue);
      #endif
      send(PowMsg.set(PowerValue,2));
      if (temperature != -127.00 && temperature != 85.00) {
        // MyMessage& set(float value, uint8_t decimals)
        send(TempMsg.set(temperature,2));
      }
      PconsoFloat += PowerValue * Temps / (3600.0*1000.0*1000.0);
      #if defined(DEBUG_POW)
        Serial.print("Energy measured : ");
        Serial.println(PconsoFloat);
      #endif
      if (int(PconsoFloat)> 0) {
        #if defined(DEBUG_POW)
          Serial.println("PconsoFloat > 1");
          
        #endif
          Pconso += int(PconsoFloat);
          PconsoFloat -= int(PconsoFloat);
        
          
        #if defined(DEBUG_POW)
          Serial.print("Energy measured sent : ");
          Serial.println(Pconso);
          // Calcul de l energie :
          //(PowerValue * Temps) / (3600*1000); // En ms 1000ms => 1s, 3600s = 1h
        #endif
        // Value is sent only if increased.
        send(EnMsg.set(Pconso,2));
      }
      
      StateMachine = 2;
      
      break;   
    //default: 
      // if nothing else matches, do the default
      // default is optional
    //break;
  }

}

void receive(const MyMessage &message)
{
  #if defined(DEBUG_STR)
    Serial.print("Message received from :");
    Serial.println(message.sensor);
    Serial.print("Content : (bool) ");
    Serial.println(message.getBool());
    Serial.print("Type : ");
    Serial.println(message.type);
    if (message.type == 47)
      Serial.print("String : ");
      Serial.println(message.getString());
  #endif
    if (message.type==V_TEXT) 
    { 
      PilotageConvecteur(message.getString());
    }
    else if (message.type==V_KWH) 
    {
     #if defined(MY_DEBUG)
        Serial.print("Puissance consommee recue : ");
        Serial.println(message.getLong());
     #endif
     Pconso = message.getLong();
    }
}
