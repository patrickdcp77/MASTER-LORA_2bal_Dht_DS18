/*
fonctionne correctement
------------************************
dernier essai

pour les balances, cablage des connecteurs étanches
cordon crème et plat
noire   A,B+
rouge   E-
jaune   E+
vert    A,B-

correspondance avec les cables noirs étanches
vert    A,B+
noir    E-
rouge   E+
bleu    A,B-

cablage des balances vues en dessous (donc platine porte jauges retournée)
vue eu U inversé
    -------------------------
    |  E-              A,B+ |
    |                       |    
    |                       |
    |   A,B-            E+  |

fils noirs des jauges relient les jauges E- avec AB-  et AB+ avec E+
fils blancs des jauges telient E- avec AB+    et AB- avec E+

Fonctionnement :
- Identification automatique du module par encodeur matériel (5 entrées = 32 modules possibles, lora-01 à lora-32).
- Pour chaque module lora-X, placer une résistance externe à VCC sur les GPIO correspondant aux bits à 1 du code binaire du numéro du module (Bit 0 = GPIO1, Bit 1 = GPIO2, Bit 2 = GPIO3, Bit 3 = GPIO4, Bit 4 = GPIO11).
- Les autres GPIO restent connectés à la résistance interne de pulldown (INPUT_PULLDOWN).
- Résistance recommandée : 10 kΩ.

Exemples :
  - lora-01 (code 1,  binaire 00001) : résistance à VCC sur GPIO1
  - lora-02 (code 2,  binaire 00010) : résistance à VCC sur GPIO2
  - lora-03 (code 3,  binaire 00011) : résistances à VCC sur GPIO1 et GPIO2
  - lora-04 (code 4,  binaire 00100) : résistance à VCC sur GPIO3
  - lora-05 (code 5,  binaire 00101) : résistances à VCC sur GPIO1 et GPIO3
  - lora-06 (code 6,  binaire 00110) : résistances à VCC sur GPIO2 et GPIO3
  - lora-07 (code 7,  binaire 00111) : résistances à VCC sur GPIO1, GPIO2 et GPIO3
  - lora-08 (code 8,  binaire 01000) : résistance à VCC sur GPIO4
  - lora-09 (code 9,  binaire 01001) : résistances à VCC sur GPIO1 et GPIO4
  - lora-10 (code 10, binaire 01010) : résistances à VCC sur GPIO2 et GPIO4
  - lora-11 (code 11, binaire 01011) : résistances à VCC sur GPIO1, GPIO2 et GPIO4
  - lora-12 (code 12, binaire 01100) : résistances à VCC sur GPIO3 et GPIO4
  - lora-13 (code 13, binaire 01101) : résistances à VCC sur GPIO1, GPIO3 et GPIO4
  - lora-14 (code 14, binaire 01110) : résistances à VCC sur GPIO2, GPIO3 et GPIO4
  - lora-15 (code 15, binaire 01111) : résistances à VCC sur GPIO1, GPIO2, GPIO3 et GPIO4
  - lora-16 (code 16, binaire 10000) : résistance à VCC sur GPIO11
  - lora-17 (code 17, binaire 10001) : résistances à VCC sur GPIO1 et GPIO11
  - lora-18 (code 18, binaire 10010) : résistances à VCC sur GPIO2 et GPIO11
  - lora-19 (code 19, binaire 10011) : résistances à VCC sur GPIO1, GPIO2 et GPIO11
  - lora-20 (code 20, binaire 10100) : résistances à VCC sur GPIO3 et GPIO11
  - lora-21 (code 21, binaire 10101) : résistances à VCC sur GPIO1, GPIO3 et GPIO11
  - lora-22 (code 22, binaire 10110) : résistances à VCC sur GPIO2, GPIO3 et GPIO11
  - lora-23 (code 23, binaire 10111) : résistances à VCC sur GPIO1, GPIO2, GPIO3 et GPIO11
  - lora-24 (code 24, binaire 11000) : résistances à VCC sur GPIO4 et GPIO11
  - lora-25 (code 25, binaire 11001) : résistances à VCC sur GPIO1, GPIO4 et GPIO11
  - lora-26 (code 26, binaire 11010) : résistances à VCC sur GPIO2, GPIO4 et GPIO11
  - lora-27 (code 27, binaire 11011) : résistances à VCC sur GPIO1, GPIO2, GPIO4 et GPIO11
  - lora-28 (code 28, binaire 11100) : résistances à VCC sur GPIO3, GPIO4 et GPIO11
  - lora-29 (code 29, binaire 11101) : résistances à VCC sur GPIO1, GPIO3, GPIO4 et GPIO11
  - lora-30 (code 30, binaire 11110) : résistances à VCC sur GPIO2, GPIO3, GPIO4 et GPIO11
  - lora-31 (code 31, binaire 11111) : résistances à VCC sur GPIO1, GPIO2, GPIO3, GPIO4 et GPIO11
  - lora-32 (code 32, binaire 00000) : aucune résistance à VCC (tous à 0, cas spécial si tu veux l'utiliser)

GPIO utilisés :
  GPIO5  : DHT22
  GPIO6  : HX711 DATA
  GPIO7  : HX711 SCK
  ADC3   : DS18B20 (OneWire)
  GPIO1-4, GPIO11 : encodeur ID module

Ne jamais laisser un GPIO en l'air : chaque entrée est configurée en INPUT_PULLDOWN, donc à 0 par défaut.
Pour coder un bit à 1, relier le GPIO à VCC via une résistance de 10 kΩ.



*/

