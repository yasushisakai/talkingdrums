#ifndef Define_h
#define Define_h

/// LED AND SOL
#define LED_PIN 3
#define SOL_PIN 5
//changed for the use of PWM (Analog Write)
//#define MIC_PIN A0

//CHECK LEDS
#define LED_PIN_00 8
#define LED_PIN_01 6
#define LED_PIN_02 4
#define LED_PIN_03 A1
#define LED_PIN_04 A2
#define LED_PIN_05 A3
#define LED_PIN_06 A4
#define LED_PIN_07 A5

/// server values
#define TICK 49
#define TOCK 48

// this is the time interval for each frame
// this is one factor to determine the duration of
// data collection from the mic
const unsigned long TIMEFRAMEINTERVAL  = 60L;

/// sequences
#define SEQITER 3
#define SEQBITS 8
#define HEBITS  3

//we might want to constain the number of bits to 8
//this way we can use byte

// this interval is for the outputs, LED and solenoid
const unsigned long  INTERVAL          = 10L;

// MODES
// we can do enum if we want
#define WAIT_START 0
#define LISTEN 1
#define ANALYZE 2
#define HEADER_PLAY 3
#define PULSE_PLAY 4
#define WAIT_PLAY 5
#define RESET 6

#endif
