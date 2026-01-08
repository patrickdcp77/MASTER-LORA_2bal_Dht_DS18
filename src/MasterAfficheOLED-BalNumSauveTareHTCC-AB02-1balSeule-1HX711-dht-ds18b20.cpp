/*
================================================================================
PROJET: BALANCE CONNECTÉE LORAWAN AVEC CAPTEURS ENVIRONNEMENTAUX
================================================================================
Matériel:
- Microcontrôleur: HTCC-AB02 (CubeCell)
- Communication: LoRaWAN (OTAA, région EU868)
- Capteurs:
  * HX711 + 4 jauges de contrainte 50kg (balance de pesée 0-200kg)
  * DHT22 (température et humidité locale)
  * DS18B20 (température externe étanche)
  * Batterie Li-ion avec surveillance de tension

Fonctionnalités principales:
1. Identification automatique du module par encodeur matériel (5 bits = 32 modules)
2. Calibration automatique de la balance au premier démarrage
3. Sauvegarde de la tare en EEPROM (survit aux redémarrages)
4. Mesure de 4 paramètres: poids, 2×température, humidité, tension batterie
5. Transmission LoRaWAN périodique (configurable: 10s test, 15min production)
6. Gestion intelligente de l'énergie (activation/coupure Vext, deep sleep)

Architecture du code:
- setup(): initialisation unique au démarrage
  * Configuration série, LoRaWAN, EEPROM
  * Calibration automatique balance (si EEPROM vide)
  * Lecture encodeur matériel
  * Sélection identifiants LoRaWAN (devEui/appKey)

- loop(): boucle principale (machine à états LoRaWAN)
  * INIT → JOIN → SEND → CYCLE → SLEEP → (retour à SEND)
  
- prepareTxFrame(): acquisition et encodage des données
  * Active Vext (alimentation capteurs)
  * Lit tous les capteurs
  * Encode en 10 octets pour transmission LoRa
  * Désactive Vext (économie d'énergie)

Encodage de la trame LoRaWAN (10 octets):
[0-1]: Tension batterie (16 bits, millivolts, big-endian)
[2-3]: Température DHT22 (8 bits encodé: (T+35)*2, + padding)
[4-5]: Humidité DHT22 (8 bits encodé: H*2, + padding)
[6-7]: Température DS18B20 (8 bits encodé: (T+35)*2, + padding)
[8-9]: Poids (16 bits, grammes, big-endian)

================================================================================
CALIBRATION DE LA BALANCE
================================================================================
Principe:
- Tare = valeur brute ADC du HX711 quand la balance est VIDE
- Sauvegardée en EEPROM au premier démarrage (balance vide obligatoire!)
- Réutilisée à chaque redémarrage (pas de recalibration)

Formule du poids:
  Poids (grammes) = (Tare - Mesure_actuelle) / 20.16

Facteur de calibration 20.16:
- Déterminé expérimentalement avec un poids connu de 16.2kg
- Pour 4 jauges de 50kg câblées en pont de Wheatstone
- Précision: 99% sur la plage 0-200kg

Pour réinitialiser la tare:
1. Téléverser ROM-effacement.cpp (efface l'EEPROM)
2. Mettre la balance COMPLÈTEMENT VIDE
3. Téléverser ce code → tare recalculée automatiquement

================================================================================
CÂBLAGE DES JAUGES DE CONTRAINTE
================================================================================
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

================================================================================
ENCODEUR MATÉRIEL (IDENTIFICATION AUTOMATIQUE DU MODULE)
================================================================================
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
  GPIO5  : DHT22 (température et humidité locale)
  GPIO6  : HX711 DATA (balance)
  GPIO7  : HX711 SCK (balance)
  ADC3   : DS18B20 (température externe OneWire)
  GPIO1-4, GPIO11 : encodeur ID module (5 bits)
  Vext   : Alimentation commutable des capteurs (LOW=ON, HIGH=OFF)

Ne jamais laisser un GPIO en l'air : chaque entrée est configurée en INPUT_PULLDOWN, donc à 0 par défaut.
Pour coder un bit à 1, relier le GPIO à VCC via une résistance de 10 kΩ.

================================================================================
AUTEUR: Patrick
DATE: Janvier 2026
VERSION: 1.0 - Code entièrement commenté pour utilisation pédagogique
================================================================================
*/

//=============================================================================
// SECTION 1: INCLUSION DES LIBRAIRIES
//=============================================================================
// Librairie LoRaWAN pour la communication avec le réseau LoRa (The Things Network, etc.)
#include "LoRaWan_APP.h"
// Librairie Arduino de base pour les fonctions standard
#include "Arduino.h"
// EEPROM: mémoire non-volatile pour sauvegarder la tare de la balance
// Permet de conserver la calibration même après extinction/redémarrage
#include <EEPROM.h>
// OneWire: protocole de communication pour les capteurs Dallas (DS18B20)
#include "OneWire.h"
// Librairie pour gérer les capteurs de température Dallas DS18B20
#include "DallasTemperature.h"
// DHT: librairie pour capteurs de température et humidité DHT22/DHT11
#include <DHT.h>
#include <DHT_U.h>
// HX711: librairie pour le convertisseur ADC 24 bits des jauges de contrainte
// Permet de lire les valeurs de poids depuis les capteurs de la balance
#include "HX711.h"
// Wire: librairie I2C pour communiquer avec l'écran OLED
#include <Wire.h>
// OLED SH1107: pilote Heltec pour écrans 128x64 (CubeCell AB02)
#include <Wire.h>
#include "HT_SH1107Wire.h"