// Librairies
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <EEPROM.h>
#include "OneWire.h"
//#include "OneWire_CubeCell.h"
#include "DallasTemperature.h"
#include <DHT.h>
#include <DHT_U.h>
#include "HX711.h"

// Définition des broches
#define EEPROM_ADDR_TARE_A 0
#define DHTPIN GPIO5
#define DHTTYPE DHT22
#define PIN_HX711_N1_DATA_OUT GPIO6
#define PIN_HX711_N1_SCK_AND_POWER_DOWN GPIO7
#define ENCODER_PIN_0 GPIO1
#define ENCODER_PIN_1 GPIO2
#define ENCODER_PIN_2 GPIO3
#define ENCODER_PIN_3 GPIO4
#define ENCODER_PIN_4 GPIO11

// Capteurs
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ADC3);
DallasTemperature ds(&oneWire);
HX711 Hx711_N1;

// Variables globales
float offset_HX711_N1_ChannelA;
float offset_HX711_N1_ChannelB;
const unsigned int Weight_sensitivity = 4;
float humidite, temperature;
/* OTAA para c'est ce OTAA paramètre qui est utilisé */

uint8_t appEui[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

const uint8_t devEuiTable[32][8] = {
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xE8 }, // lora-01
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xE9 }, // lora-02
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xEA }, // lora-03
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xEB }, // lora-04
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xE2 }, // lora-05
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xEC }, // lora-06
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xED }, // lora-07
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xE4 }, // lora-08
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xEE }, // lora-09
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xB1, 0xE7 }, // lora-10
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-11
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x23, 0x7A }, // lora-12
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-13
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-14
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-15
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x29, 0x3C }, // lora-16
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x29, 0x3E }, // lora-17
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x29, 0x3F }, // lora-18
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-19
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-20
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-21
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-22
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-23
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-24
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-25
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-26
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-27
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-28
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-29
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-30
  { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x26, 0x14 }, // lora-31
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // lora-32
};

