
#ifndef Define_h
#define Define_h

/// pins
#define LED_PIN 3
#define SOL_PIN 5
//changed for the use of PWM (Analog Write)
#define MIC_PIN A0

/// server values
#define TICK B00000001
#define TOCK B00000000

// this is the time interval for each frame
// this is one factor to determine the duration of
// data collection from the mic
#define TIMEFRAMEINTERVAL 60

/// sequences
#define SEQITER 3
#define SEQBITS 8
#define HEBITS  3

//we might want to constain the number of bits to 8
//this way we can use byte

#define INTERVAL 30
// this interval is for the outputs, LED and solenoid

///  modes
// we can do enum if we want
#define WAIT 0
#define WAIT_START 1
#define LISTEN 2
#define ANALYZE 3
#define HEADER_PLAY 4
#define PULSE_PLAY 5
#define WAIT_PLAY 6
#define RESET 7

#endif
