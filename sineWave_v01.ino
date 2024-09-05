#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

// Define output pins for high and low sides of each phase
const int pwmPinUHigh = 9;  // PWM output pin for phase U high side
const int pwmPinULow = 6;   // PWM output pin for phase U low side
const int pwmPinVHigh = 10; // PWM output pin for phase V high side
const int pwmPinVLow = 5;   // PWM output pin for phase V low side
const int pwmPinWHigh = 11; // PWM output pin for phase W high side
const int pwmPinWLow = 3;   // PWM output pin for phase W low side

const int tableSize = 48;  // Size of sine lookup table
const int sineSize = 20;    // Size of sine wave
int sineTable[tableSize];   // Sine wave lookup table

volatile int i = 0;         // index for repetition
volatile int sector = 0;    // index to identify sector
const int secdiv = tableSize / 6;   //  interval of sector shift

volatile int indexU = 0;                    // Index for U phase
volatile int indexV = tableSize / 3;        // Index for V phase
volatile int indexW = 2 * tableSize / 3;    // Index for W phase

volatile int pwmValueU;     // output strength of U phase
volatile int pwmValueV;     // output strength of V phase
volatile int pwmValueW;     // output strength of W phase

void setup() {
  // Generate sine wave lookup table
  for (int i = 0; i < tableSize; i++) {
    sineTable[i] = (sin(2 * PI * i / tableSize) * sineSize);
  }

  // Set pins as output
  pinMode(pwmPinUHigh, OUTPUT);
  pinMode(pwmPinULow, OUTPUT);
  pinMode(pwmPinVHigh, OUTPUT);
  pinMode(pwmPinVLow, OUTPUT);
  pinMode(pwmPinWHigh, OUTPUT);
  pinMode(pwmPinWLow, OUTPUT);

  // Set CTC mode
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode with no prescaler

  // Set OCR1A to define TOP value for the timer
  OCR1A = 16; // Adjust as needed for your timing

  // Enable Timer1 Compare Match A interrupt
  TIMSK1 = (1 << OCIE1A);

  // Enable global interrupts
  sei();
}

void loop() {
  // The main loop is empty, as the PWM control is handled by the ISR
}

// Timer1 Compare Match A interrupt service routine (ISR)
ISR(TIMER1_COMPA_vect) {
  // Set periodic index
  if (i == sineSize + 5) {
    indexU++;
    indexV++;
    indexW++;
    i = 0;
    if (indexU == tableSize) {
      indexU = 0;
    }
    if (indexV == tableSize) {
      indexV = 0;
    }
    if (indexW == tableSize) {
      indexW = 0;
    }
  }
  // Beginning of PWM period
  if (i == 0) {
    // Set reference of PWM value for each phase
    pwmValueU = sineTable[indexU];
    pwmValueV = sineTable[indexV];
    pwmValueW = sineTable[indexW];
  }

  // Sector specifier
  switch (indexU) {
    case 0:
      sector = 0;
      break;
    case secdiv:
      sector = 1;
      break;
    case 2*secdiv:
      sector = 2;
      break;
    case 3*secdiv:
      sector = 3;
      break;
    case 4*secdiv:
      sector = 4;
      break;
    case 5*secdiv:
      sector = 5;
      break;
  }

  //generate PWM
  switch (sector) {
    case 0:
      if (i == 0) {
        digitalWrite(pwmPinUHigh, HIGH);
        digitalWrite(pwmPinVHigh, HIGH);
        digitalWrite(pwmPinWLow, HIGH);
      }
      if (i == pwmValueU) {
        digitalWrite(pwmPinUHigh, LOW);
      }
      if (i == pwmValueV) {
        digitalWrite(pwmPinVHigh, LOW);
      }
      if (i == -pwmValueW) {
        digitalWrite(pwmPinWLow, LOW);
      }
      break;
    case 1:
      if (i == 0) {
        digitalWrite(pwmPinUHigh, HIGH);
        digitalWrite(pwmPinVLow, HIGH);
        digitalWrite(pwmPinWLow, HIGH);
      }
      if (i == -pwmValueV) {
        digitalWrite(pwmPinVLow, LOW);
      }
      if (i == pwmValueU) {
        digitalWrite(pwmPinUHigh, LOW);
      }
      if (i == -pwmValueW) {
        digitalWrite(pwmPinWLow, LOW);
      }
      break;
    case 2:
      if (i == 0) {
        digitalWrite(pwmPinUHigh, HIGH);
        digitalWrite(pwmPinVLow, HIGH);
        digitalWrite(pwmPinWHigh, HIGH);
      }
      if (i == pwmValueW) {
        digitalWrite(pwmPinWHigh, LOW);
      }
      if (i == pwmValueU) {
        digitalWrite(pwmPinUHigh, LOW);
      }
      if (i == -pwmValueV) {
        digitalWrite(pwmPinVLow, LOW);
      }
      break;
    case 3:
      if (i == 0) {
        digitalWrite(pwmPinULow, HIGH);
        digitalWrite(pwmPinVLow, HIGH);
        digitalWrite(pwmPinWHigh, HIGH);
      }
      if (i == pwmValueW) {
        digitalWrite(pwmPinWHigh, LOW);
      }
      if (i == -pwmValueV) {
        digitalWrite(pwmPinVLow, LOW);
      }
      if (i == -pwmValueU) {
        digitalWrite(pwmPinULow, LOW);
      }
      break;
    case 4:
      if (i == 0) {
        digitalWrite(pwmPinULow, HIGH);
        digitalWrite(pwmPinVHigh, HIGH);
        digitalWrite(pwmPinWHigh, HIGH);
      }
      if (i == pwmValueV) {
        digitalWrite(pwmPinVHigh, LOW);
      }
      if (i == -pwmValueU) {
        digitalWrite(pwmPinULow, LOW);
      }
      if (i == pwmValueW) {
        digitalWrite(pwmPinWHigh, LOW);
      }
      break;
    case 5:
      if (i == 0) {
        digitalWrite(pwmPinULow, HIGH);
        digitalWrite(pwmPinVHigh, HIGH);
        digitalWrite(pwmPinWLow, HIGH);
      }
      if (i == -pwmValueW) {
        digitalWrite(pwmPinWLow, LOW);
      }
      if (i == -pwmValueU) {
        digitalWrite(pwmPinULow, LOW);
      }
      if (i == pwmValueV) {
        digitalWrite(pwmPinVHigh, LOW);
      }
      break;
  }
  if (i == sineSize) {
    digitalWrite(pwmPinUHigh, LOW);
    digitalWrite(pwmPinULow, LOW);
    digitalWrite(pwmPinVHigh, LOW);
    digitalWrite(pwmPinVLow, LOW);
    digitalWrite(pwmPinWHigh, LOW);
    digitalWrite(pwmPinWLow, LOW);
  }
  i++;
}
