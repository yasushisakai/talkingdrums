
#ifndef Define_h
#define Define_h

//ID
//Talking Drum ID module
#define TD_ID       1

/// DONT Change this values
#define LED_PIN 3

//pin values for the solenoids
#define SOL_PHASE  5
#define SOL_ENABLE 4
#define SOL_SLEEP  6

//pin values for the pots
#define POT_POT    A1   
#define POT_CAL_01 A2
#define POT_CAL_02 A3

//changed for the use of PWM (Analog Write)
#define MIC_PIN A0

/// Clock values
#define TICK    B00000001

//not Usen int NRF
#define TOCK    B10000000


//different modes.
#define CHANGE_MODULE     B00000001

#define PWM_MODE           0
#define MIC_MODE           1
#define RETURN_MODE        2
#define SONLENOID_MODE     3
#define CALIBRATE_MODE     4
#define DEBUG_MODE         5


#define DEBUG_MODULE      B00001110


//Helper 0
#define ZERO_BYTE         B00000000

//Protocol

#define READY_BYTE   B00000000
#define READ_BYTE    B00000001
#define WRITE_BYTE   B00000010
#define FINAL_BYTE   B00001010

// this is the time interval for each frame
// this is one factor to determine the duration of
// data collection from the mic
#define TIME_MIN_INTERVAL 244

/// sequences
#define SEQITER 3
#define SEQBITS 8
#define HEBITS  3


//we might want to constain the number of bits to 8
//this way we can use byte

const unsigned long  HIT_INTERVAL          = 75L;
// this interval is for the outputs, LED and solenoid

 
const int DEFAULT_PWM  = 255;

//buffer size of signal processing window
//BUFFER_SIZE = 25 -> 5ms 45 ->10ms
const int BUFFER_SIZE = 9;//25


///  modes
// we can do enum if we want

//TEST
#define STOP 0
#define TEST_MIC 1
#define TEST_SOLENOID 3

#define CALIBRATE_MIC 4
#define CALIBRATE_TIME 5

//actual sequence
#define WAIT_START 6

#define LISTEN_HEADER 7
#define LISTEN_SEQUENCE 8

#define ANALYZE 9

#define HEADER_PLAY 10
#define PULSE_PLAY 11
#define WAIT_PLAY 12

#define RESET 13

#define READ_INPUT 14

#define SEND_INPUT 15

#endif
