/*
Test de détection de résistance sur les GPIO digitaux et analogiques (utilisés en digital).
Mettre une résistance de 10kΩ entre VCC et le GPIO/ADC à tester.
Le programme affiche l'état de chaque broche toutes les secondes.
*/

#include <Arduino.h>

#define GPIO1 1
#define GPIO2 2
#define GPIO3 3
#define GPIO4 4
#define GPIO5 5
#define GPIO6 6
#define GPIO7 7
#define GPIO8 8
#define ADC1 9
#define ADC2 10
#define ADC3 11

void setup() {
  Serial.begin(9600);
  pinMode(GPIO1, INPUT_PULLUP);
  pinMode(GPIO2, INPUT_PULLUP);
  pinMode(GPIO3, INPUT_PULLUP);
  pinMode(GPIO4, INPUT_PULLUP);
  pinMode(GPIO5, INPUT_PULLUP);
  pinMode(GPIO6, INPUT_PULLUP);
  pinMode(GPIO7, INPUT_PULLUP);
  pinMode(GPIO8, INPUT_PULLUP);
  // ADC1, ADC2, ADC3 : à éviter pour digital
}

void loop() {
  Serial.print("GPIO1: "); Serial.print(digitalRead(GPIO1));
  Serial.print(" | GPIO2: "); Serial.print(digitalRead(GPIO2));
  Serial.print(" | GPIO3: "); Serial.print(digitalRead(GPIO3));
  Serial.print(" | GPIO4: "); Serial.print(digitalRead(GPIO4));
  Serial.print(" | GPIO5: "); Serial.print(digitalRead(GPIO5));
  Serial.print(" | GPIO6: "); Serial.print(digitalRead(GPIO6));
  Serial.print(" | GPIO7: "); Serial.print(digitalRead(GPIO7));
  Serial.print(" | GPIO8: "); Serial.println(digitalRead(GPIO8));
  delay(1000);
}