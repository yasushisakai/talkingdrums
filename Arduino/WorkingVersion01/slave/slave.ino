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

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOL_PIN, LOW);

  //sequence
  sequenceState = WAIT;
  bitIndex = sequenceIndex = 0;
  lock = true;
  isRecord = false;
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
  if (sequenceState == WAIT || sequenceState == LISTEN) {
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
  char value;
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
      case WAIT: {
          /*
            waits untill its good enough to get peaks
          */
          TimeKeeper::signalCount++;
          if (!TimeKeeper::wait()) sequenceState = LISTEN;
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
            Serial.print("L: ");
            timeKeeper.timeFrameChar();
            Serial.print(", ");
            Serial.println(peakToPeak);
          }
          if (peakToPeak > signalThreshold) {
            /*
              it might be easier if we use the threshold after (when we aggreagate)
              TODO: revisit when to threshold
            */
            if (!isRecord) Serial.println("L:rec start");
            isRecord = true;
            valueHit = true;
          }

          if (isRecord) {
            Serial.print("L: ");
            timeKeeper.timeFrameChar();
            Serial.print(", ");
            Serial.print(peakToPeak);
            Serial.print(", ");
            Serial.print(sequenceIndex);
            Serial.print(", ");
            Serial.print(bitIndex);
            Serial.print(", ");
            Serial.println(valueHit);
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
          TimeKeeper::signalCount++;
          if (TimeKeeper::wait()) sequenceState = LISTEN;

          sequenceIndex++;
          bitIndex = 0;
          if (sequenceIndex < SEQITER) {
            sequenceState = LISTEN;
          } else {
            isRecord = false;
            sequenceIndex = 0;
            for (int i = 0; i < SEQBITS; i++) {
              float average = 0.0;
              for (int j = 0; j < SEQITER; j++) {
                average += recording[j][i];
              }
              playSequence[i] = average >= 0.5 * SEQITER;
            }

            for (int i = 0; i < SEQITER; i++) {
              Serial.print("L: ");
              Serial.print(i);
              Serial.print('=');
              for (int j = 0; j < SEQBITS; j++) {
                Serial.print(recording[i][j]);
              }
              Serial.println();
            }

            Serial.print("L: r=");
            for (int i = 0; i < SEQBITS; i++) {
              Serial.print(playSequence[i]);
            }
            Serial.println();

            // check sequence if its correct
            if (DEBUG) {
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
            }

            sequenceState = PLAYPULSE;
            Serial.print("L: playing=");
            Serial.print(sequenceIndex);
            Serial.print(", ");
          } //- anaylze

        }
        break;
      case PLAYPULSE: {
          /*
            plays single pulse
          */
          Serial.print(playSequence[bitIndex]);
          if (playSequence[bitIndex]) timeKeeper.hit();

          bitIndex++;
          if (bitIndex == SEQBITS) {
            Serial.println("");
            bitIndex = 0;
            sequenceState = REPLAY;
          }
        }
        break;
      case REPLAY: {
          /*
            returns to playpulse if there is something left to play
            (may not need this phase though)
          */

          TimeKeeper::signalCount++;
          if (!TimeKeeper::wait()) {
            sequenceState = PLAYPULSE;
            bitIndex = 0;
            sequenceIndex ++;

            if (sequenceIndex == SEQITER) {
              sequenceIndex = 0;
              bitIndex = 0;
              sequenceState = LISTEN;
            } else {
              Serial.print("L: playing=");
              Serial.print(sequenceIndex);
              Serial.print(", ");
            }
          }
        }
        break;
    }
    lock = !lock;
  }

  // outputs
  digitalWrite(LED_PIN, timeKeeper.checkFlash());
  digitalWrite(SOL_PIN, timeKeeper.checkHit());
}

