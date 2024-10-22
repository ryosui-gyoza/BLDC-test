// sine wave PWM, open loop control for Arduino Mega (ATMega2560)
// Hall sensor to get omega

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

// Sine wave table
float sineTable[360], rateTable[1024], duty;
uint16_t amp = 499, top, t1, t2, halfT0 = 1e4, pwmValueAH, pwmValueBH, pwmValueCH, pwmValueAL, pwmValueBL, pwmValueCL;
volatile uint16_t h1, h2;
uint8_t degree, skip, j;

void hallRISE() {
  h1 = micros();
}

void hallFALL() {
  h2 = micros();
}

void setup() {
  Serial.begin(9600);
  Serial.println("A");
  for (int i = 0; i < 360; ++i) {
    sineTable[i] = max(sin(2*M_PI*i/360), 0);
  }

  for (int i = 0; i < 1024; ++i) {
    rateTable[i] = 0.001 + pow((i/1030), 3);
  }

  // Set up output pins for Timer 4 (OC4A, OC4B, OC4C) and Timer 5 (OC5A, OC5B, OC5C)
  pinMode(6, OUTPUT);  // OC4A
  pinMode(7, OUTPUT);  // OC4B
  pinMode(8, OUTPUT);  // OC4C
  pinMode(44, OUTPUT); // OC5C
  pinMode(45, OUTPUT); // OC5B
  pinMode(46, OUTPUT); // OC5A

  pinMode(A0, INPUT); // ADC for PWM Freq command
  pinMode(A8, INPUT); // ADC for Power command

  pinMode(18, INPUT); // hall interruption

  /* set ADC prescaler */
    ADCSRA &= 0b11111000; // reset Prescaler Select Bits
    ADCSRA |= 0b00000011; // set prescaler to 8
  
  /* start Fast PWM */
    // Timer 4 Setup: Phase Correct PWM, arbitrary resolution
    TCCR4A = (1 << COM4A1) | (1 << COM4B1) | (1 << COM4C1); // Phase Correct PWM mode, non-inverting
    TCCR4B = (1 << CS40) | (1 << WGM43);  // No prescaler, Phase Correct PWM, arbitrary resolution

    // Timer 5 Setup: Phase Correct PWM, arbitrary resolution
    TCCR5A = (1 << COM5A1) | (1 << COM5B1) | (1 << COM5C1); // Phase Correct PWM mode, non-inverting
    TCCR5B = (1 << CS50) | (1 << WGM53);  // No prescaler, Phase Correct PWM, arbitrary resolution

    // Default resolution
    ICR4 = 0x01F3;
    ICR5 = 0x01F3;

  /* activate Interruption */
    attachInterrupt(18, hallRISE, RISING);
    attachInterrupt(18, hallFALL, FALLING);
    sei();

  /* forced commutation */
  Serial.println("B");
  for (int j = 0; j < 3600; j++) {
    degree = j%360;
    pwmValueAH = amp * sineTable[degree];                     // Phase A sine value
    if (pwmValueAH < 8) {pwmValueAH = 0;}
    pwmValueBH = amp * sineTable[(degree + 120) % 360];        // Phase B sine value (120 degrees offset)
    if (pwmValueBH < 8) {pwmValueBH = 0;}
    pwmValueCH = amp * sineTable[(degree + 240) % 360];        // Phase C sine value (240 degrees offset)
    if (pwmValueCH < 8) {pwmValueCH = 0;}

    pwmValueAL = amp * sineTable[(degree + 180) % 360];        // Phase A sine value
    if (pwmValueAL < 8) {pwmValueAL = 0;}
    pwmValueBL = amp * sineTable[(degree + 300) % 360];        // Phase B sine value (120 degrees offset)
    if (pwmValueBL < 8) {pwmValueBL = 0;}
    pwmValueCL = amp * sineTable[(degree + 60) % 360];        // Phase C sine value (240 degrees offset)
    if (pwmValueCL < 8) {pwmValueCL = 0;}

    // Apply to Timer 4 (pins 6, 7, 8)
    OCR4A = pwmValueAH;  // Output to phase A (OC4A, pin 6)
    OCR4B = pwmValueBH;  // Output to phase B (OC4B, pin 7)
    OCR4C = pwmValueCH;  // Output to phase C (OC4C, pin 8)

    // Apply to Timer 5 (pins 44, 45, 46)
    OCR5A = pwmValueAL;  // Complement phase A (OC5A, pin 44)
    OCR5B = pwmValueBL;  // Complement phase B (OC5B, pin 45)
    OCR5C = pwmValueCL;  // Complement phase C (OC5C, pin 46)

    delayMicroseconds(50);
  }
  Serial.println("C");
  t1 = micros();
}

