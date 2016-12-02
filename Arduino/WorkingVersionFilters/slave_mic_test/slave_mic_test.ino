#include "define.h"
#include "helpers.h"
#include "TimeKeeper.h"

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

///DEBUG
bool const DEBUG = false;
bool const careHeader = true; // cares about the header or not


///Sequence
byte sequenceState, sequenceIndex, bitIndex;
bool lock, isRecord, isHead;
bool recording[SEQITER][SEQBITS];
bool playSequence[SEQBITS];
bool correctHeader[] = {1, 1, 0};
bool headerSequence[sizeof(correctHeader) / sizeof(bool)];
bool debugSequence[] = {1, 0, 0, 1, 1, 0, 0, 1};

///Signal Processing
int signalMin, signalMax;

float avgValue    = 0;
int counterSignal = 0;

const int signalThreshold = 260; // 50-1024 we may need to make this dynamic

/// PWM-ing the Solenoid will need additional test 0-255
byte const solenoid_pwm = 200;

//clock cyles keepers
uint8_t clockCounter = 0;

void setup() {
  Serial.begin(19200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOL_PIN, LOW);

  //sequence
  sequenceState = LISTEN; // new wait!!!
  bitIndex = sequenceIndex = 0;
  lock = true;
  isRecord = false;
  isHead = false;

  resetSequence(); //resets recording, play and head Sequence

  //signal
  signalMin = 1024;
  signalMax = 0;

  initNRF(nrf24);
}

void loop() {
  //collect signal readings
  if (sequenceState == LISTEN) {
    int micValue = analogRead(MIC_PIN);
    if (micValue < 1023 && micValue > 50) { // for weird readings??
      if (micValue > signalMax) signalMax = micValue;
      if (micValue < signalMin) signalMin = micValue;
    }
  }

  // updates the timeKeeper
  timeKeeper.cycle();

  // unlocks if we recieve a TICK from the server
  // and timeFrame is more than TIMEFRAMEINTERVAL (60ms)
  uint8_t value;
  delay(1);
  if (checkServer(nrf24, value)) {
    if (value == TICK && timeKeeper.timeFrame > TIMEFRAMEINTERVAL) {
      value = TOCK;
      timeKeeper.tick();
      timeKeeper.flash();
      lock = false;

      clockCounter++;
    }
  }

  if (!lock) {
    switch (sequenceState) {
      case LISTEN:
        {
          bool valueHit = false;
          int peakToPeak = abs(signalMax - signalMin); // abs... weird stuff happens

          Serial.write(peakToPeak);

          // show heartbeat
          unsigned long timeFrame = timeKeeper.timeFrameChar();
          if (DEBUG) {
            Serial.print("L: ");
            Serial.print(timeFrame);
            Serial.print(", ");
            Serial.println(peakToPeak);
          }

          //
          // reset signal Max and Min
          //
          signalMax = 0;
          signalMin = 1024;

          
        }
        break;
    } // switch
    lock = !lock;
  }

  // outputs
  digitalWrite(LED_PIN, timeKeeper.checkFlash());
  digitalWrite(SOL_PIN, timeKeeper.checkHit());

  if (timeKeeper.checkHit()) {
    analogWrite(SOL_PIN, solenoid_pwm);
  } else {
    analogWrite(SOL_PIN, 0);
  }
}


bool isHit() {

  bool valueHit = false;
  int peakToPeak = abs(signalMax - signalMin); // abs... weird stuff happens

  // show heartbeat
  if (DEBUG) {
    unsigned long timeFrame = timeKeeper.timeFrameChar();
    Serial.print("L: ");
    Serial.print(timeFrame);
    Serial.print(", ");
    Serial.println(peakToPeak);
  }

  //
  // reset signal Max and Min
  //
  signalMax = 0;
  signalMin = 1024;

  valueHit = peakToPeak > signalThreshold;

  return valueHit;
}




void resetSequence() {
  //reset sequences
  for (char i = 0; i < SEQBITS; i++) {
    playSequence[i] = false;
    for (char j = 0; j < SEQITER; j++) {
      recording[j][i] = false;
    }
  }

  //reset header
  for (uint8_t i = 0; i < sizeof(correctHeader) / sizeof(bool); i++) {
    headerSequence[i] = false;
  }

}