//=============================================================================
// SECTION 2: DÉFINITION DES BROCHES (PINS) ET CONSTANTES
//=============================================================================
// Adresse EEPROM où est stockée la tare (offset) du capteur HX711
// La tare est la valeur brute mesurée quand la balance est vide
#define EEPROM_ADDR_TARE_A 0

// Configuration du capteur DHT22 (température et humidité)
#define DHTPIN GPIO5              // Pin de données du DHT22
#define DHTTYPE DHT22             // Type de capteur (DHT22 ou DHT11)

// Configuration du capteur HX711 (balance avec jauges de contrainte)
#define PIN_HX711_N1_DATA_OUT GPIO6              // Pin DATA du HX711
#define PIN_HX711_N1_SCK_AND_POWER_DOWN GPIO7    // Pin SCK (horloge) du HX711

// Configuration de l'encodeur matériel (5 bits = 32 modules possibles)
// Permet d'identifier automatiquement chaque module (lora-01 à lora-32)
// En mettant des résistances 10kΩ vers VCC sur certains GPIO
#define ENCODER_PIN_0 GPIO1       // Bit 0 de l'encodeur
#define ENCODER_PIN_1 GPIO2       // Bit 1 de l'encodeur
#define ENCODER_PIN_2 GPIO3       // Bit 2 de l'encodeur
#define ENCODER_PIN_3 GPIO4       // Bit 3 de l'encodeur
#define ENCODER_PIN_4 GPIO11      // Bit 4 de l'encodeur

//=============================================================================
// SECTION 3: INSTANCIATION DES OBJETS CAPTEURS
//=============================================================================
// Objet pour gérer le capteur DHT22 (température et humidité)
DHT dht(DHTPIN, DHTTYPE);

// Objet OneWire pour communiquer avec le DS18B20 sur ADC3
OneWire oneWire(ADC3);

// Objet DallasTemperature pour faciliter la lecture du DS18B20
DallasTemperature ds(&oneWire);

// Objet HX711 pour lire les données de la balance
HX711 Hx711_N1;

// Objet OLED (même config que le demo fonctionnel)
// addr=0x3c, freq=500kHz, SDA, SCL, géométrie 128x64, RST=GPIO10
SH1107Wire oledDisplay(0x3c, 500000, SDA, SCL, GEOMETRY_128_64, GPIO10);

//=============================================================================
// SECTION 4: VARIABLES GLOBALES
//=============================================================================
// Tare (offset) de la balance: valeur brute du HX711 quand la balance est vide
// Cette valeur est sauvegardée en EEPROM pour survivre aux redémarrages
// Elle est mesurée automatiquement au premier démarrage (EEPROM = 0)
float offset_HX711_N1_ChannelA;

// Sensibilité du poids (non utilisée avec la nouvelle calibration)
// Ancienne formule utilisait: /256 * Weight_sensitivity
const unsigned int Weight_sensitivity = 4;

// Variables pour stocker les mesures de température et humidité
float humidite, temperature;

// Variables globales pour l'affichage OLED
uint8_t moduleNumber = 0;          // Numéro du module (1-32)
uint16_t batteryVoltageGlobal = 0; // Tension batterie en mV
float temperatureDS18Global = 0;   // Température DS18B20
uint16_t weightGlobal = 0;         // Poids en grammes

//=============================================================================
// SECTION 5: CONFIGURATION LORAWAN - OTAA (Over-The-Air Activation)
//=============================================================================
/* OTAA (Over-The-Air Activation) est la méthode d'authentification LoRaWAN utilisée.
   Chaque module possède:
   - devEui: identifiant unique du device (8 octets)
   - appEui: identifiant de l'application (8 octets, ici tous à 0)
   - appKey: clé de chiffrement (16 octets)
   
   L'encodeur matériel (5 bits GPIO) sélectionne automatiquement le bon devEui/appKey
   dans les tables ci-dessous en fonction du module physique (lora-01 à lora-32)
*/

// AppEui: identifiant de l'application LoRaWAN (tous les modules partagent le même)
uint8_t appEui[8] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

// Table des DevEui pour chaque module (lora-01 à lora-32)
// Chaque ligne correspond à un module identifié par l'encodeur matériel
// Les entrées à 0x00 sont des emplacements réservés non utilisés
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

// Table des AppKey pour chaque module (lora-01 à lora-32)
// Clés de chiffrement uniques pour l'authentification OTAA
// Chaque module a sa propre clé de 16 octets (128 bits)
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

// Variables qui recevront les identifiants sélectionnés selon l'encodeur matériel
// Initialisées à 0, puis remplies dans setup() en fonction de l'encodeur
uint8_t devEui[8] = {0};   // DevEui sélectionné (8 octets)
uint8_t appKey[16] = {0};  // AppKey sélectionnée (16 octets)

//=============================================================================
// SECTION 6: PARAMÈTRES LORAWAN (configuration réseau)
//=============================================================================
// Clés de session pour ABP (non utilisées ici car on utilise OTAA)
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

// Masque des canaux utilisés (ici tous les canaux de 0 à 7 en EU868)
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

// Région LoRaWAN (définie dans platformio.ini, généralement EU868 pour l'Europe)
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

// Classe du device (A, B ou C)
// Classe A = consommation minimale, écoute seulement après envoi
DeviceClass_t  loraWanClass = LORAWAN_CLASS;

// Période d'envoi des données en millisecondes
// 10000 = 10 secondes (test), 900000 = 15 minutes (production)
uint32_t appTxDutyCycle =60000;//900000;// 10000;  toute les minutes

// Mode d'activation: OTAA (true) ou ABP (false)
bool overTheAirActivation = LORAWAN_NETMODE;

