
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
#define WAIT_START 0
#define LISTEN 1
#define ANALYZE 2
#define HEADER_PLAY 3
#define PULSE_PLAY 4
#define WAIT_PLAY 5
#define RESET 6

#endif
