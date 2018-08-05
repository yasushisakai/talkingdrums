#include "define.h"
#include "helpers.h"
#include "TimeKeeper.h"
#include "BandPassFilter.h"

/*
   This is rewrite initially written by Thomas
   and revised by Yasushi

   Talking drums...
        ___           ___       ___           ___           ___
       /\  \         /\__\     /\  \         /\__\     dfv er    /\  \
      /::\  \       /:/  /    /::\  \       /:/  /        /::\  \
     /:/\ \  \     /:/  /    /:/\:\  \     /:/  /        /:/\:\  \
    _\:\~\ \  \   /:/  /    /::\~\:\  \   /:/__/  ___   /::\~\:\  \
   /\ \:\ \ \__\ /:/__/    /:/\:\ \:\__\  |:|  | /\__\ /:/\:\ \:\__\
   \:\ \:\ \/__/ \:\  \    \/__\:\/:/  /  |:|  |/:/  / \:\~\:\ \/__/
    \:\ \:\__\    \:\  \        \::/  /   |:|__/:/  /   \:\ \:\__\
     \:\/:/  /     \:\  \       /:/  /     \::::/__/     \:\ \/__/
      \::/  /       \:\__\     /:/  /       ~~~~          \:\__\
       \/__/         \/__/     \/__/                       \/__/


  This is the slave program, which can be seen in any data passing phase,
  it is also a client, that recieves data from the server

   look for defines.h file for important variables
   you might want to look at TimeKeeper.h for other const variables as well
*/

//define SERVER SLAVE
#define SERVER_SLAVE 2

//1 -> server
//2 -> sender
//0 all the little ones.


// Objects
RH_NRF24 nrf24;
TimeKeeper timeKeeper;
TimeKeeper timeKeeperNRF;

//define what sequence or process to execute
bool isTestMic = true;

//print debug information
bool  DEBUG      = true;
bool  DEBUG_IN   = false;

bool const DEBUG_TIME = false;


bool const useHeader  = true;  // cares about the header or not
//sequence
/*
   TEST_TIMERS   -> test RF and timers
   TEST_MIC      -> test microphone sensor
   TEST_SOLENOID -> test solenoid sensor


   WAIT_START -> sleve start hearing the header.

   READ_INPUT -> server
*/

///Sequence
byte sequenceState = 0;//TEST_MIC;//TEST_SOLENOID;//TEST_MIC;// READ_INPUT;//TEST_MIC; //TEST_MIC;
byte sequenceIndex = 0;
byte bitIndex      = 0;

//hit solenoid if the microphone reads hit, or the sequence has a 1
bool micHit    = false;
bool lock      = true;

//HEADER
bool isRecordHeader = false;
bool isHead         = true; //false
bool isFirstHit     = true;
uint8_t numHeaderBits    = 0;
uint8_t headerBitCounter = 0;

bool correctHeader[] = {1, 1, 0};
bool headerSequence[(sizeof(correctHeader) / sizeof(bool)) * SEQITER];

//SEQUENCE

bool recording[SEQITER][SEQBITS];
bool playSequence[SEQBITS];

bool debugSequence[]    = {0, 0, 0, 1, 0, 0, 1, 1};
bool debugSequenceTap[] = {1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1};


bool firstCalibration = true;

float buffSignal[30];
uint8_t maxBuffer = sizeof(buffSignal) / sizeof(float);

bool ledTick = false;

//clock cyles keepers
uint8_t clockCounter = 0;

///Signal Processing

float f_s   = 0.01; //0.023
float bw_s  = 0.015; //0.25
float EMA_a_low_s   = 0.04;  //0.18    //initialization of EMA alpha (cutoff-frequency)
float EMA_a_high_s  = 0.2;  //0.87

float prev_f_s = 0;
float prev_bw_s = 0;
float prev_EMA_a_low_s = 0;
float prev_EMA_a_high_s = 0;

// BandPadd Filter
BandPassFilter bandPassFilter(f_s, bw_s, EMA_a_low_s, EMA_a_high_s, BUFFER_SIZE);

//mic value
int micValue = 0;
int indexMic = 0;

//calibrate numers of NF calls
//in theory we are only going to chance the time once,
//if we change the time, we can recalibrate using the incomming values.
unsigned long nrfTime     = 240L;
unsigned long nrfCallTime = 10;

//iterators
uint8_t itri = 0;
uint8_t itrj = 0;
uint8_t itrCounter = 0;

//tmp conter
unsigned long tempConter = 0;

unsigned long cTime = 0;

uint8_t valueByte = B00000000;




/// PWM-ing the Solenoid will need additional test 0-255
byte solenoid_pwm = DEFAULT_PWM;

//CLK input Value
uint8_t clkTICK = B10000001;

//id of the module, this is used to check the incomming clock information
uint8_t clkModuleId = byte(TD_ID);

//value for changing the modules, mic calibration, pwm
uint8_t clkPWM  = solenoid_pwm;

//threshold peak for the signal processing
int THRESHOLD_PEAK = 20;//68; //38

//value for changing the mic calibration
uint8_t clkMIC  = THRESHOLD_PEAK;
int     micThreshold  =  THRESHOLD_PEAK;

uint8_t clkTEMP  = 0;

