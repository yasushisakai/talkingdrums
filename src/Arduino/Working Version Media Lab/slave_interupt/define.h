
#ifndef Define_h
#define Define_h

//ID
//Talking Drum ID module
#define TD_ID       12

/// DONT Change this values
#define LED_PIN 3
#define SOL_PIN 5
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


#define DEBUG_MODULE      B00001110


//Helper 0
#define ZERO_BYTE         B00000000


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

const unsigned long  HIT_INTERVAL          = 40L;
// this interval is for the outputs, LED and solenoid

//threshold peak for the signal processing
const int THRESHOLD_PEAK = 90;//68; //38
 
const int DEFAULT_PWM  = 255;

//buffer size of signal processing window
//BUFFER_SIZE = 25 -> 5ms 45 ->10ms
const int BUFFER_SIZE = 25;//25

const float f_s   = 0.023; //0.023
const float bw_s  = 0.025; //0.25
const float EMA_a_low_s   = 0.18;  //0.18    //initialization of EMA alpha (cutoff-frequency)
const float EMA_a_high_s  = 0.87;  //0.87

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