void loop() {
  // Apply to Timer 4 (pins 6, 7, 8)
  OCR4A = pwmValueAH;  // Output to phase A (OC4A, pin 6)
  OCR4B = pwmValueBH;  // Output to phase B (OC4B, pin 7)
  OCR4C = pwmValueCH;  // Output to phase C (OC4C, pin 8)

  // Apply to Timer 5 (pins 44, 45, 46)
  OCR5A = pwmValueAL;  // Complement phase A (OC5A, pin 44)
  OCR5B = pwmValueBL;  // Complement phase B (OC5B, pin 45)
  OCR5C = pwmValueCL;  // Complement phase C (OC5C, pin 46)

  /* calculate next pwm width */
  pwmValueAH = amp * sineTable[degree];                     // Phase A sine value
  if (pwmValueAH < 8) {pwmValueAH = 0;}
  pwmValueBH = amp * sineTable[(degree + 120) % 360];        // Phase B sine value (120 degrees offset)
  if (pwmValueBH < 8) {pwmValueBH = 0;}
  pwmValueCH = amp * sineTable[(degree + 240) % 360];        // Phase C sine value (240 degrees offset)
  if (pwmValueCH < 8) {pwmValueCH = 0;}

  pwmValueAL = amp * sineTable[(degree + 180) % 360];        // Phase A sine value
  if (pwmValueAL < 8) {pwmValueAL = 0;}
  pwmValueBL = amp * sineTable[(degree + 300) % 360];        // Phase B sine value (120 degrees offset)
  if (pwmValueBL < 8) {pwmValueBL = 0;}
  pwmValueCL = amp * sineTable[(degree + 60) % 360];        // Phase C sine value (240 degrees offset)
  if (pwmValueCL < 8) {pwmValueCL = 0;}
  t2 = micros();

  /* calculate duration and next electric degree */
  uint16_t halfT = abs(h1 - h2);
  if (halfT >= 2*halfT0) {
    halfT = halfT0;
  } else {
    halfT0 = halfT;
  }

  /* read and apply commands */
  if (t2 - t1 > halfT) {
    PORTB |= 0b10000000; // indicate lack of time for calculation
    amp--;
  } else if (t2 - t1 + 100 > halfT) {
    cli();
    /* Freq command ADC */
      ADMUX = 0b00000001;           // select pin A0
      ADCSRA |= (1 << ADSC);        // set Start Conversion to High
        /* wait for ADC */
        PORTB &= 0b01111111; // imdicate enough time for calculation
        uint8_t dL, dH;
      while (ADCSRA & (1 << ADSC)); // wait for measurement
      dL = ADCL;                    // get lower 8 digits
      dH = ADCH;	                  // get upper 8 digits
      uint16_t freq_comm = (dH << 8 | dL);  // assign analog voltage
    /* Duty command ADC */
      ADMUX = 0b10000000;           // select pin A8
      ADCSRA |= (1 << ADSC);        // set Start Conversion to High
        /* wait for ADC */
        top = 4*freq_comm + 399;
        ICR4 = top;
        ICR5 = top;
      while (ADCSRA & (1 << ADSC)); // wait for measurement
      dL = ADCL;                    // get lower 8 digits
      dH = ADCH;	                  // get upper 8 digits
      uint16_t duty_comm = (dH << 8 | dL);  // assign analog voltage
    amp = min(rateTable[duty_comm]*top, top - 8);
    Serial.print(top);
    Serial.print("\t");
    Serial.print(duty);
    Serial.print("\t");
    Serial.print(halfT);
    Serial.print("\n");
    
    t2 = micros();
    sei();
  }

  t1 = t2;
  skip = 180*(t2 - t1)/halfT;
  degree = (degree + skip)%360;
}