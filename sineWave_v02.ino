// sine wave control for Arduino Mega (ATMega2560)
// hall sensor feedback, no sensorless control
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

// Define output pins for high and low sides of each phase
const int pwmPinUHigh = 2;  // PWM output pin for phase U high side
const int pwmPinULow = 3;   // PWM output pin for phase U low side
const int pwmPinVHigh = 4; // PWM output pin for phase V high side
const int pwmPinVLow = 5;   // PWM output pin for phase V low side
const int pwmPinWHigh = 6; // PWM output pin for phase W high side
const int pwmPinWLow = 7;   // PWM output pin for phase W low side

const int tableSize = 72;  // Size of sine lookup table
const float sineSize = 36;    // Size of sine wave
int sineTable[tableSize];

volatile int i = 0;         // index for repetition
int sector = 0;    // index to identify sector
int secdiv = tableSize / 6;   //  interval of sector shift

volatile int indexU = 0;                    // Index for U phase
volatile int indexV = tableSize / 3;        // Index for V phase
volatile int indexW = 2 * tableSize / 3;    // Index for W phase

volatile int pwmValueU;     // output strength of U phase
volatile int pwmValueV;     // output strength of V phase
volatile int pwmValueW;     // output strength of W phase

volatile int skip = 1;

/* define Interrupt function (Hall sensor) */
void HUH () {
  ut1 = micros();
  thu = 0;
  th = 0;
  uHalfT = ut1 - ut2;
}
void HUL () {
  ut2 = micros();
  thu = 1800;
  th = 1800;
  uHalfT = ut2 - ut1;
}
void HVH () {
  vt1 = micros();
  thv = 0;
  vHalfT = vt1 - vt2;
}
void HVL () {
  vt2 = micros();
  thv = 180;
  vHalfT = vt2 - vt1;
}
void HWH () {
  wt1 = micros();
  thw = 0;
  wHalfT = wt1 - wt2;
}
void HWL () {
  wt2 = micros();
  thw = 180;
  wHalfT = wt2 - wt1;
}
/* define Interrupt function (Hall sensor) */

void setup() {
  /* set pinMode */
  pinMode(5, OUTPUT); // U High
  pinMode(6, OUTPUT); // U Low
  pinMode(2, OUTPUT); // V High
  pinMode(7, OUTPUT); // V Low
  pinMode(3, OUTPUT); // W High
  pinMode(8, OUTPUT); // W Low

  pinMode(18, INPUT); // Hall U
  pinMode(19, INPUT); // Hall V
  pinMode(20, INPUT); // Hall W

  pinMode(A0, INPUT); // Shunt U
  pinMode(A1, INPUT); // Shunt V
  pinMode(A2, INPUT); // Shunt W

  pinMode(A3, INPUT); // Speed command

  /* set AD prescaler */
  ADCSRA &= 0b11111000; // initialize Prescaler Select Bits
  ADCSRA |= 0b00000011; // set prescaler to 8

  /* generate sin table */
  for (i = 0; i <= 900; i++) {
    sint[i] = sin(PI*i/1800);
  }

  /* generate arcsin table */
  for (i = 0; i <= 1000; i++) {
    asint[i] = asin(i/1000) * 180 / PI ;
  }

  /* activate Interruption */
    attachInterrupt(18, HUH, RISING);
    attachInterrupt(19, HVH, RISING);
    attachInterrupt(20, HWH, RISING);

    attachInterrupt(18, HUL, FALLING);
    attachInterrupt(19, HVL, FALLING);
    attachInterrupt(20, HWL, FALLING);
  /* activate Interruption */

  sei();
}

void loop() {
  if (ut1 < ut2) {
    thu = (1800 / uHalfT * (micros() - ut2) + 1800); // calculate from half of period
  } else {
    thu = 1800 / uHalfT * (micros() - ut1);
  }
  if (vt1 < vt2) {
    thv = 1800 / vHalfT * (micros() - vt2) + 1800;
  } else {
    thv = 1800 / vHalfT * (micros() - vt1);
  }
  if (wt1 < wt2) {
    thw = 1800 / wHalfT * (micros() - wt2) + 1800;
  } else {
    thw = 1800 / wHalfT * (micros() - wt1);
  } 

  if (0 <= thu) {
    OCR4A = 0;
    if (thu <= 900) {
      OCR3A = round(sint[thu] * duty * 1023);
    } else {
      OCR3A = round(sint[180 - thu] * duty * 1023);
    }
  } else {
    OCR3A = 0;
    if (thu >= -900) {
      OCR4A = round(-sint[-thu] * duty * 1023);
    } else {
      OCR4A = round(-sint[180 + thu] * duty * 1023);
    }
  }
  if (0 <= thv) {
    OCR4B = 0;
    if (thv <= 900) {
      OCR3B = round(sint[thv] * duty * 1023);
    } else {
      OCR3B = round(sint[180 - thv] * duty * 1023);
    }
  } else {
    OCR3B = 0;
    if (thv >= -900) {
      OCR4B = round(-sint[-thv] * duty * 1023);
    } else {
      OCR4B = round(-sint[180 + thv] * duty * 1023);
    }
  }
  if (0 <= thw) {
    OCR4C = 0;
    if (thw <= 900) {
      OCR3C = round(sint[thw] * duty * 1023);
    } else {
      OCR3C = round(sint[180 - thw] * duty * 1023);
    }
  } else {
    OCR3C = 0;
    if (thw >= -900) {
      OCR4C = round(-sint[-thw] * duty * 1023);
    } else {
      OCR4C = round(-sint[180 + thw] * duty * 1023);
    }
  }
}

// Timer1 Compare Match A interrupt service routine (ISR)
ISR(TIMER1_COMPA_vect) {

  }