uint8_t clkValue = 0;

uint8_t clkMode = 0;

uint8_t activateNRFMode = 0;

//Slave values
//Serial Port
//request a byte, write the s command
bool requestByte = true;
bool readInBytes = false;

//incoming msg, keep it as an array in case we need to
//read values bigger than a byte
byte byteMSG8[] = {
  B10010011
};

uint8_t inCommingMSg[2] = {B00010000, B00010000};

/*
   36 single sequence, waiting time 36 for the next one to finish its sequence,
   72 in total, 8 steps for buffer.
*/
int LIMIT_READ_COUNTER = 99;

int counterIteratios = 0;
int resetCounter = 0;

bool activateSend = false;



void setup() {

  Serial.begin(115200);

  if (sequenceState != TEST_MIC ) {
    setInitSequence();
  }

  pinMode(LED_PIN, OUTPUT);
  pinMode(SOL_ENABLE, OUTPUT);

  if (sequenceState == TEST_MIC) {
    //analogWrite(SOL_PIN, 255);
    analogWrite(SOL_PHASE, 0);
    digitalWrite(SOL_ENABLE, LOW);
    analogWrite(SOL_SLEEP, 0);
  }
  digitalWrite(LED_PIN, HIGH);
  delay(100);

  digitalWrite(LED_PIN, LOW);
  //analogWrite(SOL_PIN, 0);


  bitIndex = sequenceIndex = 0;

  numHeaderBits =  ((sizeof(correctHeader) / sizeof(bool)) * SEQITER);

  resetSequence(); //resets recording, play and head Sequence

  if (DEBUG)Serial.println("start nrf");
  initNRF(nrf24, false);

  //setSequenceState(CALIBRATE_TIME);

  //set intervals
  timeKeeper.setInterval(HIT_INTERVAL);
  timeKeeperNRF.setInterval(nrfTime);

  setupInterrupt();


  if (DEBUG) Serial.print("MIC: ");
  if (DEBUG) Serial.print(THRESHOLD_PEAK);
  if (DEBUG) Serial.print("  Solenoid PWM: ");
  if (DEBUG)Serial.println(solenoid_pwm);
}


void loop() {

  cTime = millis();
  // updates the timeKeeper
  timeKeeper.cycle(cTime);
  timeKeeperNRF.cycle(cTime);

  //obtain the values from the pots
  //float potCal_1 = analogRead(POT_CAL_01)/1024.0;
  // float potCal_2 = analogRead(POT_CAL_02)/1024.0;
  // float potCal_frq  = (analogRead(POT_POT)/1024.0); // 0 -.25
  // float potCal_band = (analogRead(POT_CAL_01)/1024.0);

  //bandPassFilter.setEMALow(potCal_1);
  //bandPassFilter.setEMAHigh(potCal_2);
  //bandPassFilter.setupFilter(potCal_frq/100.0, potCal_band/100.0);

  //  micThreshold = analogRead(POT_CAL_01);

  
  //collect signal readings with the interrupt function

  if (sequenceState == TEST_MIC) {
    bandPassFilter.filterSignal(true);
  } else {
    bandPassFilter.filterSignal();
    counterIteratios++;
  }
  // unlocks if we recieve a TICK from the server
  // and timeFrame is more than TIMEFRAMEINTERVAL (60ms)
  //only check server the last 10 ms of the global time.


  if (timeKeeperNRF.isTick() ) {
    valueByte = checkServer(nrf24, clkTICK, clkModuleId, clkMode, clkValue, activateNRFMode, inCommingMSg[0], SERVER_SLAVE); //10ms  -30count

    //Serial.println(valueByte);

    clockMode(clkTICK, clkMode, clkValue);
  }

  activateSequenceLoop();

  //update times (now - prev)
  timeKeeper.updateTimes();
  timeKeeperNRF.updateTimes();

  //feedback
  if (sequenceState == RESET ||  sequenceState == ANALYZE || sequenceState == WAIT_START || sequenceState == CALIBRATE_TIME) {
    digitalWrite(LED_PIN, timeKeeper.checkTick());

  }
  //start after 10 ms
  if (sequenceState == TEST_MIC || sequenceState == LISTEN_HEADER || sequenceState == LISTEN_SEQUENCE) {
    digitalWrite(LED_PIN, timeKeeper.checkHit());
  }


  if (sequenceState == PULSE_PLAY || sequenceState == HEADER_PLAY || sequenceState == TEST_SOLENOID) {
    if (timeKeeper.getTimeHit() > 40L ) {
      if (timeKeeper.checkHit()) {

        analogWrite(SOL_PHASE, 255);
        digitalWrite(SOL_ENABLE, HIGH);
        analogWrite(SOL_SLEEP, 255);

        digitalWrite(LED_PIN, HIGH);
      } else {
        //sleep
        analogWrite(SOL_PHASE, 0);
        digitalWrite(SOL_ENABLE, LOW);
        analogWrite(SOL_SLEEP, 0);

        digitalWrite(LED_PIN, 0);
      }
    }
  }

  if (activateSend) {

    activateSend = false;
    setSequenceState(HEADER_PLAY);
    if (DEBUG) Serial.println("PLAY HEADER ");

  }



}
