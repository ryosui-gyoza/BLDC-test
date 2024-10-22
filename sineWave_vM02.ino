// sine wave PWM, open loop control for Arduino Mega (ATMega2560)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

// Sine wave table
uint16_t sineTableH[360], sineTableL[360];
uint16_t pwmValueAH, pwmValueBH, pwmValueCH, pwmValueAL, pwmValueBL, pwmValueCL;

void setup() {
  for (int i = 0; i < 360; ++i) {
    if (483*sin(2*M_PI*i/360) < 16) {
      sineTableH[i] = 0;
      sineTableL[(i+180)%360] = 0;
    } else {
      sineTableH[i] = 90*sin(2*M_PI*i/360);
      sineTableL[(i+180)%360] = 90*sin(2*M_PI*i/360);
    }
    // if (483*sin(2*M_PI*(i+180)/360) > 483) {
    //   sineTableL[i] = 500;
    // } else {
    //  sineTableL[i] = 483*sin(2*M_PI*(i+180)/360);
    // }
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
  TCCR5A = (1 << COM5A1) | (1 << COM5B1) | (1 << COM5C1); // Phase Correct PWM mode, non-inverting
  TCCR5B = (1 << CS50) | (1 << WGM53);  // No prescaler, Phase Correct PWM, arbitrary resolution

  // Define resolution
  ICR4 = 0x01F3;
  ICR5 = 0x01F3;
}

void loop() {
  // Generate the sine wave modulated PWM signals
  for (int degree = 0; degree < 360; degree++) {
    pwmValueAH = sineTableH[degree];                     // Phase A sine value
    pwmValueBH = sineTableH[(degree + 120) % 360];        // Phase B sine value (120 degrees offset)
    pwmValueCH = sineTableH[(degree + 240) % 360];        // Phase C sine value (240 degrees offset)

    pwmValueAL = sineTableL[degree];                     // Phase A sine value
    pwmValueBL = sineTableL[(degree + 120) % 360];        // Phase B sine value (120 degrees offset)
    pwmValueCL = sineTableL[(degree + 240) % 360];        // Phase C sine value (240 degrees offset)

    // Apply to Timer 4 (pins 6, 7, 8)
    OCR4A = pwmValueAH;  // Output to phase A (OC4A, pin 6)
    OCR4B = pwmValueBH;  // Output to phase B (OC4B, pin 7)
    OCR4C = pwmValueCH;  // Output to phase C (OC4C, pin 8)

    // Apply to Timer 5 (pins 44, 45, 46)
    OCR5A = pwmValueAL;  // Complement phase A (OC5A, pin 44)
    OCR5B = pwmValueBL;  // Complement phase B (OC5B, pin 45)
    OCR5C = pwmValueCL;  // Complement phase C (OC5C, pin 46)
    
    // Add a delay for the frequency control
    delayMicroseconds(5);  // Adjust this delay for the desired frequency
  }
}
