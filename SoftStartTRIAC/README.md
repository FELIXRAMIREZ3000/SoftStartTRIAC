
# SoftStartTRIAC

Librería para Arduino que permite controlar el arranque progresivo de cargas inductivas (como bombas o motores) usando un TRIAC con optoacoplador (MOC3021), y conmutar automáticamente a un relé tras completar el proceso.

## Características

- Arranque por ángulo de fase con decremento ajustable.
- Protección por frecuencia (fuera de 115-125 Hz reinicia la rampa).
- Soporte para relé de bypass en estado estacionario.
- Control externo vía pin digital con `INPUT_PULLUP`.
- Compatible con Arduino Nano y otros AVR que usen Timer1.

## Ejemplo de uso

```cpp
#include <SoftStartTRIAC.h>

SoftStartTRIAC motor(2, 5, 4, 6);

void setup() {
  motor.begin();
  attachInterrupt(digitalPinToInterrupt(2), handleZeroCross, CHANGE);
}

void loop() {
  motor.update();
}

void handleZeroCross() {
  motor.handleZeroCross();
}
```

## Autor

Desarrollado por Felix Ramirez con ayuda de ChatGPT. Para uso en sistemas solares, acuapónicos o industriales.

---

