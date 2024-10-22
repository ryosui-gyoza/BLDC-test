// sine wave PWM, open loop control for Arduino Mega (ATMega2560)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

// Sine wave table
uint16_t sineTable[360];
const uint8_t ofs = 24;
uint16_t pwmValueA, pwmValueB, pwmValueC;

void setup() {
  for (int i = 0; i < 360; ++i) {
    sineTable[i] = 450*sin(2*M_PI*i/360) + 25;
  }
  // Set up output pins for Timer 4 (OC4A, OC4B, OC4C) and Timer 5 (OC5A, OC5B, OC5C)
  pinMode(6, OUTPUT);  // OC4A
  pinMode(7, OUTPUT);  // OC4B
  pinMode(8, OUTPUT);  // OC4C
  pinMode(44, OUTPUT); // OC5A
  pinMode(45, OUTPUT); // OC5B
  pinMode(46, OUTPUT); // OC5C
  
  // Timer 4 Setup: Phase Correct PWM, arbitrary resolution
  TCCR4A = (1 << COM4A1) | (1 << COM4B1) | (1 << COM4C1); // Phase Correct PWM mode, non-inverting
  TCCR4B = (1 << CS40) | (1 << WGM43);  // No prescaler, Phase Correct PWM, arbitrary resolution

  // Timer 5 Setup: Phase Correct PWM, arbitrary resolution
  TCCR5A = (1 << COM5A1) | (1 << COM5B1) | (1 << COM5C1) | (1 << COM5A0) | (1 << COM5B0) | (1 << COM5C0); // Phase Correct PWM mode, non-inverting
  TCCR5B = (1 << CS50) | (1 << WGM53);  // No prescaler, Phase Correct PWM, arbitrary resolution

  // Define resolution
  ICR4 = 0x01F4;
  ICR5 = 0x01F4;
}

void loop() {
  // Generate the sine wave modulated PWM signals
  for (int degree = 0; degree < 360; degree++) {
    pwmValueA = sineTable[degree];                     // Phase A sine value
    pwmValueB = sineTable[(degree + 120) % 360];        // Phase B sine value (120 degrees offset)
    pwmValueC = sineTable[(degree + 240) % 360];        // Phase C sine value (240 degrees offset)

    // Apply to Timer 4 (pins 6, 7, 8)
    OCR4A = pwmValueA - ofs;  // Output to phase A (OC4A, pin 6)
    OCR4B = pwmValueB - ofs;  // Output to phase B (OC4B, pin 7)
    OCR4C = pwmValueC - ofs;  // Output to phase C (OC4C, pin 8)

    // Apply to Timer 5 (pins 44, 45, 46)
    OCR5A = pwmValueA + ofs;  // Complement phase A (OC5A, pin 44)
    OCR5B = pwmValueB + ofs;  // Complement phase B (OC5B, pin 45)
    OCR5C = pwmValueC + ofs;  // Complement phase C (OC5C, pin 46)
    
    // Add a delay for the frequency control
    delayMicroseconds(50);  // Adjust this delay for the desired frequency
  }
}
