/*******************************************************************************
  Dennis van Gils
  29-07-2018
 ******************************************************************************/

#include <Arduino.h>
#include "DvG_ZeroTimer.h"

//#define TOGGLE_PIN 13
#ifdef ARDUINO_ARCH_AVR
  #define REGTYPE uint8_t   // AVR uses 8-bit registers
#else
  #define REGTYPE uint32_t
#endif

REGTYPE pin13;
volatile REGTYPE *mode13;
volatile REGTYPE *out13;

#define Ser Serial
bool fToggle = true;

void myISR() {
  //digitalWrite(TOGGLE_PIN, fToggle);
  if (fToggle) {
    *out13 |= pin13;
  } else {
    *out13 &= ~pin13;
  }
  fToggle = not(fToggle);
}

/*******************************************************************************
  setup
*******************************************************************************/

void setup() {
  Ser.begin(9600);

  //pinMode(TOGGLE_PIN, OUTPUT);
  pin13 = digitalPinToBitMask(13);
  mode13 = portModeRegister(digitalPinToPort(13));
  out13 = portOutputRegister(digitalPinToPort(13));

  // set pin 13 port as ouput
  *mode13 |= pin13;

  TC.startTimer(10e3, myISR);
}

/*******************************************************************************
  loop
*******************************************************************************/

uint32_t tock = micros();
uint32_t now = micros();
const uint32_t PERIOD = 3e6;
bool fToggle2 = false;

void loop() {
  //*
  now = micros();
  if (now - tock > PERIOD) {
    Ser.println(now - tock);
    tock = now;

    if (fToggle2) {
      TC.setPeriod(10e3);
    } else {
      TC.setPeriod(30e3);
    }
    fToggle2 = not(fToggle2);
  }
  //*/
}
