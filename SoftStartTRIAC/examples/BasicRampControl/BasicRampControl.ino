
/*
  BasicRampControl.ino - Ejemplo de uso básico de la librería SoftStartTRIAC
  Este ejemplo muestra cómo iniciar la rampa de encendido usando un optoacoplador
  y conmutar a un relé físico tras completar la rampa.
*/

#include <SoftStartTRIAC.h>

// Definición de pines para este ejemplo
const int PIN_ZERO = 2;
const int PIN_TRIAC = 5;
const int PIN_CONTROL = 4;
const int PIN_RELAY = 6;
const int PIN_LED = 13;

SoftStartTRIAC arranque(PIN_ZERO, PIN_TRIAC, PIN_CONTROL, PIN_RELAY, PIN_LED);

void setup() {
  arranque.begin();
  attachInterrupt(digitalPinToInterrupt(PIN_ZERO), handleZeroCross, CHANGE);
}

void loop() {
  arranque.update();
}

void handleZeroCross() {
  arranque.handleZeroCross();
}
