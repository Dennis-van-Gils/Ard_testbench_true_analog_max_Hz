# Arduino (Atmel SAMD) testbench for true analog-out signal generation and analog-in data acquisition.
Generates a sine wave on the analog out port (A0) of an Arduino M0 Pro using an interrupt timer and direct port access. Sine waves with frequencies of up to ~500 Hz are feasible, albeit with some stepping occuring at these high frequencies. Everything <= 100 Hz is smooth.