const uint8_t appKeyTable[32][16] = {
  { 0x64, 0x15, 0x08, 0x32, 0x47, 0x90, 0x93, 0x43, 0x1D, 0x26, 0xB5, 0xFD, 0x34, 0xF8, 0xF0, 0xB8 }, // lora-01
  { 0x44, 0x9B, 0x5A, 0x40, 0x01, 0x94, 0x8A, 0x20, 0xE0, 0xD5, 0x89, 0x1B, 0xB6, 0x64, 0x7C, 0xCC }, // lora-02
  { 0xDF, 0x6F, 0x66, 0x43, 0x25, 0x79, 0x56, 0x07, 0x11, 0x06, 0x0D, 0x5A, 0x39, 0x4B, 0x9C, 0x36 }, // lora-03
  { 0x03, 0x6C, 0x0D, 0x88, 0x44, 0xDC, 0x68, 0xE4, 0xE5, 0xAE, 0xEE, 0x00, 0xCF, 0xA5, 0xFD, 0x67 }, // lora-04
  { 0x1E, 0xE9, 0x60, 0x7D, 0x71, 0x7A, 0x40, 0x17, 0x67, 0x10, 0x62, 0xFD, 0x02, 0xF3, 0xBB, 0x54 }, // lora-05
  { 0xD2, 0x4E, 0x7A, 0xBF, 0xC9, 0x3E, 0x44, 0x4D, 0x3A, 0x37, 0x21, 0xA8, 0x92, 0xAD, 0x7F, 0x1B }, // lora-06
  { 0xF1, 0x3D, 0x66, 0x8A, 0x97, 0x04, 0xE7, 0x9D, 0x9F, 0xA6, 0x7A, 0x8B, 0x35, 0x9E, 0xF1, 0x24 }, // lora-07
  { 0x8D, 0x7D, 0xFE, 0x05, 0x32, 0x45, 0xAD, 0xAC, 0x3A, 0x40, 0x74, 0x04, 0x78, 0x85, 0xDE, 0xF0 }, // lora-08
  { 0x1C, 0x92, 0x06, 0x95, 0xD4, 0x03, 0xF4, 0x34, 0xC6, 0x60, 0xB8, 0xC4, 0xFD, 0x90, 0xA2, 0x8D }, // lora-09
  { 0xAD, 0x07, 0x52, 0x8F, 0x33, 0x61, 0xBC, 0x9B, 0xDD, 0x28, 0x54, 0x93, 0x35, 0x4A, 0x35, 0x29 }, // lora-10
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-11
  { 0x47, 0x27, 0xED, 0x19, 0x8F, 0x1C, 0x91, 0xFE, 0x85, 0xBB, 0xD7, 0xD5, 0x33, 0x67, 0xF2, 0x66 }, // lora-12
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-13
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-14
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-15
  { 0x94, 0xBD, 0xDD, 0xB6, 0x6B, 0x05, 0x43, 0x0E, 0xAF, 0x39, 0x69, 0x00, 0x2A, 0xA1, 0xF3, 0x98 }, // lora-16
  { 0xB8, 0x1F, 0x7F, 0x49, 0x4D, 0x0E, 0x21, 0x6C, 0x3D, 0x6C, 0x3A, 0x72, 0xE0, 0x49, 0x85, 0x23 }, // lora-17
  { 0x25, 0x00, 0x7C, 0xF2, 0xCB, 0xFB, 0x27, 0x3D, 0x43, 0xD9, 0xA2, 0x5C, 0xC4, 0x3F, 0xD7, 0xCE }, // lora-18
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-19
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-20
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-21
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-22
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-23
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-24
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-25
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-26
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-27
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-28
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-29
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // lora-30
  { 0xB4, 0x0F, 0xA5, 0x5E, 0x86, 0x8E, 0x60, 0xD0, 0x8B, 0xCB, 0x93, 0xE8, 0xEB, 0x8C, 0xE2, 0x76 }, // lora-31
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // lora-32
};

uint8_t devEui[8] = {0};   // Doit être défini ici, pas juste déclaré
uint8_t appKey[16] = {0};  // Doit être défini ici, pas juste déclaré

// LoRaWAN paramètres (à adapter selon ton réseau)
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t  loraWanClass = LORAWAN_CLASS;
uint32_t appTxDutyCycle =900000;// 10000;
bool overTheAirActivation = LORAWAN_NETMODE;
bool loraWanAdr = LORAWAN_ADR;
bool keepNet = LORAWAN_NET_RESERVE;
bool isTxConfirmed = LORAWAN_UPLINKMODE;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;
int maxtry = 50;



