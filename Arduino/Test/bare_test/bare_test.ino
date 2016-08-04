#include "define.h"
#include "helpers.h"

#include "TimeKeeper.h"

#include <SPI.h>

RH_NRF24 nrf24;
TimeKeeper timekeeper;

///Sequence
char sequenceState, sequenceIndex, recordIndex;
boolean isRecord;
boolean isPlay;
boolean recording[SEQITER][SEQBITS];
boolean playing[SEQBITS];

///Signal Processing
int signalMin, signalMax;
const int signalThreshold = 600; // 50-1024 we may need to make this dynamic

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOL_PIN, LOW);

  //sequence
  sequenceState = WAIT;
  recordIndex = sequenceIndex = 0;
  isRecord = isPlay = false;
  for (char i = 0; i < SEQBITS; i++) {
    playing[i] = false;
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
  timekeeper.cycle();

  char value;
  if (checkServer(nrf24, value)) {
    if (value == TICK) {
      timekeeper.tick();
      timekeeper.flash();
    }
  }

  //collect signal readings
  if (sequenceState == WAIT || sequenceState == LISTEN) {
    int micValue = analogRead(MIC_PIN);
    if (micValue < 1023 && micValue > 50) { // for weird readings??
      if (micValue > signalMax) signalMax = micValue;
      if (micValue < signalMin) signalMin = micValue;
    }
  }

  //
  switch (sequenceState) {
    case WAIT:
      {
        TimeKeeper::signalCount++;
        if (!TimeKeeper::wait()) sequenceState = LISTEN;
      }
      break;

    case LISTEN: {
        boolean valueHit = false;
        int peakToPeak = signalMax - signalMin;
        if (peakToPeak > signalThreshold) {
          isRecord = true;
          valueHit = true;
          Serial.println("L:start record");
        }

        if (isRecord) {
          recording[sequenceIndex][recordIndex] = valueHit;
          recordIndex++;
          if (recordIndex > SEQBITS) {
          recordIndex=0;
          }
        }
      }
      break;

    case ANALYZE: {
      }
      break;
    case PLAYPULSE: {
      }
      break;
    case REPLAY: {
      }
      break;
  }

  // outputs
  digitalWrite(LED_PIN, timekeeper.checkFlash());
  digitalWrite(SOL_PIN, timekeeper.checkHit());
}


