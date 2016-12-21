
#ifndef Define_h
#define Define_h

/// pins
#define LED_PIN 3
#define SOL_PIN 5
//changed for the use of PWM (Analog Write)
#define MIC_PIN A0

/// Clock values

#define TICK    B00000010
#define TOCK    B10000001

#define START   B00000010
#define MIC     B00000011
#define STOP    B10000100


#define PWM255  B00000101
#define PWM200  B00000110
#define PWM150  B00000111
#define PWM100  B00001000
#define PWM50   B00001001
#define PWM0    B00001010
#define DUBUG_C B00001011

// this is the time interval for each frame
// this is one factor to determine the duration of
// data collection from the mic
#define TIME_MIN_INTERVAL 144

/// sequences
#define SEQITER 3
#define SEQBITS 8
#define HEBITS  3


//we might want to constain the number of bits to 8
//this way we can use byte

const unsigned long  HIT_INTERVAL          = 30L;
// this interval is for the outputs, LED and solenoid

//threshold peak for the signal processing
const int THRESHOLD_PEAK = 68; //38

//buffer size of signal processing window
//BUFFER_SIZE = 25 -> 5ms 45 ->10ms
const int BUFFER_SIZE = 25;

const float f_s   = 0.02; //0.023
const float bw_s  = 0.028; //0.25
const float EMA_a_low_s   = 0.15;  //0.18    //initialization of EMA alpha (cutoff-frequency)
const float EMA_a_high_s  = 0.9;  //0.87

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

#endif
