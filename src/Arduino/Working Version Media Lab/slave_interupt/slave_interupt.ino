#include "define.h"
#include "helpers.h"
#include "TimeKeeper.h"
#include "BandPassFilter.h"

/*
   This is rewrite initially written by Thomas
   and revised by Yasushi

   Talking drums...
        ___           ___       ___           ___           ___
       /\  \         /\__\     /\  \         /\__\         /\  \
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
#define SERVER_SLAVE 1

//1 -> server
//0 all the little ones.


// Objects
RH_NRF24 nrf24;
TimeKeeper timeKeeper;
TimeKeeper timeKeeperNRF;

//define what sequence or process to execute
bool isTestMic = true;

bool  DEBUG      = false;

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
byte sequenceState = 0;// READ_INPUT;//TEST_MIC; //TEST_MIC;
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
bool debugSequence[] = {0, 0, 0, 1, 0, 0, 1, 1};

bool firstCalibration = true;

float buffSignal[30];
uint8_t maxBuffer = sizeof(buffSignal) / sizeof(float);

bool ledTick = false;

//clock cyles keepers
uint8_t clockCounter = 0;

///Signal Processing
// BandPadd Filter
BandPassFilter bandPassFilter(f_s, bw_s, EMA_a_low_s, EMA_a_high_s, BUFFER_SIZE);

//mic value
int micValue = 0;
int indexMic = 0;

//calibrate numers of NF calls
//in theory we are only going to chance the time once,
//if we change the time, we can recalibrate using the incomming values.
unsigned long nrfTime     = 130L;
unsigned long nrfCallTime = 20L;

//iterators
uint8_t itri = 0;
uint8_t itrj = 0;

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
int8_t clkPWM  = solenoid_pwm;

//value for changing the mic calibration
int8_t clkMIC  = THRESHOLD_PEAK;

int8_t clkTEMP  = 0;



//Slave values
//Serial Port
//request a byte, write the s command
bool requestByte = true;
bool readInBytes = false;

//incoming msg, keep it as an array in case we need to
//read values bigger than a byte
byte byteMSG8[] = {
  B00010011
};

int LIMIT_READ_COUNTER = 80;

void setup() {

  Serial.begin(115200);

  setInitSequence();

  pinMode(LED_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);

  if (sequenceState == TEST_MIC) {
    analogWrite(SOL_PIN, 255);
  }
  digitalWrite(LED_PIN, HIGH);
  delay(100);

  digitalWrite(LED_PIN, LOW);
  analogWrite(SOL_PIN, 0);


  bitIndex = sequenceIndex = 0;

  numHeaderBits =  ((sizeof(correctHeader) / sizeof(bool)) * SEQITER);

  resetSequence(); //resets recording, play and head Sequence

  initNRF(nrf24, false);

  //set intervals
  timeKeeper.setInterval(HIT_INTERVAL);
  timeKeeperNRF.setInterval(nrfTime);

  setupInterrupt();


  //
  Serial.println("MIC: " + THRESHOLD_PEAK);
  Serial.println("Solenoid PWM:" + solenoid_pwm);
}

void loop() {

  cTime = millis();
  // updates the timeKeeper
  timeKeeper.cycle(cTime);
  timeKeeperNRF.cycle(cTime);

  //collect signal readings with the interrupt function

  if (sequenceState == TEST_MIC) {
    bandPassFilter.filterSignal(true);
  } else {
    bandPassFilter.filterSignal();
  }
  // unlocks if we recieve a TICK from the server
  // and timeFrame is more than TIMEFRAMEINTERVAL (60ms)
  //only check server the last 10 ms of the global time.


  if (timeKeeperNRF.isTick() ) {
    valueByte = checkServer(nrf24, clkTICK, clkModuleId, clkTEMP); //10ms  -30count

    //Serial.println(valueByte);

    clockMode(clkTICK, clkTEMP);
  }

  acticateSequenceLoop();

  //update times (now - prev)
  timeKeeper.updateTimes();
  timeKeeperNRF.updateTimes();

  //feedback
  if (sequenceState == RESET ||  sequenceState == ANALYZE || sequenceState == WAIT_START) {
    digitalWrite(LED_PIN, timeKeeper.checkTick());
  }

  //start after 10 ms
  if (sequenceState == TEST_MIC || sequenceState == LISTEN_HEADER || sequenceState == LISTEN_SEQUENCE) {
    digitalWrite(LED_PIN, timeKeeper.checkHit());
  }


  if (sequenceState == PULSE_PLAY || sequenceState == HEADER_PLAY) {
    if (timeKeeper.getTimeHit() > 10L ) {
      if (timeKeeper.checkHit()) {
        analogWrite(SOL_PIN, solenoid_pwm);
        digitalWrite(LED_PIN, HIGH);
      } else {
        analogWrite(SOL_PIN, 0);
        digitalWrite(LED_PIN, 0);
      }
    }
  }

}
