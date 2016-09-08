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
bool const DEBUG = true;
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
const int signalThreshold = 500; // 50-1024 we may need to make this dynamic

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
  sequenceState = WAIT_START; // new wait!!!
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
  if (sequenceState == LISTEN || sequenceState == WAIT_START) {
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
      case WAIT_START: {
          /*
            initial wait for 3 cycles for a stable mic reading
          */

          TimeKeeper::signalCount++;
          if (!TimeKeeper::wait()) {

            if (DEBUG) {
              Serial.print("L: WAIT_START ");
              Serial.print(TimeKeeper::signalLimit);
              Serial.print(" ");
              Serial.println(TimeKeeper::signalCount);
            }

            sequenceState = LISTEN;
          }
        }
        break;

      case LISTEN: {
          /*
            listens
            1. listens for the right header
            2. listens for the sequence
          */

          bool valueHit = isHit();

          //
          // detecting the right header
          //
          if (!isRecord) {
            headerSequence[bitIndex] = valueHit;
            isHead = true;

            for (uint8_t i = 0; i <= bitIndex; i++) {
              if (headerSequence[i] != correctHeader[i]) {
                bitIndex = 0;
                isHead = false;
                break;
              }
            }

            // forces the head to pass if careHead is off.
            if (!careHeader) {
              isHead = true;
            }

            if (isHead) {
              bitIndex ++;
              if (bitIndex >= sizeof(correctHeader) / sizeof(bool)) {
                Serial.println("L: found head"); // notify head detection to ImageReciever

                if (DEBUG && !careHeader) {
                  Serial.print("L: h=");
                  for (uint8_t i = 0; i < sizeof(headerSequence) / sizeof(bool); i++)
                    Serial.print(headerSequence[i]);
                  Serial.println();
                }

                isRecord = true;
                bitIndex = 0; //reset!
                clockCounter = 2;
              }
            }
          }
          //
          // recording the sequence
          //
          if (isRecord) {
            if (DEBUG) {
              Serial.print("L: ");
              Serial.print(", ");
              Serial.print(sequenceIndex);
              Serial.print(", ");
              Serial.print(bitIndex);
              Serial.print(", ");
              Serial.println(valueHit);
            }
            recording[sequenceIndex][bitIndex] = valueHit;
            bitIndex++;
            if (bitIndex >= SEQBITS) {
              sequenceState = ANALYZE;
            }
          }

          if (DEBUG) Serial.println(clockCounter);
        }
        break;

      case ANALYZE: {
          /*
            collects and analyses the readings
            gets the average
          */

          sequenceIndex++;
          bitIndex = 0;

          if (sequenceIndex < SEQITER) {
            sequenceState = LISTEN;
          } else {

            //
            // gets the average and defines what it heard to "playSequence"
            //
            for (int i = 0; i < SEQBITS; i++) {
              float average = 0.0;
              for (int j = 0; j < SEQITER; j++) {
                average += recording[j][i];
              }
              playSequence[i] = average >= 0.5 * SEQITER;
            }

            sequenceIndex = 0;
            sequenceState = PULSE_PLAY;

            //
            //
            // IMPORTANT!! this line is for the ImageReciever app!!
            //
            //
            Serial.print("L: r=");
            for (int i = 0; i < SEQBITS; i++)
              Serial.print(playSequence[i]);

            Serial.println();
            
            //
            // prints the recordings
            //
            if (DEBUG) {

              Serial.println("L: Done Analyze");

              for (int i = 0; i < SEQITER; i++) {
                Serial.print("L: ");
                Serial.print(i);
                Serial.print('=');
                for (int j = 0; j < SEQBITS; j++) {
                  Serial.print(recording[i][j]);
                }
                Serial.println();
              }

              // check sequence if its correct

              bool flag = true;
              for (int i = 0; i < SEQBITS; i++) {
                if (debugSequence[i] != playSequence[i]) {
                  flag = !flag;
                  break;
                }
              }
              if (flag) {
                Serial.println("L:sequence correct");
              } else {
                Serial.println("L:sequence incorrect");
              }

              Serial.print("L: playing=");
              Serial.print(sequenceIndex);
              Serial.print(", ");
            }

          }

          if (DEBUG) Serial.println(clockCounter);
        }
        break;
      case PULSE_PLAY: {
          /*
            plays single pulse
          */

          if (isHead) {
            if (headerSequence[bitIndex]) timeKeeper.hit();
            bitIndex++;
            if (bitIndex >= sizeof(correctHeader) / sizeof(bool)) {
              isHead = false;
              bitIndex = 0;
              // even if it ends playing the header,
              // it won't go to WAIT_PLAY
            }
          } else {
            if (DEBUG) Serial.print(playSequence[bitIndex]);

            if (playSequence[bitIndex]) timeKeeper.hit();

            bitIndex++;
            if (bitIndex == SEQBITS) {
              if (DEBUG) Serial.println("");
              bitIndex = 0;
              sequenceState = WAIT_PLAY;
            }

            if (DEBUG) Serial.println(clockCounter);
          }
        }
        break;
      case WAIT_PLAY:
        {
          if (DEBUG)Serial.println("Waiting play");

          bitIndex = 0;
          sequenceIndex++;

          // did it play it for enough times??
          if (sequenceIndex > SEQITER) {
            // yes, proceed to reset
            sequenceState = RESET;
          } else {
            // nope go back playing
            sequenceState = PULSE_PLAY;
            if (DEBUG) {
              Serial.print("L: playing=");
              Serial.print(sequenceIndex);
              Serial.print(", ");
            }
          }

          if (DEBUG) Serial.println(clockCounter);

        }
        break;
      case RESET: {
          /*
            returns to PULSE_PLAY if there is iterations left to play
            (may not need this phase though)
          */

          Serial.println("L: end");
          
          // the whole process (including the first head detection) is 61 steps.
          // head + (SEQBITS + gap)*SEQITER + head + (SEQBITS + gap) * SEQITER + RESET
          // 3 + (8+1)*3 + 3 + (8+1)*3 + 1 = 30 + 30 + 1 = 61
          // https://docs.google.com/spreadsheets/d/1OzL0YygAY_DSaA0wT8SLMD5zovmk4wVnEPwZ80VJpbA/edit#gid=0

          // starting from 0, so end is 60
          if (clockCounter == 60) {

            if (DEBUG) Serial.println("L: RESET");
            if (DEBUG) Serial.println(clockCounter);

            clockCounter = 0;
            sequenceState = LISTEN;

            // reset values
            bitIndex = 0;
            sequenceIndex = 0;
            resetSequence();

            isRecord = false;

          }

          if (DEBUG) Serial.println(clockCounter);

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
