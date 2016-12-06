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

// Objects
RH_NRF24 nrf24;
TimeKeeper timeKeeper;
TimeKeeper timeKeeperNRF;

//define what sequence or process to execute
bool isTestMic = true;

bool const DEBUG = true;
bool const DEBUG_TIME = false;
bool const careHeader = true; // cares about the header or not


///Sequence
byte sequenceState = 0;
byte sequenceIndex = 0;
byte bitIndex      = 0;

//hit solenoid if the microphone reads hit, or the sequence has a 1
bool micHit    = false;
bool lock      = true;

//HEADER
bool isRecordHeader = false;
bool isHead         = false;
bool isFirstHit     = true;
uint8_t numHeaderBits   = 0;

bool recording[SEQITER][SEQBITS];
bool playSequence[SEQBITS];
bool correctHeader[] = {1, 1, 0};
bool headerSequence[SEQITER * (sizeof(correctHeader) / sizeof(bool))];
bool debugSequence[] = {0, 0, 0, 1, 0, 0, 1, 1};

bool firstCalibration = true;

float buffSignal[30];
uint8_t maxBuffer = sizeof(buffSignal) / sizeof(float);

bool ledTick = false;

/// PWM-ing the Solenoid will need additional test 0-255
byte const solenoid_pwm = 155;

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
unsigned long nrfTime     = 90L;
unsigned long nrfCallTime = 20L;

//iterators
uint8_t itri = 0;
uint8_t itrj = 0;

//tmp conter
unsigned long tempConter = 0;

unsigned long cTime = 0;

uint8_t valueByte = B00000000;

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);


  digitalWrite(LED_PIN, HIGH);
  delay(100);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOL_PIN, LOW);

  //sequence
  sequenceState = TEST_MIC;//WAIT_DEBUG;//WAIT_START; // new wait!!!
  bitIndex = sequenceIndex = 0;

  numHeaderBits =  ((sizeof(correctHeader) / sizeof(bool)) * SEQITER);

  resetSequence(); //resets recording, play and head Sequence

  initNRF(nrf24);

  //set intervals
  timeKeeper.setInterval(HIT_INTERVAL);
  timeKeeperNRF.setInterval(nrfTime);
}

void loop() {

  cTime = millis();
  // updates the timeKeeper
  timeKeeper.cycle(cTime);
  timeKeeperNRF.cycle(cTime);

  //collect signal readings
  if (sequenceState <= 8) {
    micValue = analogRead(MIC_PIN);
    bandPassFilter.fillWindow(cTime, micValue, true, false);
  }

  // unlocks if we recieve a TICK from the server
  // and timeFrame is more than TIMEFRAMEINTERVAL (60ms)
  //only check server the last 10 ms of the global time.
  if (timeKeeperNRF.isTick() ) {
    valueByte = checkServer(nrf24); //10ms  -30count

    //    Serial.println(valueByte);

    if (valueByte == TICK && timeKeeper.getTimeTick() >= TIME_MIN_INTERVAL) {
      timeKeeper.tick();
      timeKeeperNRF.tick();

      if (DEBUG_TIME) {
        Serial.print("MSG ");
        Serial.print(valueByte);
        Serial.print(" ");
        Serial.println(timeKeeper.getTimeTick());
      }
      lock = false;
      clockCounter++;
      valueByte = TOCK;
    }
  }

  acticateSequenceLoop();

  //update times (now - prev)
  timeKeeper.updateTimes();
  timeKeeperNRF.updateTimes();

  //start after 10 ms
  if (timeKeeper.getTimeHit() > 10L ) {
    if (timeKeeper.checkTick()) {
      analogWrite(SOL_PIN, solenoid_pwm);
      digitalWrite(LED_PIN, HIGH);
    } else {
      analogWrite(SOL_PIN, 0);
      digitalWrite(LED_PIN, 0);
    }
  }
  
}
