
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
#define TIME_MIN_INTERVAL 60

/// sequences
#define SEQITER 3
#define SEQBITS 8
#define HEBITS  3


//we might want to constain the number of bits to 8
//this way we can use byte

const unsigned long  HIT_INTERVAL          = 30L;
// this interval is for the outputs, LED and solenoid


//threshold peak for the signal processing
const int THRESHOLD_PEAK = 8;

//buffer size of signal processing window
//BUFFER_SIZE = 25 -> 5ms 45 ->10ms
const int BUFFER_SIZE = 17;

const float f_s   = 0.05; //0 -0.5
const float bw_s  = 0.008;
const float EMA_a_low_s   = 0.08;     //initialization of EMA alpha (cutoff-frequency)
const float EMA_a_high_s  = 0.4;

///  modes
// we can do enum if we want

//TEST
#define WAIT_DEBUG 0
#define TEST_MIC 1
#define TEST_SOLENOID 3
#define WAIT_START 4
#define CALIBRATE_MIC 5
#define CALIBRATE_TIME 6


#define LISTEN_HEADER 7
#define LISTEN_SEQUENCE 8
#define ANALYZE 9
#define HEADER_PLAY 10
#define PULSE_PLAY 11
#define WAIT_PLAY 12
#define RESET 13

#endif