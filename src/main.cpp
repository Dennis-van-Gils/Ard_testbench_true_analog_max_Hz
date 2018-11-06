/*------------------------------------------------------------------------------
Dennis van Gils
06-11-2018
------------------------------------------------------------------------------*/

#include <Arduino.h>
#include "DvG_SerialCommand.h"
#include "ZeroTimer.h"

// Wait for synchronization of registers between the clock domains
static __inline__ void syncDAC() __attribute__((always_inline, unused));
static void syncDAC() {while (DAC->STATUS.bit.SYNCBUSY == 1);}

#define Ser Serial
DvG_SerialCommand sc(Ser);  // Instantiate serial command listener

#define ANALOG_WRITE_RESOLUTION 10    // [bits] Fixed to 10 on M0 Pro
#define INTERRUPT_CLOCK_PERIOD  50    // [usec]

double sine_freq = 200.0;   // [Hz]
volatile uint16_t sine_out;

/*------------------------------------------------------------------------------
    Sine wave look-up table (LUT)
------------------------------------------------------------------------------*/

#define N_LUT 2048    // Number of elements for one full period
#define A_REF 3.300   // [V] Analog voltage reference

// Tip: limiting the output voltage range to slighty above 0.0 V will improve
// the shape of the sine wave at it's minimum. Apparently, the analog out port
// has difficulty in cleanly dropping the output voltage completely to 0.0 V.
#define V_out_p2p    2.0      // [V] peak to peak
#define V_out_center 2.0      // [V]

uint16_t LUT_sine[N_LUT] = {0};
volatile double LUT_micros_to_index_factor = 1e-6 * sine_freq * N_LUT;

void create_LUT_sine() {
  const double offset = V_out_center / A_REF;
  const double amplitude = 0.5 / A_REF * V_out_p2p;
  for (int i = 0; i < N_LUT; i++) {
    LUT_sine[i] = (uint16_t) round((pow(2, ANALOG_WRITE_RESOLUTION) - 1) * \
                  (offset + amplitude * sin(2*PI*i/N_LUT)));
  }
}

/*------------------------------------------------------------------------------
    Interrupt service routine
------------------------------------------------------------------------------*/

void my_ISR() {
  uint16_t LUT_index;
  LUT_index = ((uint16_t) round(micros() * LUT_micros_to_index_factor)) % N_LUT;
  sine_out = LUT_sine[LUT_index];

  syncDAC();
  DAC->DATA.reg = sine_out & 0x3FF;
  return;
}

/*------------------------------------------------------------------------------
    setup
------------------------------------------------------------------------------*/

void setup() {
  Ser.begin(9600);

  create_LUT_sine();
  DAC->CTRLA.bit.ENABLE = 0x01;
  TC.startTimer(INTERRUPT_CLOCK_PERIOD, my_ISR);
}

/*------------------------------------------------------------------------------
    loop
------------------------------------------------------------------------------*/

void loop() {
  char* strCmd; // Incoming serial command string

  if (sc.available()) {
    strCmd = sc.getCmd();

    if (strcmpi(strCmd, "sf?") == 0) {
      // Get frequency of output sine wave [Hz]
      Ser.print("sf = "); Ser.print(sine_freq); Ser.println(" Hz");
    } else if (strncmpi(strCmd, "sf", 2) == 0) {
      // Set frequency of output sine wave [Hz]
      sine_freq = parseFloatInString(strCmd, 2);
      LUT_micros_to_index_factor = 1e-6 * sine_freq * N_LUT;
      Ser.print("sf = "); Ser.print(sine_freq); Ser.println(" Hz");
    }
  }
}