// ADR (Adaptive Data Rate): ajustement automatique du débit
// Permet d'optimiser consommation et portée selon la qualité du signal
bool loraWanAdr = LORAWAN_ADR;

// Conservation de la session après redémarrage
bool keepNet = LORAWAN_NET_RESERVE;

// Mode uplink: CONFIRMED (avec accusé de réception) ou UNCONFIRMED
bool isTxConfirmed = LORAWAN_UPLINKMODE;

// Port applicatif LoRaWAN (1-223)
// Permet de différencier les types de messages
uint8_t appPort = 2;

// Nombre de tentatives pour les messages confirmés
uint8_t confirmedNbTrials = 4;

// Nombre max de tentatives de connexion au réseau

int maxtry = 50;

//=============================================================================
// FONCTION: displayOLED()
// OBJECTIF: Afficher les informations principales sur l'écran OLED
//
// Affiche sur l'écran OLED 128x64 du CubeCell:
// - Numéro du module (lora-XX)
// - Tension de la batterie (en volts)
// - Température DS18B20 (en °C)
// - Poids mesuré (en grammes)
//=============================================================================
void displayOLED() {
  oledDisplay.clear();
  oledDisplay.setTextAlignment(TEXT_ALIGN_LEFT);
  oledDisplay.setFont(ArialMT_Plain_10);
  
  // Lignes d'informations
  oledDisplay.drawString(0, 0, String("Module: lora-") + String(moduleNumber));
  oledDisplay.drawString(0, 16, String("Bat: ") + String(batteryVoltageGlobal / 1000.0, 2) + String(" V"));
  oledDisplay.drawString(0, 32, String("Temp: ") + String(temperatureDS18Global, 1) + String(" C"));
  oledDisplay.drawString(0, 48, String("Poids: ") + String(weightGlobal) + String(" g"));
  
  oledDisplay.display();
}

