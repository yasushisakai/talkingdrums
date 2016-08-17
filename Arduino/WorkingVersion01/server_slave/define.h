
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
#define TIMEFRAMEINTERVAL 60

/// sequences
#define SEQITER 3
#define SEQBITS 8
//we might want to constain the number of bits to 8
//this way we can use byte

#define INTERVAL 50
// this interval is for the outputs, LED and solenoid

///  modes
// we can do enum if we want
#define WAIT 0
#define LISTEN 1
#define ANALYZE 2
#define PLAYPULSE 3
#define RESET_PLAYPULSE 4

#endif
