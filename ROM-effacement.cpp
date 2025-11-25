/*
  Ce programme permet de lire les valeurs de tare (offset HX711) enregistrées en EEPROM,
  puis d'effacer complètement la mémoire EEPROM du CubeCell.

  - Au démarrage, il attend 10 secondes pour permettre à l'utilisateur d'ouvrir le terminal série
    et de visualiser les valeurs de tare actuellement stockées.
    cette attente permet de s'assurer que l'utilisateur a le temps 
    de se préparer à ouvrir le terminal avant que les données ne soient affichées.
    Sans ce délai, les données pourraient être manquées 
    car le programme continue immédiatement et l'ouverture du terminal entraîne un reset et la ROM est déjà effacée.
  - Il lit et affiche les valeurs float de tare pour Channel A (adresse 0) et Channel B (adresse 4).
  - Ensuite, il efface les 64 octets de l'EEPROM en les mettant à zéro et valide l'effacement avec EEPROM.commit().
  - Un message confirme que l'EEPROM a bien été effacée.

  Utiliser ce programme avant une nouvelle calibration ou pour réinitialiser la mémoire de tare.
*/
#include "Arduino.h"
#include <EEPROM.h>

void setup() {
    Serial.begin(9600);
    EEPROM.begin(64);

    Serial.println("Attente 10 secondes avant lecture et effacement...");
    delay(10000); // Attente de 10 secondes

    float tareA, tareB;
    EEPROM.get(0, tareA);
    EEPROM.get(sizeof(float), tareB);

    Serial.print("Tare A lue en EEPROM : "); Serial.println(tareA, 4);
    Serial.print("Tare B lue en EEPROM : "); Serial.println(tareB, 4);

    for (int i = 0; i < 64; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();

    Serial.println("EEPROM effacée.");
}

void loop() {}