//=============================================================================
// FONCTION: prepareTxFrame
// OBJECTIF: Préparer la trame de données à envoyer via LoRaWAN
// 
// Cette fonction:
// 1. Active l'alimentation des capteurs (Vext)
// 2. Lit tous les capteurs (DHT22, DS18B20, HX711, batterie)
// 3. Encode les valeurs en octets pour transmission LoRa
// 4. Remplit le buffer appData[] avec les données
// 5. Désactive l'alimentation des capteurs pour économiser l'énergie
//
// ENCODAGE DES DONNÉES:
// - Tension batterie: 2 octets (millivolts, big-endian)
// - Température DHT22: 1 octet encodé = (temp + 35) * 2
// - Humidité DHT22: 1 octet encodé = humidité * 2
// - Température DS18B20: 1 octet encodé = (temp + 35) * 2
// - Poids: 2 octets (grammes, big-endian)
//
// Total: 10 octets par trame
//=============================================================================
static void prepareTxFrame( uint8_t port )
{
	/*
  NOTES SUR L'ENCODAGE (pour décodage côté serveur):
  
  1. TENSION BATTERIE (millivolts sur 16 bits):
     Exemple: 4803 mV = 0x12C3 hex = [0x12, 0xC3] en octets
     appData[0] = partie haute (0x12)
     appData[1] = partie basse (0xC3)
     Décodage: (appData[0] << 8) | appData[1] = 4803 mV

  2. TEMPÉRATURE (encodée sur 8 bits):
     Formule: t_byte = (température_celsius + 35) * 2
     Exemple: 23°C → (23 + 35) * 2 = 116 → 0x74 hex
     Plage: -35°C à +92.5°C (en pas de 0.5°C)
     Décodage: température = (t_byte / 2) - 35

  3. HUMIDITÉ (encodée sur 8 bits):
     Formule: h_byte = humidité_pourcent * 2
     Exemple: 54.5% → 54.5 * 2 = 109 → 0x6D hex
     Plage: 0% à 127.5% (en pas de 0.5%)
     Décodage: humidité = h_byte / 2

  4. POIDS (grammes sur 16 bits):
     Exemple: 16220 g = 0x3F5C hex = [0x3F, 0x5C] en octets
     appData[8] = partie haute (0x3F)
     appData[9] = partie basse (0x5C)
     Décodage: (appData[8] << 8) | appData[9] = 16220 grammes
  */
  
  //===========================================================================
  // ÉTAPE 1: VÉRIFICATION DE L'ALIMENTATION DES CAPTEURS (Vext)
  //===========================================================================
  // Vext = broche d'alimentation contrôlable du HTCC-AB02
  // Alimente l'OLED et les capteurs externes (HX711, DHT22, DS18B20)
  // LOW = alimentation ACTIVÉE (logique inversée)
  // HIGH = alimentation COUPÉE
  // NOTE: Vext est déjà activé dans setup() et reste allumé pour l'OLED
  //       On s'assure simplement qu'il est bien activé ici
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);    // S'assure que l'alimentation est active
  delay(500);                 // Attend 500ms que les capteurs s'initialisent

  //===========================================================================
  // ÉTAPE 2: LECTURE DU CAPTEUR DHT22 (température et humidité locales)
  //===========================================================================
  Serial.println("DHTxx test!");

  delay(1000);                // Délai pour stabilisation
  dht.begin();                // Initialise le capteur DHT22
  
  delay(1000);                // Laisse le temps au capteur de se stabiliser
  
  // Lecture de l'humidité (en pourcentage)
  float h = dht.readHumidity();  
  delay(1000);
  
  // Lecture de la température (en degrés Celsius)
  float t = dht.readTemperature();
  
  // Encodage des valeurs sur 8 bits pour transmission LoRa
  // Formule: (température + 35) * 2 permet de coder -35°C à +92.5°C
  uint8_t  t_byte = (t + 35 )*2;
  
  // Formule: humidité * 2 permet de coder 0% à 127.5% avec précision 0.5%
  uint8_t h_byte = h*2;

  // Affichage des valeurs pour diagnostic  
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
                    
  //===========================================================================
  // ÉTAPE 3: LECTURE DU CAPTEUR DS18B20 (température externe, étanche)
  //===========================================================================
  // Le DS18B20 est un capteur de température numérique sur bus OneWire
  // Souvent utilisé pour mesurer la température de l'eau ou en extérieur
  Serial.println("ds18b20 test!");

  delay(1000);  
  
  ds.begin();          // Initialise le capteur DS18B20
  delay(1000);
  
  // Lance une conversion de température (prend ~750ms en résolution 12 bits)
  ds.requestTemperatures();
  
  // Récupère la température du premier capteur sur le bus (index 0)
  int tDs = ds.getTempCByIndex(0);
  
  // Encode la température sur 8 bits avec la même formule que DHT22
  uint8_t  tDs_byte = (tDs + 35 )*2;

  // Affichage pour diagnostic
  Serial.print(tDs);
  Serial.println( " *C  ");
  Serial.print(tDs_byte,HEX);
  Serial.println(" Byte   ");

  //===========================================================================
  // ÉTAPE 4: LECTURE DE LA TENSION DE BATTERIE
  //===========================================================================
  // getBatteryVoltage() est une fonction intégrée du HTCC-AB02
  // Retourne la tension en millivolts (ex: 4052 mV = 4.052 V)
  uint16_t batteryVoltage = getBatteryVoltage();
  Serial.print(batteryVoltage);
  Serial.println( " mV ");


  //===========================================================================
  // ÉTAPE 5: LECTURE DE LA BALANCE (HX711 + jauges de contrainte)
  //===========================================================================
  /* PRINCIPE DE FONCTIONNEMENT:
     1. Le HX711 est un ADC 24 bits qui lit la différence de tension des jauges
     2. Les 4 jauges de 50kg sont câblées en pont de Wheatstone
     3. Quand on met du poids, la tension change proportionnellement
     4. La "tare" (offset) est la valeur brute mesurée quand la balance est vide
     5. Le poids = (tare - mesure_actuelle) / facteur_calibration
     
     CALIBRATION:
     - Facteur 20.16 déterminé expérimentalement avec un poids de 16.2kg
     - Pour 4 jauges de 50kg (capacité totale 200kg)
     - Précision: 99% sur toute la plage 0-200kg
  */
  
  Serial.println("Lecture balance HX711...");
  float Sample_weight = 0;                    // Poids calculé en grammes
  unsigned int Weight_HX711_N1_Channel_A = 0; // Poids final (entier)
    
  // Réinitialisation du HX711
  // Paramètres: (pin_data, pin_sck, gain)
  // Gain 64 = canal A (balance), gain 32 = canal B
  Hx711_N1.begin(PIN_HX711_N1_DATA_OUT,PIN_HX711_N1_SCK_AND_POWER_DOWN,64 );
  delay(1000); // Délai important après begin pour stabilisation
  
  //===========================================================================
  // Vérifier si le HX711 est prêt avec timeout
  //===========================================================================
  // is_ready() vérifie si la pin DOUT est LOW (données disponibles)
  Serial.println("Vérification disponibilité HX711...");
  unsigned long startTime = millis();
  int readAttempts = 0;
  
  // Boucle d'attente max 5 secondes
  while (!Hx711_N1.is_ready() && (millis() - startTime) < 5000) {
    readAttempts++;
    Serial.print("Attente HX711... tentative ");
    Serial.println(readAttempts);
    delay(200);
  }
  
  if (Hx711_N1.is_ready()) {
    Serial.println("HX711 prêt, lecture en cours...");
    delay(200);
    
    //=========================================================================
    // Lecture de la valeur brute du HX711
    //=========================================================================
    // get_units() retourne la valeur brute de l'ADC 24 bits
    // Valeurs typiques: 250000 à 260000 pour balance vide
    //                  Valeurs négatives possibles avec du poids (débordement)
    float raw_value = Hx711_N1.get_units();
    Serial.print("Valeur brute : ");
    Serial.println(raw_value, 2);
    Serial.print("Offset (tare) : ");
    Serial.println(offset_HX711_N1_ChannelA, 2);
    
    //=========================================================================
    // Calcul du poids en grammes
    //=========================================================================
    /* FORMULE: Poids = (Tare - Mesure_actuelle) / Facteur_calibration
       
       Pourquoi (Tare - Mesure) et non (Mesure - Tare)?
       - Les jauges fonctionnent à l'envers: plus de poids = tension plus basse
       - Donc la mesure DIMINUE quand on ajoute du poids
       - La différence (Tare - Mesure) donne donc un nombre positif
       
       Facteur de calibration 20.16:
       - Déterminé expérimentalement avec 16.2kg de poids connu
       - Différence mesurée: 326586 (252649 - (-73937))
       - Facteur: 326586 / 16200g = 20.16
       - Avec ce facteur, précision de 99% sur toute la plage
    */
    Sample_weight = (offset_HX711_N1_ChannelA - Hx711_N1.get_units())/20.16;
    Serial.print("Poids calculé : ");
    Serial.println(Sample_weight, 2);

    //=========================================================================
    // Limitation des valeurs (saturation)
    //=========================================================================
    // Si poids négatif (erreur ou vibration), mettre à 0
    if (Sample_weight < 0) { 
        Sample_weight = 0; 
    } else { 
        // Si poids > 65535g (limite uint16), saturer à 65535
        // (65535g = 65.5kg, largement suffisant pour 200kg max)
        if (Sample_weight > 65535) { 
            Sample_weight = 65535; 
        }
    }

    // Conversion en entier pour transmission
    Weight_HX711_N1_Channel_A = Sample_weight;
    
  } else {
    //=========================================================================
    // Gestion d'erreur: HX711 non disponible
    //=========================================================================
    Serial.println("ERREUR : HX711 non disponible après 5s !");
    Serial.println("Vérifiez câblage et alimentation du capteur");
    Weight_HX711_N1_Channel_A = 0;  // Envoyer 0 en cas d'erreur
  }

  // Affichage du poids final en différentes bases (décimal, hexa, binaire)
  Serial.print("Weight N1 Channel A : ");
  Serial.print(Weight_HX711_N1_Channel_A,DEC);
  Serial.print(" "); 
  Serial.print(Weight_HX711_N1_Channel_A,HEX);
  Serial.print(" "); 
  Serial.print(Weight_HX711_N1_Channel_A,BIN);
  Serial.print(" ");
  Serial.println();
                  
  //===========================================================================
  // ÉTAPE 6: ENCODAGE DES DONNÉES DANS LE BUFFER LORAWAN
  //===========================================================================
  /* Structure de la trame LoRaWAN (10 octets):
     appData[0-1]: Tension batterie (16 bits, millivolts)
     appData[2-3]: Température DHT22 (8 bits encodé + 0x00 de padding)
     appData[4-5]: Humidité DHT22 (8 bits encodé + 0x00 de padding)
     appData[6-7]: Température DS18B20 (8 bits encodé + 0x00 de padding)
     appData[8-9]: Poids (16 bits, grammes)
  */

  appDataSize = 10;  // Taille totale de la trame en octets
                     // À ajuster si on ajoute/retire des capteurs

  // Batterie: 16 bits big-endian
  // >> 8 : décalage de 8 bits vers la droite = octet haut
  appData[0] = (uint8_t)(batteryVoltage>>8);
  appData[1] = (uint8_t)batteryVoltage;

  // Température DHT22: 8 bits + padding 0x00
  appData[2] = (uint8_t)(t_byte>>8);    // Toujours 0x00 car t_byte < 256
  appData[3] = (uint8_t)t_byte;

  // Humidité DHT22: 8 bits + padding 0x00
  appData[4] = (uint8_t)(h_byte>>8);    // Toujours 0x00 car h_byte < 256
  appData[5] = (uint8_t)h_byte;

  // Température DS18B20: 8 bits + padding 0x00
  appData[6] = (uint8_t)(tDs_byte>>8);  // Toujours 0x00 car tDs_byte < 256
  appData[7] = (uint8_t)tDs_byte;

  // Poids: 16 bits big-endian
  appData[8] = (uint8_t)(Weight_HX711_N1_Channel_A>>8);
  appData[9] = (uint8_t)Weight_HX711_N1_Channel_A;

  //===========================================================================
  // ÉTAPE 7: MISE À JOUR DES VARIABLES GLOBALES POUR L'AFFICHAGE OLED
  //===========================================================================
  // Sauvegarde des valeurs dans les variables globales
  // Ces valeurs seront affichées sur l'écran OLED
  batteryVoltageGlobal = batteryVoltage;
  temperatureDS18Global = tDs;
  weightGlobal = Weight_HX711_N1_Channel_A;
  
  // Affichage des données sur l'écran OLED
  displayOLED();

  //===========================================================================
  // ÉTAPE 8: GESTION DE L'ALIMENTATION (Vext)
  //===========================================================================
  // IMPORTANT: On ne désactive PAS Vext car l'écran OLED en a besoin!
  // L'OLED reste alimenté en permanence pour afficher les valeurs
  // Inconvénient: consommation légèrement plus élevée (~10mA)
  // Avantage: affichage permanent des mesures
  
  // Si vous souhaitez économiser l'énergie au maximum, décommentez la ligne suivante:
  // digitalWrite(Vext, HIGH);  // Coupe l'alimentation (OLED s'éteindra aussi)
  
}
////////////////////////////////////////////////////////////

