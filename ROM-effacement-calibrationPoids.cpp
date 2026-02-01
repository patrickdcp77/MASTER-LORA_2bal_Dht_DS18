/*
    PROGRAMME D'EFFACEMENT ET CALIBRATION ROM

    Fonctionnalités :
    1. Lit les valeurs de tare enregistrées en EEPROM
    2. Calibre la balance avec un poids de 16kg
         - Mesure la tare (balance vide)
         - Demande de mettre 16kg
         - Mesure la valeur avec 16kg
         - Calcule le facteur de calibration
         - Affiche le facteur à noter dans le code
    3. Efface complètement l'EEPROM (tares)

    Procédure :
    1. Balance VIDE au démarrage
    2. À l'affichage "Mettez 16kg", posez exactement 16kg
    3. Notez le FACTEUR affiché et reportez-le dans calibrationFactorTable
    4. L'EEPROM est effacée à la fin
*/

    //***************************************************************************************** */
    // !! IMPORTANT: Activer VEXT pour alimenter le HX711
    //***************************************************************************************** */


#include "Arduino.h"
#include <EEPROM.h>
#include "HX711.h"

#define PIN_HX711_DATA GPIO6
#define PIN_HX711_SCK GPIO7

HX711 hx711;
float tare_value = 0.0f;

void setup() {
    Serial.begin(9600);
    EEPROM.begin(64);

    Serial.println("\n========================================");
    Serial.println("EFFACEMENT ET CALIBRATION ROM");
    Serial.println("========================================");
    //***************************************************************************************** */
    // !! IMPORTANT: Activer VEXT pour alimenter le HX711
    //***************************************************************************************** */
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);  // LOW = activé
    delay(500);  // Attendre stabilisation alimentation
    
    Serial.println("Attente 10 secondes...");
    delay(10000); // Attente de 10 secondes

    Serial.println("\n[1/5] LECTURE DE L'EEPROM ACTUELLE");
    float tareA, tareB;
    EEPROM.get(0, tareA);
    EEPROM.get(sizeof(float), tareB);

    Serial.print("Tare A lue en EEPROM : "); Serial.println(tareA, 4);
    Serial.print("Tare B lue en EEPROM : "); Serial.println(tareB, 4);

    Serial.println("\n[2/5] INITIALISATION HX711");
    Serial.println("Initialisation HX711 (DATA=GPIO6, SCK=GPIO7)...");
    
    // Test GPIO avant HX711
    pinMode(PIN_HX711_DATA, INPUT);
    pinMode(PIN_HX711_SCK, OUTPUT);
    digitalWrite(PIN_HX711_SCK, HIGH);
    delay(100);
    
    hx711.begin(PIN_HX711_DATA, PIN_HX711_SCK, 64);
    
    Serial.println("Attente 3 secondes pour stabilisation...");
    delay(3000);
    
    // !! IMPORTANT: Lecture dummy pour initialiser la librairie
    float dummy = hx711.get_units();
    Serial.print("Lecture dummy (initialization) : "); Serial.println(dummy, 2);
    delay(1000);

    if (!hx711.is_ready()) {
        Serial.println("✗ HX711 NON DÉTECTÉ !");
        while (1) {}
    }

    Serial.println("✓ HX711 DÉTECTÉ");

    Serial.println("\n[3/5] MESURE DE LA TARE (BALANCE VIDE)");
    Serial.println("Assurez-vous que la balance est COMPLÈTEMENT VIDE !");
    delay(2000);

    float sum_tare = 0.0f;
    for (int i = 0; i < 5; i++) {
        float reading = hx711.get_units();
        Serial.print("  Lecture "); Serial.print(i + 1);
        Serial.print(" : "); Serial.println(reading, 2);
        sum_tare += reading;
        delay(500);
    }

    tare_value = sum_tare / 5.0f;
    Serial.print("✓ TARE MESURÉE : "); Serial.println(tare_value, 4);

    Serial.println("\n[4/5] MESURE AVEC 16kg");
    Serial.println(">>> METTEZ EXACTEMENT 16kg SUR LA BALANCE <<<");
    Serial.println("Attente 10 secondes...");
    delay(10000);

    float sum_16kg = 0.0f;
    for (int i = 0; i < 5; i++) {
        float reading = hx711.get_units();
        Serial.print("  Lecture "); Serial.print(i + 1);
        Serial.print(" : "); Serial.println(reading, 2);
        sum_16kg += reading;
        delay(500);
    }

    float value_16kg = sum_16kg / 5.0f;
    Serial.print("✓ VALEUR AVEC 16kg : "); Serial.println(value_16kg, 4);

    float difference = tare_value - value_16kg;
    float facteur = difference / 16000.0f;

    Serial.println("\n=== RÉSULTATS DE CALIBRATION ===");
    Serial.print("Tare : "); Serial.println(tare_value, 4);
    Serial.print("Avec 16kg : "); Serial.println(value_16kg, 4);
    Serial.print("Différence : "); Serial.println(difference, 4);
    Serial.print(">>> FACTEUR À NOTER : "); Serial.println(facteur, 6);
    Serial.println("==================================");

    Serial.println("\n[5/5] EFFACEMENT DE L'EEPROM");
    
    // Méthode 1 : Écriture explicite de 0.0 aux adresses des tares
    float zero = 0.0f;
    EEPROM.put(0, zero);  // Tare A
    EEPROM.put(sizeof(float), zero);  // Tare B
    EEPROM.commit();
    delay(100);  // Attendre que l'écriture soit terminée
    
    // Méthode 2 : Effacement complet (au cas où)
    for (int i = 0; i < 64; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    delay(100);  // Attendre que l'écriture soit terminée
    
    Serial.println("Effacement effectué + commit validé");

    Serial.println("\nVÉRIFICATION DE L'EFFACEMENT");
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
    Serial.println("CALIBRATION TERMINÉE");
    Serial.println("========================================\n");
    Serial.println("PROCÉDURE À SUIVRE :");
    Serial.println("1. Prenez le FACTEUR affiché ci-dessus");
    Serial.println("2. Ouvrez MasterAfficheOLED-...cpp");
    Serial.println("3. Trouvez calibrationFactorTable[32]");
    Serial.println("4. Remplacez la valeur du module par le facteur");
    Serial.println("5. Recompilez et téléversez le code");
    Serial.println("========================================\n");
}

void loop() {}