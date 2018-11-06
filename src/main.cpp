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

#define ANALOG_READ_RESOLUTION  12    // [bits] 10 or 12 for M0 Pro
#define ANALOG_WRITE_RESOLUTION 10    // [bits] Fixed to 10 on M0 Pro
#define INTERRUPT_CLOCK_PERIOD  100   // [usec]

double sine_freq = 133.33;   // [Hz]
volatile uint32_t sine_out;

/*------------------------------------------------------------------------------
    Sine wave look-up table (LUT)
------------------------------------------------------------------------------*/

#define A_REF 3.300   // [V] Analog voltage reference
#define N_LUT 2048    // Number of elements for one full period

// Tip: limiting the output voltage range to slighty above 0.0 V will improve
// the shape of the sine wave at it's minimum. Apparently, the analog out port
// has difficulty in cleanly dropping the output voltage completely to 0.0 V.
const double V_p2p = 2.0;     // [V] peak to peak
const double V_center = 2.0;  // [V]
uint32_t LUT_sine[N_LUT] = {0};

void create_LUT_sine() {
  const double offset = V_center / A_REF;
  const double amplitude = 0.5 / A_REF * V_p2p;
  for (int i = 0; i < N_LUT; i++) {
    LUT_sine[i] = (uint32_t) round((pow(2, ANALOG_WRITE_RESOLUTION) - 1) * \
                  (offset + amplitude * sin(2*PI*i/N_LUT)));
  }
}

/*------------------------------------------------------------------------------
    Interrupt service routine
------------------------------------------------------------------------------*/

volatile uint32_t LUT_index;
double LUT_micros_to_index_factor = 1e-6 * sine_freq * N_LUT;

void myISR() {
  LUT_index = ((uint32_t) round(micros() * LUT_micros_to_index_factor)) % N_LUT;
  sine_out = LUT_sine[LUT_index];

  syncDAC();
  DAC->DATA.reg = sine_out & 0x3FF;
  syncDAC();
  DAC->CTRLA.bit.ENABLE = 0x01;
  syncDAC();
  return;
}

/*------------------------------------------------------------------------------
    setup
------------------------------------------------------------------------------*/

void setup() {
  Ser.begin(9600);
  analogReadResolution(ANALOG_READ_RESOLUTION);

  create_LUT_sine();
  TC.startTimer(INTERRUPT_CLOCK_PERIOD, myISR);
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