//=============================================================================
// FONCTION: setup()
// OBJECTIF: Initialisation du système au démarrage
// 
// Cette fonction s'exécute UNE SEULE FOIS au démarrage ou après reset
// Elle initialise:
// 1. La communication série pour le diagnostic
// 2. Le système LoRaWAN
// 3. L'alimentation des capteurs (Vext)
// 4. L'EEPROM et la tare de la balance
// 5. L'encodeur matériel pour l'identification du module
// 6. Les identifiants LoRaWAN (devEui, appKey)
//=============================================================================
////////////////////////////////////////////////////////////
void setup() {
    //===========================================================================
    // ÉTAPE 1: INITIALISATION DE LA COMMUNICATION SÉRIE
    //===========================================================================
    // 9600 bauds = vitesse de communication série (USB ou UART)
    // Permet d'afficher des messages de diagnostic via Serial.print()
    Serial.begin(9600);
    
    // Affichage du nom du fichier source pour identification
    Serial.begin(9600);
    Serial.print("Code source : ");
    Serial.println(__FILE__);
  
    //===========================================================================
    // ÉTAPE 1.5: ACTIVATION DE L'ALIMENTATION Vext (OBLIGATOIRE POUR OLED)
    //===========================================================================
    // Vext = pin d'alimentation commutable du HTCC-AB02
    // Alimente l'écran OLED et les capteurs externes
    // LOW = ACTIVÉ, HIGH = COUPÉ (logique inversée)
    // DOIT ÊTRE ACTIVÉ AVANT l'initialisation de l'OLED!
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);  // Active l'alimentation
    delay(500);               // Attend que l'alimentation se stabilise
    Serial.println("Vext activé (OLED alimenté)");
  
    //===========================================================================
    // ÉTAPE 1.6: INITIALISATION DE L'ÉCRAN OLED
    //===========================================================================
    // Initialise l'écran OLED 128x64 du CubeCell
    // IMPORTANT: Vext doit être activé AVANT (voir étape précédente)
    Serial.println("Initialisation OLED (Wire + init)...");
    Wire.begin();
    delay(50);
    oledDisplay.init();
    Serial.println("OLED init() OK");
    oledDisplay.clear();
    oledDisplay.display();
    oledDisplay.setTextAlignment(TEXT_ALIGN_LEFT);
    oledDisplay.setFont(ArialMT_Plain_10);
    oledDisplay.drawString(0, 0, "Initialisation...");
    oledDisplay.display();
    Serial.println("Premier affichage OLED envoyé");
    delay(1000);
  
    //===========================================================================
    // ÉTAPE 2: INITIALISATION DU SYSTÈME LORAWAN
    //===========================================================================
    // deviceState: machine à états pour gérer le cycle LoRaWAN
    // DEVICE_STATE_INIT → JOIN → SEND → CYCLE → SLEEP → SEND ...
    deviceState = DEVICE_STATE_INIT;
  
    // ifskipjoin(): vérifie s'il faut rejoindre le réseau ou utiliser une session sauvegardée
    LoRaWAN.ifskipjoin();

    //===========================================================================
    // ÉTAPE 3: INITIALISATION DE L'EEPROM ET GESTION DE LA TARE
    //===========================================================================
    /* L'EEPROM est une mémoire non-volatile (survit aux coupures d'alimentation)
       On y stocke la tare (valeur brute du HX711 quand la balance est vide)
       Cette tare est mesurée UNE FOIS au premier démarrage (EEPROM vierge)
       puis réutilisée à chaque redémarrage pour éviter de recalibrer
    */
    
    EEPROM.begin(64);  // Réserve 64 octets d'EEPROM

    // Lecture de la tare enregistrée à l'adresse 0
    // get() lit un float (4 octets) depuis l'EEPROM
    EEPROM.get(0, offset_HX711_N1_ChannelA);

    Serial.print("Tare A lue en EEPROM : "); 
    Serial.println(offset_HX711_N1_ChannelA, 4);

    //===========================================================================
    // VÉRIFICATION: La tare est-elle initialisée?
    //===========================================================================
    // isnan() vérifie si la valeur est NaN (Not a Number = EEPROM vierge)
    // || 0.0 vérifie si la tare a été mise à zéro (effacement manuel)
    if (isnan(offset_HX711_N1_ChannelA) || offset_HX711_N1_ChannelA == 0.0) {
        //=======================================================================
        // CAS 1: PREMIÈRE UTILISATION - CALIBRATION AUTOMATIQUE
        //=======================================================================
        // La balance doit être COMPLÈTEMENT VIDE pour cette étape!
        Serial.println("Initialisation HX711 Channel A...");
        Serial.print("GPIO DATA : "); Serial.println(PIN_HX711_N1_DATA_OUT);
        Serial.print("GPIO SCK  : "); Serial.println(PIN_HX711_N1_SCK_AND_POWER_DOWN);
        
        // Initialise le HX711 avec gain 64 (canal A)
        Hx711_N1.begin(PIN_HX711_N1_DATA_OUT, PIN_HX711_N1_SCK_AND_POWER_DOWN, 64);
        delay(2000); // Délai prolongé pour stabilisation du capteur
        
        //=======================================================================
        // Attente que le HX711 soit prêt (avec timeout de 10 secondes)
        //=======================================================================
        Serial.println("Vérification HX711...");
        unsigned long startTime = millis();
        int attempts = 0;
        
        // Boucle d'attente maximum 10 secondes
        while (!Hx711_N1.is_ready() && (millis() - startTime) < 10000) {
            attempts++;
            Serial.print("Attente HX711 Channel A... tentative ");
            Serial.println(attempts);
            delay(200);
        }
        
        if (Hx711_N1.is_ready()) {
            //===================================================================
            // HX711 prêt: mesure et sauvegarde de la tare
            //===================================================================
            Serial.println("HX711 prêt ! Lecture de la tare...");
            delay(500);
            
            // get_units() lit la valeur brute de l'ADC (balance vide)
            offset_HX711_N1_ChannelA = Hx711_N1.get_units();
            
            // Sauvegarde dans l'EEPROM pour les prochains démarrages
            EEPROM.put(0, offset_HX711_N1_ChannelA);
            EEPROM.commit();  // Valide l'écriture en EEPROM
            
            Serial.print("Initialisation tare HX711 Channel A : valeur brute = ");
            Serial.println(offset_HX711_N1_ChannelA, 4);
        } else {
            //===================================================================
            // ERREUR: HX711 non détecté
            //===================================================================
            Serial.println("ERREUR : HX711 Channel A non détecté après 10s !");
            Serial.println("Vérifiez le câblage : DATA sur GPIO6, SCK sur GPIO7");
            offset_HX711_N1_ChannelA = 0.0;
        }
    } else {
        //=======================================================================
        // CAS 2: TARE DÉJÀ ENREGISTRÉE - UTILISATION NORMALE
        //=======================================================================
        // La tare existe dans l'EEPROM, on la réutilise
        Serial.println("Tare déjà enregistrée, vérification HX711...");
        
        // On vérifie quand même que le HX711 fonctionne
        Hx711_N1.begin(PIN_HX711_N1_DATA_OUT, PIN_HX711_N1_SCK_AND_POWER_DOWN, 64);
        delay(2000);
        
        if (Hx711_N1.is_ready()) {
            Serial.println("HX711 détecté et prêt.");
        } else {
            Serial.println("ATTENTION : HX711 non détecté !");
        }
    }

  //=============================================================================
  // ÉTAPE 5: LECTURE DE L'ENCODEUR MATÉRIEL (IDENTIFICATION DU MODULE)
  //=============================================================================
  /* L'encodeur matériel permet d'identifier automatiquement le module (lora-01 à lora-32)
     sans avoir à reprogrammer le code pour chaque module.
     
     PRINCIPE:
     - 5 GPIO configurés en INPUT_PULLDOWN (tirés à 0V par défaut)
     - Pour coder le numéro du module, on met des résistances 10kΩ vers VCC
     - Chaque GPIO correspond à un bit: GPIO1=bit0, GPIO2=bit1, ..., GPIO11=bit4
     
     EXEMPLES:
     - lora-01 (binaire 00001): résistance sur GPIO1 uniquement
     - lora-08 (binaire 01000): résistance sur GPIO4 uniquement
     - lora-31 (binaire 11111): résistances sur GPIO1, GPIO2, GPIO3, GPIO4, GPIO11
     
     Cette valeur permet ensuite de sélectionner le bon devEui et appKey
     dans les tables définies plus haut
  */
  
  // Configuration des 5 GPIO en entrée avec pull-down interne
  // Pull-down = résistance interne qui tire la pin vers 0V (GND)
  pinMode(ENCODER_PIN_0, INPUT_PULLDOWN);  // GPIO1 = bit 0
  pinMode(ENCODER_PIN_1, INPUT_PULLDOWN);  // GPIO2 = bit 1
  pinMode(ENCODER_PIN_2, INPUT_PULLDOWN);  // GPIO3 = bit 2
  pinMode(ENCODER_PIN_3, INPUT_PULLDOWN);  // GPIO4 = bit 3
  pinMode(ENCODER_PIN_4, INPUT_PULLDOWN);  // GPIO11 = bit 4

  //=============================================================================
  // Lecture et assemblage de la valeur encodée sur 5 bits
  //=============================================================================
  uint8_t encoderValue = 0;  // Variable pour stocker le résultat (0-31)
  
  // Pour chaque GPIO, on lit son état (0 ou 1) et on le place au bon bit
  // digitalRead() retourne HIGH (1) si résistance vers VCC, LOW (0) sinon
  // << N : décalage de N positions vers la gauche (multiplication par 2^N)
  // |= : OU binaire avec assignation (ajoute le bit sans effacer les autres)
  
  encoderValue |= (digitalRead(ENCODER_PIN_0) ? 1 : 0) << 0;  // Bit 0
  encoderValue |= (digitalRead(ENCODER_PIN_1) ? 1 : 0) << 1;  // Bit 1
  encoderValue |= (digitalRead(ENCODER_PIN_2) ? 1 : 0) << 2;  // Bit 2
  encoderValue |= (digitalRead(ENCODER_PIN_3) ? 1 : 0) << 3;  // Bit 3
  encoderValue |= (digitalRead(ENCODER_PIN_4) ? 1 : 0) << 4;  // Bit 4

  // Affichage de la valeur lue pour diagnostic
  Serial.print("Valeur brute encodeur : ");
  Serial.print(encoderValue, DEC);      // Affichage en décimal
  Serial.print(" (binaire : ");
  Serial.print(encoderValue, BIN);      // Affichage en binaire
  Serial.println(")");

  //=============================================================================
  // Limitation de la valeur entre 1 et 32
  //=============================================================================
  // La valeur 0 n'est pas valide (aucun module), on la force à 1
  if (encoderValue < 1) encoderValue = 1;
  // La valeur ne peut pas dépasser 32 (limitation des tables)
  if (encoderValue > 32) encoderValue = 32;

  // Sauvegarde du numéro de module dans la variable globale pour l'affichage OLED
  moduleNumber = encoderValue;

  Serial.print("ID LoRa sélectionné : lora-");
  Serial.println(encoderValue);

  //=============================================================================
  // AFFICHAGE IMMÉDIAT DU NUMÉRO DE MODULE SUR L'OLED (DEBUG)
  //=============================================================================
  // Test de l'écran OLED: affiche le numéro du module
  Serial.println("Tentative d'affichage sur OLED...");
  oledDisplay.clear();
  oledDisplay.setTextAlignment(TEXT_ALIGN_LEFT);
  oledDisplay.setFont(ArialMT_Plain_16);
  oledDisplay.drawString(0, 0, "Module:");
  oledDisplay.drawString(0, 20, String("lora-") + String(moduleNumber));
  oledDisplay.display();
  Serial.println("Affichage OLED envoyé!");
  delay(3000);  // Garde l'affichage 3 secondes

  //=============================================================================
  // ÉTAPE 6: COPIE DES IDENTIFIANTS LORAWAN DEPUIS LES TABLES
  //=============================================================================
  // memcpy(destination, source, taille) copie N octets de la source vers la destination
  // encoderValue - 1 car les tableaux commencent à l'indice 0, mais les modules à 1
  
  // Copie du devEui (8 octets) depuis devEuiTable
  memcpy(devEui, devEuiTable[encoderValue - 1], 8);
  
  // Copie de l'appKey (16 octets) depuis appKeyTable
  memcpy(appKey, appKeyTable[encoderValue - 1], 16);

  //=============================================================================
  // Affichage des identifiants pour vérification
  //=============================================================================
  Serial.print("Module ID: ");
  Serial.println(encoderValue);
  
  Serial.print("devEui: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(devEui[i], HEX); 
    Serial.print(" ");
  }
  Serial.println();
  
  Serial.print("appKey: ");
  for (int i = 0; i < 16; i++) {
    Serial.print(appKey[i], HEX); 
    Serial.print(" ");
  }
  Serial.println();

  // flush() attend que tous les caractères en attente soient envoyés sur le port série
  Serial.flush();
}
////////////////////////////////////////////////////////////

