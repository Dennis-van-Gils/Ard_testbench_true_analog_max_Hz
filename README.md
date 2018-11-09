# Arduino (Atmel SAMD) testbench for true analog-out signal generation using an interrupt timer and direct port access.
Maximum stable DAC output frequency is 20 kHz. That means that acceptable sine waves with frequencies of up to ~500 Hz are feasible, albeit with some stepping occuring at these high frequencies. Sine waves <= 100 Hz are smooth.
