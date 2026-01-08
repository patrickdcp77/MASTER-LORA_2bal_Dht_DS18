/*
  Ce programme permet de lire les valeurs de tare (offset HX711) enregistrées en EEPROM,
  puis d'effacer complètement la mémoire EEPROM du CubeCell.

  - Au démarrage, il attend 10 secondes pour permettre à l'utilisateur d'ouvrir le terminal série
    et de visualiser les valeurs de tare actuellement stockées.
  - Il lit et affiche les valeurs float de tare pour Channel A (adresse 0) et Channel B (adresse 4).
  - Teste le HX711 pour diagnostiquer les problèmes d'étalonnage
  - Ensuite, il efface les 64 octets de l'EEPROM en les mettant à zéro et valide l'effacement avec EEPROM.commit().
  - Vérifie que l'EEPROM a bien été vidée
  - Affiche la valeur brute du HX711 pour diagnostic matériel

  Utiliser ce programme avant une nouvelle calibration ou pour réinitialiser la mémoire de tare.
*/
#include "Arduino.h"
#include <EEPROM.h>
#include "HX711.h"

#define PIN_HX711_DATA GPIO6
#define PIN_HX711_SCK GPIO7

HX711 hx711;

void setup() {
    Serial.begin(9600);
    EEPROM.begin(64);

    Serial.println("\n========================================");
    Serial.println("PROGRAMME D'EFFACEMENT ET DIAGNOSTIC ROM");
    Serial.println("========================================");
    
    // !! IMPORTANT: Activer VEXT pour alimenter le HX711
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);  // LOW = activé
    delay(500);  // Attendre stabilisation alimentation
    
    Serial.println("Attente 10 secondes avant lecture et effacement...");
    delay(10000); // Attente de 10 secondes

    Serial.println("\n[1/4] LECTURE DE L'EEPROM ACTUELLE");
    float tareA, tareB;
    EEPROM.get(0, tareA);
    EEPROM.get(sizeof(float), tareB);

    Serial.print("Tare A lue en EEPROM : "); Serial.println(tareA, 4);
    Serial.print("Tare B lue en EEPROM : "); Serial.println(tareB, 4);

    Serial.println("\n[2/4] DIAGNOSTIC HX711");
    Serial.println("Initialisation HX711 (DATA=GPIO6, SCK=GPIO7)...");
    
    // Test GPIO avant HX711
    pinMode(PIN_HX711_DATA, INPUT);
    pinMode(PIN_HX711_SCK, OUTPUT);
    digitalWrite(PIN_HX711_SCK, HIGH);
    delay(100);
    
    Serial.print("État GPIO6 (DATA) : "); Serial.println(digitalRead(PIN_HX711_DATA));
    Serial.print("GPIO7 (SCK) : "); Serial.println(digitalRead(PIN_HX711_SCK));
    
    hx711.begin(PIN_HX711_DATA, PIN_HX711_SCK, 64);
    
    Serial.println("Attente 3 secondes pour stabilisation...");
    delay(3000);
    
    // !! IMPORTANT: Lecture dummy pour initialiser la librairie
    float dummy = hx711.get_units();
    Serial.print("Lecture dummy (initialization) : "); Serial.println(dummy, 2);
    delay(1000);

    if (hx711.is_ready()) {
        Serial.println("✓ HX711 DÉTECTÉ - Lecture de la valeur brute...");
        delay(1000);
        
        for (int i = 0; i < 5; i++) {
            if (hx711.is_ready()) {
                float raw = hx711.get_units();
                Serial.print("  Mesure "); Serial.print(i+1); 
                Serial.print(" : "); Serial.println(raw, 2);
            } else {
                Serial.println("  ✗ HX711 pas prêt!");
            }
            delay(500);
        }
        
        // Affichage supplémentaire
        Serial.println("\n--- ANALYSE DES RÉSULTATS ---");
        if (hx711.is_ready()) {
            Serial.println("✓ HX711 est prêt");
        } else {
            Serial.println("✗ PROBLÈME: HX711 n'est PAS prêt");
        }
    } else {
        Serial.println("✗ HX711 NON DÉTECTÉ - Problème matériel");
        Serial.println("Vérifiez:");
        Serial.println("  1) Câbles DATA (GPIO6) et SCK (GPIO7) bien connectés");
        Serial.println("  2) Alimentation du HX711 (VCC et GND)");
        Serial.println("  3) Jauges de contrainte (E+, E-, A+, A-, B+, B-)");
    }

    Serial.println("\n[3/4] EFFACEMENT DE L'EEPROM");
    for (int i = 0; i < 64; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("Effacement effectué + commit validé");

    Serial.println("\n[4/4] VÉRIFICATION DE L'EFFACEMENT");
    float tareA_verify, tareB_verify;
    EEPROM.get(0, tareA_verify);
    EEPROM.get(sizeof(float), tareB_verify);
    
    if (tareA_verify == 0.0 && tareB_verify == 0.0) {
        Serial.println("✓ EEPROM CORRECTEMENT VIDÉE");
    } else {
        Serial.println("✗ PROBLÈME : L'EEPROM n'a pas été vidée correctement");
        Serial.print("  Tare A vérifiée : "); Serial.println(tareA_verify, 4);
        Serial.print("  Tare B vérifiée : "); Serial.println(tareB_verify, 4);
    }

    Serial.println("\n========================================");
    Serial.println("Prêt pour le test du code principal");
    Serial.println("Mettez la balance VIDE avant de charger le code");
    Serial.println("========================================\n");
}

void loop() {}