static void prepareTxFrame( uint8_t port )
{
	/*
  //en miliVolts

  //appData[0] = (uint8_t)(batteryVoltage >> 8);//décalage de 8 bit vers la droite, 
  //4803  donne 0001001011000011 sur 16bit donc reste les 8 1ers bits 00010010  soit 12 hex
  //Serial.println(appData[4]);
  
  //appData[1] = (uint8_t)batteryVoltage;
  //4803  donne 0001001011000011 donc si uint_8  on prend donc 8bit de droite 11000011 soit C3 hex
  // quand on concatène 12C3 hex dans un convertisseur hexa vers décimal, cela donne 4803 mV au décodage de la trame LORA
  //Serial.println(appData[5]);

  //calcul pour la température
  //sur lorawan on obtient 00 74 hex
  //on concatène 00 et 74 , on convertit hexa vers décimal et on obtient 116 décimal
  //on reprend la formule de Sylvain  t_byte = (t + 35 )*2  donc partie entière de t=116/2 moins 35 degrés soit 23 degrés

  //calcul pour la pesée 
  //sur lorawan on obtient 04 9B hex  pour une donnée de 1179 correspondant à un poids de 13800gr
  //ou 1C E0 pour 7392  correspondant à 85700gr
  //le rapport entre 85700gr et 7392 est un multiplicateur de 11,6
  //quand on multiplie par 11,6 une mesure convertie en décimal, on obtient le poids réel en grammes
  */
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);// pour activer l'alimentation des capteurs branchés dur Vext
  delay(500);

  // DHT22 température et humidité locales


  Serial.println("DHTxx test!");

  delay(1000);
  dht.begin(); 
  
  delay(1000);//pour laisser le temps de montée de la sonde
  float h = dht.readHumidity();  
  delay(1000);
  // Read temperature as Celsius
  float t = dht.readTemperature();
  uint8_t  t_byte = (t + 35 )*2;      //  à mieux commenter
  uint8_t h_byte = h*2;              // à mieux commenter

    
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print(" *C  ");
  Serial.print(t_byte,HEX);
  Serial.print(" Byte   ");
  Serial.print("Humidity : "); 
  Serial.print(h);
  Serial.print(" %\t  ");
  Serial.print(h_byte,HEX);
  Serial.println(" Byte   ");
                    
                    


  
  //sonde dc18b20 en externe******************************
  
  Serial.println("ds18b20 test!");

  delay(1000);  
  
  ds.begin();          // sonde activée

  delay(1000);
  ds.requestTemperatures();
  int tDs = ds.getTempCByIndex(0);
  uint8_t  tDs_byte = (tDs + 35 )*2;

  
  Serial.print(tDs);
  Serial.println( " C  ");
  Serial.print(tDs_byte,HEX);
  Serial.println(" Byte   ");

  //la tension batterie
  uint16_t batteryVoltage = getBatteryVoltage();
  Serial.print(batteryVoltage);
  Serial.println( " mV ");


  //les balances*******************************************************
  
    
  float Sample_weight;
    
  Hx711_N1.begin(PIN_HX711_N1_DATA_OUT,PIN_HX711_N1_SCK_AND_POWER_DOWN,64 );
  Sample_weight = Hx711_N1.get_units();                                          // For nothing library problem...
  Sample_weight = (offset_HX711_N1_ChannelA - Hx711_N1.get_units())/256*Weight_sensitivity;

  if (Sample_weight < 0) { 
      Sample_weight =0; 
      }
  else { 
      if (Sample_weight > 65535) { 
          Sample_weight = 65535; 
          }
      }

  unsigned int Weight_HX711_N1_Channel_A = Sample_weight;

  Hx711_N1.begin(PIN_HX711_N1_DATA_OUT,PIN_HX711_N1_SCK_AND_POWER_DOWN,32 );
  Sample_weight = Hx711_N1.get_units();                                          // For nothing library problem...
  Sample_weight = (offset_HX711_N1_ChannelB - Hx711_N1.get_units())/128*Weight_sensitivity;

  if (Sample_weight < 0) {
      Sample_weight =0; 
      }
      else { 
      if (Sample_weight > 65535) { 
          Sample_weight = 65535; 
          }
      }

  unsigned int Weight_HX711_N1_Channel_B = Sample_weight;



  Serial.print("Weight N1 Channel A : ");
  Serial.print(Weight_HX711_N1_Channel_A,DEC);
  Serial.print(" "); 
  Serial.print(Weight_HX711_N1_Channel_A,HEX);
  Serial.print(" "); 
  Serial.print(Weight_HX711_N1_Channel_A,BIN);
  Serial.print(" ");

  Serial.println();


  Serial.print("Weight N1 Channel B : ");
  Serial.print(Weight_HX711_N1_Channel_B,DEC);
  Serial.print(" "); 
  Serial.print(Weight_HX711_N1_Channel_B,HEX);
  Serial.print(" "); 
  Serial.print(Weight_HX711_N1_Channel_B,BIN);
  Serial.print(" ");

  Serial.println();
                  
  






  appDataSize = 12;// nombre total d'octets de la trame envoyée
  //et à changer selon le nombre de balances et capteurs

  appData[0] = (uint8_t)(batteryVoltage>>8);
  appData[1] = (uint8_t)batteryVoltage;

  appData[2] = (uint8_t)(t_byte>>8);
  appData[3] = (uint8_t)t_byte;


  appData[4] = (uint8_t)(h_byte>>8);
  appData[5] = (uint8_t)h_byte;

  appData[6] = (uint8_t)(tDs_byte>>8);
  appData[7] = (uint8_t)tDs_byte;

  appData[8] = (uint8_t)(Weight_HX711_N1_Channel_A>>8);
  appData[9] = (uint8_t)Weight_HX711_N1_Channel_A;

  appData[10] = (uint8_t)(Weight_HX711_N1_Channel_B>>8);
  appData[11] = (uint8_t)Weight_HX711_N1_Channel_B;


  digitalWrite(Vext, HIGH); // pour déconnecter les capteurs branchés dur Vext
  
}
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
void setup() {
    Serial.begin(9600);
    
 Serial.begin(9600);
  Serial.print("Code source : ");
  Serial.println(__FILE__);
  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();

  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(500);
    
    
    EEPROM.begin(64);

    // Lecture de la tare enregistrée en EEPROM
    EEPROM.get(0, offset_HX711_N1_ChannelA);
    EEPROM.get(sizeof(float), offset_HX711_N1_ChannelB);

    Serial.print("Tare A lue en EEPROM : "); Serial.println(offset_HX711_N1_ChannelA, 4);
    Serial.print("Tare B lue en EEPROM : "); Serial.println(offset_HX711_N1_ChannelB, 4);

    // Si la tare n'est pas initialisée, on la mesure et on l'enregistre
    if (isnan(offset_HX711_N1_ChannelA) || offset_HX711_N1_ChannelA == 0.0) {
        Hx711_N1.begin(PIN_HX711_N1_DATA_OUT, PIN_HX711_N1_SCK_AND_POWER_DOWN, 64);
        delay(500); // Ajoute ce délai pour laisser le HX711 se stabiliser
        offset_HX711_N1_ChannelA = Hx711_N1.get_units();
        EEPROM.put(0, offset_HX711_N1_ChannelA);
        EEPROM.commit();
        Serial.print("Initialisation tare HX711 Channel A : valeur brute = ");
        Serial.println(offset_HX711_N1_ChannelA, 4);
    }

    if (isnan(offset_HX711_N1_ChannelB) || offset_HX711_N1_ChannelB == 0.0) {
        Hx711_N1.begin(PIN_HX711_N1_DATA_OUT, PIN_HX711_N1_SCK_AND_POWER_DOWN, 32);
        offset_HX711_N1_ChannelB = Hx711_N1.get_units();
        EEPROM.put(sizeof(float), offset_HX711_N1_ChannelB);
        EEPROM.commit();
        Serial.print("Initialisation tare HX711 Channel B : valeur brute = ");
        Serial.println(offset_HX711_N1_ChannelB, 4);
    }


     // Initialisation des 5 entrées encodeur
  pinMode(ENCODER_PIN_0, INPUT_PULLDOWN);
  pinMode(ENCODER_PIN_1, INPUT_PULLDOWN);
  pinMode(ENCODER_PIN_2, INPUT_PULLDOWN);
  pinMode(ENCODER_PIN_3, INPUT_PULLDOWN);
  pinMode(ENCODER_PIN_4, INPUT_PULLDOWN);

  // Lecture de l'encodeur (5 bits)
  uint8_t encoderValue = 0;
  encoderValue |= (digitalRead(ENCODER_PIN_0) ? 1 : 0) << 0;
  encoderValue |= (digitalRead(ENCODER_PIN_1) ? 1 : 0) << 1;
  encoderValue |= (digitalRead(ENCODER_PIN_2) ? 1 : 0) << 2;
  encoderValue |= (digitalRead(ENCODER_PIN_3) ? 1 : 0) << 3;
  encoderValue |= (digitalRead(ENCODER_PIN_4) ? 1 : 0) << 4;

  Serial.print("Valeur brute encodeur : ");
  Serial.print(encoderValue, DEC);
  Serial.print(" (binaire : ");
  Serial.print(encoderValue, BIN);
  Serial.println(")");

  if (encoderValue < 1) encoderValue = 1;
  if (encoderValue > 32) encoderValue = 32;

  Serial.print("ID LoRa sélectionné : lora-");
  Serial.println(encoderValue);

  memcpy(devEui, devEuiTable[encoderValue - 1], 8);
  memcpy(appKey, appKeyTable[encoderValue - 1], 16);

  Serial.print("Module ID: ");
  Serial.println(encoderValue);
  Serial.print("devEui: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(devEui[i], HEX); Serial.print(" ");
  }
  Serial.println();
  Serial.print("appKey: ");
  for (int i = 0; i < 16; i++) {
    Serial.print(appKey[i], HEX); Serial.print(" ");
  }
  Serial.println();


    Serial.flush();
}
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
void loop()
{
	 switch (deviceState) {
    case DEVICE_STATE_INIT:
      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;
    case DEVICE_STATE_JOIN:
      LoRaWAN.join();
      break;
    case DEVICE_STATE_SEND:
      prepareTxFrame(appPort);
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    case DEVICE_STATE_CYCLE:
      txDutyCycleTime = appTxDutyCycle + randr(0, APP_TX_DUTYCYCLE_RND);
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    case DEVICE_STATE_SLEEP:
      LoRaWAN.sleep();
      break;
    default:
      deviceState = DEVICE_STATE_INIT;
      break;
  }
}
////////////////////////////////////////////////////////////
