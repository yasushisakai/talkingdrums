#include "define.h"
#include "helpers.h"

#include "TimeKeeper.h"

//#include <SPI.h>

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

  timeKeeper.cycle();

  char value;
  if (checkServer(nrf24, value)) {
    if (value == TICK && timeKeeper.timeFrame > 60) {
      value = TOCK;
      timeKeeper.tick();
      timeKeeper.flash();
      lock = false;
    }
  }

  if (!lock) {
    switch (sequenceState) {
      case WAIT:
        {
          TimeKeeper::signalCount++;
          if (!TimeKeeper::wait()) sequenceState = LISTEN;
        }
        break;

      case LISTEN: {
          bool valueHit = false;
          int peakToPeak = signalMax - signalMin;
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
    lock = !lock; //unlock
  }

  // outputs
  digitalWrite(LED_PIN, timeKeeper.checkFlash());
  digitalWrite(SOL_PIN, timeKeeper.checkHit());
}

