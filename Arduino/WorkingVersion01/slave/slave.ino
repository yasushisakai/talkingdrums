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

///Sequence
byte sequenceState, sequenceIndex, bitIndex;
bool lock, isRecord;
bool recording[SEQITER][SEQBITS];
bool playSequence[SEQBITS];
bool debugSequence[] = {1, 0, 0, 1, 1, 0, 0, 1};

///Signal Processing
int signalMin, signalMax;
const int signalThreshold = 800; // 50-1024 we may need to make this dynamic

/// PWM-ing the Solenoid will need additional test 0-255
byte const solenoid_pwm = 200;

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

  //reset values
  for (char i = 0; i < SEQBITS; i++) {
    playSequence[i] = false;
    for (char j = 0; j < SEQITER; j++) {
      recording[j][i] = false;
    }
  }

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
  if (checkServer(nrf24, value)) {
    if (value == TICK && timeKeeper.timeFrame > TIMEFRAMEINTERVAL) {
      value = TOCK;
      timeKeeper.tick();
      timeKeeper.flash();
      lock = false;
    }
  }

  if (!lock) {
    switch (sequenceState) {
      case WAIT_START: {
          TimeKeeper::signalCount++;
          if (!TimeKeeper::wait()) {

            if (DEBUG) {
              Serial.println("L: WAIT_START");
            }

            sequenceState = LISTEN;
          }
        }
        break;

      case LISTEN: {
          /*
            listens and looks at the reading
          */
          bool valueHit = false;
          int peakToPeak = abs(signalMax - signalMin); // abs... weird stuff happens
          /* reset */
          signalMax = 0;
          signalMin = 1024;

          if (!isRecord) {
            unsigned long timeFrame = timeKeeper.timeFrameChar();
            if (DEBUG) {
              Serial.print("L: ");
              Serial.print(timeFrame);
              Serial.print(", ");
              Serial.println(peakToPeak);
            }
          }
          if (peakToPeak > signalThreshold) {
            /*
              it might be easier if we use the threshold after (when we aggreagate)
              TODO: revisit when to threshold
            */
            if (!isRecord) {

              Serial.println("L:rec start");

            }
            isRecord = true;
            valueHit = true;
          }

          if (isRecord) {
            unsigned long timeFrame = timeKeeper.timeFrameChar();
            if (DEBUG) {
              Serial.print("L: ");
              Serial.print(", ");
              Serial.print(peakToPeak);
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

            for (int i = 0; i < SEQBITS; i++) {
              float average = 0.0;
              for (int j = 0; j < SEQITER; j++) {
                average += recording[j][i];
              }
              playSequence[i] = average >= 0.5 * SEQITER;
            }

            if (DEBUG) {
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


            isRecord = false;
            sequenceIndex = 0;
            sequenceState = WAIT_PLAY;
            if (DEBUG) Serial.println("Done Analyze");

          } //- anaylze

        }
        break;
      case PLAYPULSE: {
          /*
            plays single pulse
          */
          if (DEBUG) Serial.print(playSequence[bitIndex]);

          if (playSequence[bitIndex]) timeKeeper.hit();

          bitIndex++;
          if (bitIndex == SEQBITS) {
            if (DEBUG) Serial.println("");
            bitIndex = 0;
            sequenceState = WAIT_PLAY;
          }
        }
        break;
      case WAIT_PLAY:
        {

          TimeKeeper::signalCount ++;
          if (!TimeKeeper::wait()) {
            if (DEBUG) Serial.println("L: WAIT_PLAY");

            bitIndex = 0;
            sequenceIndex++;

            // did it play it for enough times??
            if (sequenceIndex > SEQITER) {
              // yes, proceed to reset
              sequenceState = RESET;
            } else {
              // nope go back playing
              sequenceState = PLAYPULSE;
              if (DEBUG) {
                Serial.print("L: playing=");
                Serial.print(sequenceIndex);
                Serial.print(", ");
              }
            }

          }
        }
        break;
      case RESET: {
          /*
            returns to playpulse if there is iterations left to play
            (may not need this phase though)
          */
            if (DEBUG) Serial.println("L: RESET");
            bitIndex = 0;
            sequenceIndex = 0;
            bitIndex = 0;
            sequenceState = LISTEN;

            //reset listen values
            Serial.print("L: r=");
            for (int i = 0; i < SEQBITS; i++)
              Serial.print(playSequence[i]);

            Serial.println();

            // reset values
            sequenceIndex = 0;
            for (char i = 0; i < SEQBITS; i++) {
              playSequence[i] = false;
              for (char j = 0; j < SEQITER; j++) {
                recording[j][i] = false;
              }
            }
          } else {

            if (DEBUG) {
              Serial.print("L: playing=");
              Serial.print(sequenceIndex);
              Serial.print(", ");
            }

            sequenceState = WAIT_START;
        } // case RESET
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

