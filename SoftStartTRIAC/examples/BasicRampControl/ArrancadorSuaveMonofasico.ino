// === Arduino Nano - Arranque progresivo con TRIAC + control por MOC3021 + relé de bypass + blindaje ===
// Este código controla el arranque de una bomba de agua de 350 W conectada a un TRIAC.
// Tras completar una rampa de encendido por ángulo de fase usando un MOC3021, conmuta a un relé.
// Se blindó contra errores causados por solapamiento entre el tiempo de retardo y la duración del pulso.

const int ZeroPin = 2;           // Pin de entrada para cruce por cero (señal del optoacoplador)
const int mocPin = 5;            // Salida digital hacia MOC3021 (activación del TRIAC)
const int controlPin = 4;        // Pin de control externo con pull-up. LOW = apagado forzado
const int ledPin = 13;           // LED del Nano: parpadea lento/rápido según fase
const int relePin = 6;           // Salida digital para activar el relé o contactor

// === Parámetros y estado ===
volatile bool estadoPinAnterior = LOW;
volatile unsigned long tiempoUltimoFlanco = 0;
volatile unsigned long periodoMedido = 0;

const unsigned long pulsoFijo = 300;       // Duración del pulso de disparo del TRIAC en µs
// 🛠️ Elegido por ser suficientemente largo para garantizar disparo post-cero,
// incluso con tensión baja o interferencias, y suficientemente corto para no pisar el siguiente semiciclo.

unsigned long retardoSalida = 7950;        // Retardo inicial (arranque suave) 7850
const unsigned long retardoMinimo = 1000;   // Retardo final. ⚠️ Debe ser > pulsoFijo por seguridad 1050
const unsigned long pasoDecremento = 5;    // Decremento por ciclo de cruce
const int ciclosSuperpuestos = 5;         // Ciclos extra en que sigue activo el TRIAC tras activar el relé
int contadorCiclosRestantes = 0;           // Cuenta regresiva de los ciclos superpuestos

bool rampaCompletada = false;
bool salidaFijaActiva = false;
bool estadoAnteriorControl = HIGH;

unsigned long ultimoParpadeo = 0;
bool estadoLed = LOW;

// Límites aceptables de frecuencia (115–125 Hz)
const unsigned long periodoMaximo = 8696;  // Límite inferior de frecuencia
const unsigned long periodoMinimo = 8000;  // Límite superior de frecuencia

void setup() {
  pinMode(ZeroPin, INPUT);
  pinMode(mocPin, OUTPUT);
  pinMode(controlPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(relePin, OUTPUT);

  digitalWrite(mocPin, HIGH);     // MOC3021 inactivo (alto)
  digitalWrite(ledPin, LOW);      // LED apagado
  digitalWrite(relePin, LOW);     // Relé apagado

  attachInterrupt(digitalPinToInterrupt(ZeroPin), flancoCambio, CHANGE);
  setupTimer1();
}

void loop() {
  bool estadoActualControl = digitalRead(controlPin);

  if (estadoActualControl == LOW) {
    // Apagado forzado → reset de todo
    rampaCompletada = false;
    salidaFijaActiva = false;
    retardoSalida = 7850;
    contadorCiclosRestantes = 0;
    digitalWrite(mocPin, HIGH);
    digitalWrite(ledPin, LOW);
    digitalWrite(relePin, LOW);
    return;
  }

  if (estadoAnteriorControl == LOW && estadoActualControl == HIGH) {
    // Reinicio del proceso (flanco de subida)
    rampaCompletada = false;
    salidaFijaActiva = false;
    retardoSalida = 7850;
    contadorCiclosRestantes = 0;
    digitalWrite(relePin, LOW);
  }

  estadoAnteriorControl = estadoActualControl;

  // LED parpadea lento o rápido según fase
  unsigned long intervalo = rampaCompletada ? 500 : 100;
  if (millis() - ultimoParpadeo >= intervalo) {
    ultimoParpadeo = millis();
    estadoLed = !estadoLed;
    digitalWrite(ledPin, estadoLed);
  }
}

void flancoCambio() {
  int estadoActual = digitalRead(ZeroPin);

  if (estadoActual == HIGH && estadoPinAnterior == LOW) {
    // Flanco de subida del cruce por cero
    unsigned long ahora = micros();
    periodoMedido = ahora - tiempoUltimoFlanco;
    tiempoUltimoFlanco = ahora;

    // Verificación de frecuencia anómala
    if (periodoMedido < periodoMinimo || periodoMedido > periodoMaximo) {
      rampaCompletada = false;
      salidaFijaActiva = false;
      retardoSalida = 7850;
      contadorCiclosRestantes = 0;
      digitalWrite(relePin, LOW);
      estadoPinAnterior = estadoActual;
      return;
    }

    if (digitalRead(controlPin) == HIGH) {
      if (!rampaCompletada) {
        // En rampa: programar disparo
        unsigned int cuentasRetardo = retardoSalida * 2;
        cli(); TCNT1 = 0; OCR1A = cuentasRetardo;
        TIFR1 |= (1 << OCF1A); TIMSK1 |= (1 << OCIE1A); sei();

        if (retardoSalida > retardoMinimo + pasoDecremento) {
          retardoSalida -= pasoDecremento;
        } else {
          // Finaliza rampa
          retardoSalida = retardoMinimo;
          rampaCompletada = true;
          salidaFijaActiva = true;
          contadorCiclosRestantes = ciclosSuperpuestos;
          digitalWrite(relePin, HIGH);  // Activa el relé
        }
      }
      else if (salidaFijaActiva && contadorCiclosRestantes > 0) {
        // Ciclos extra de disparo para cubrir latencia del relé
        contadorCiclosRestantes--;
        unsigned int cuentasRetardo = retardoMinimo * 2;
        cli(); TCNT1 = 0; OCR1A = cuentasRetardo;
        TIFR1 |= (1 << OCF1A); TIMSK1 |= (1 << OCIE1A); sei();
      }
    }
  }

  estadoPinAnterior = estadoActual;
}

void setupTimer1() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B |= (1 << WGM12);   // Modo CTC
  TCCR1B |= (1 << CS11);    // Prescaler 8 → 0.5us por cuenta
  sei();
}

ISR(TIMER1_COMPA_vect) {
  static bool estadoSalida = false;

  if (!estadoSalida) {
    digitalWrite(mocPin, LOW);  // Activa el MOC3021
    unsigned int finPulso = TCNT1 + pulsoFijo * 2;

    // Protección contra desbordamiento (blindaje contra errores)
    if (finPulso <= TCNT1) {
      finPulso = pulsoFijo * 2;
    }

    OCR1A = finPulso;
    estadoSalida = true;
  } else {
    digitalWrite(mocPin, HIGH);  // Apaga MOC3021
    TIMSK1 &= ~(1 << OCIE1A);    // Desactiva interrupción hasta siguiente flanco
    estadoSalida = false;
  }
}