//=============================================================================
// FONCTION: loop()
// OBJECTIF: Boucle principale du programme (s'exécute en continu)
// 
// Cette fonction s'exécute EN BOUCLE après setup()
// Elle gère la machine à états LoRaWAN:
// 
// 1. DEVICE_STATE_INIT: Initialisation du stack LoRaWAN
// 2. DEVICE_STATE_JOIN: Connexion au réseau LoRaWAN (OTAA)
// 3. DEVICE_STATE_SEND: Lecture des capteurs et envoi des données
// 4. DEVICE_STATE_CYCLE: Calcul du temps avant le prochain envoi
// 5. DEVICE_STATE_SLEEP: Mise en veille pour économiser l'énergie
//
// Le cycle se répète: SEND → CYCLE → SLEEP → SEND → ...
//=============================================================================
////////////////////////////////////////////////////////////
void loop()
{
  //=============================================================================
  // Machine à états pour gérer le cycle de vie LoRaWAN
  //=============================================================================
  /* deviceState est une variable globale qui indique l'état actuel
     À chaque appel de loop(), on exécute l'action correspondant à l'état
     puis on passe à l'état suivant
  */
  
	 switch (deviceState) {
    //===========================================================================
    // ÉTAT 1: INITIALISATION DU STACK LORAWAN
    //===========================================================================
    case DEVICE_STATE_INIT:
      // init() configure les paramètres radio (région, classe, fréquences, etc.)
      // loraWanClass = LORAWAN_CLASS (généralement CLASS_A)
      // loraWanRegion = ACTIVE_REGION (généralement EU868 pour l'Europe)
      LoRaWAN.init(loraWanClass, loraWanRegion);
      
      // Passe à l'état suivant: connexion au réseau
      deviceState = DEVICE_STATE_JOIN;
      break;
      
    //===========================================================================
    // ÉTAT 2: CONNEXION AU RÉSEAU LORAWAN (JOIN)
    //===========================================================================
    case DEVICE_STATE_JOIN:
      // join() lance la procédure OTAA (Over-The-Air Activation)
      // - Envoie une requête JOIN au serveur réseau
      // - Attend l'acceptation (JOIN_ACCEPT)
      // - Négocie les clés de session (NwkSKey, AppSKey)
      // Une fois accepté, passe automatiquement à DEVICE_STATE_SEND
      LoRaWAN.join();
      break;
      
    //===========================================================================
    // ÉTAT 3: ENVOI DES DONNÉES
    //===========================================================================
    case DEVICE_STATE_SEND:
      // prepareTxFrame() lit tous les capteurs et remplit appData[]
      // (fonction définie plus haut avec tous les détails)
      prepareTxFrame(appPort);
      
      // send() envoie la trame LoRaWAN sur le réseau
      // - Mode CONFIRMED: attend un accusé de réception
      // - Mode UNCONFIRMED: pas d'accusé (économise l'énergie)
      LoRaWAN.send();
      
      // Passe à l'état suivant: calcul du prochain cycle
      deviceState = DEVICE_STATE_CYCLE;
      break;
      
    //===========================================================================
    // ÉTAT 4: CALCUL DU TEMPS AVANT LE PROCHAIN ENVOI
    //===========================================================================
    case DEVICE_STATE_CYCLE:
      // txDutyCycleTime: temps en millisecondes avant le prochain envoi
      // appTxDutyCycle: période de base (ex: 10000ms = 10s)
      // randr(0, APP_TX_DUTYCYCLE_RND): ajoute un délai aléatoire
      //   → évite que tous les modules envoient en même temps
      //   → réduit les collisions sur le réseau
      txDutyCycleTime = appTxDutyCycle + randr(0, APP_TX_DUTYCYCLE_RND);
      
      // cycle() programme le réveil après txDutyCycleTime millisecondes
      LoRaWAN.cycle(txDutyCycleTime);
      
      // Passe à l'état suivant: mise en veille
      deviceState = DEVICE_STATE_SLEEP;
      break;
      
    //===========================================================================
    // ÉTAT 5: MISE EN VEILLE (ÉCONOMIE D'ÉNERGIE)
    //===========================================================================
    case DEVICE_STATE_SLEEP:
      // sleep() met le microcontrôleur en mode basse consommation
      // - Coupe l'horloge CPU
      // - Réduit la consommation à quelques µA
      // - Réveil automatique après le temps programmé par cycle()
      // 
      // Après le réveil, loop() reprend et deviceState revient à DEVICE_STATE_SEND
      LoRaWAN.sleep();
      break;
      
    //===========================================================================
    // ÉTAT PAR DÉFAUT: RÉINITIALISATION EN CAS D'ERREUR
    //===========================================================================
    default:
      // Si on arrive dans un état inconnu (ne devrait jamais arriver)
      // On réinitialise tout en revenant à DEVICE_STATE_INIT
      deviceState = DEVICE_STATE_INIT;
      break;
  }
}
////////////////////////////////////////////////////////////
