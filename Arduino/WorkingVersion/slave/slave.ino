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
bool const DEBUG_TIME = false;
bool const careHeader = false; // cares about the header or not


///Sequence
byte sequenceState = 0;
byte sequenceIndex = 0;
byte bitIndex      = 0;

bool lock      = true;
bool isRecord  = false;
bool isHead    = false;

bool recording[SEQITER][SEQBITS];
bool playSequence[SEQBITS];
bool correctHeader[] = {1, 1, 0};
bool headerSequence[sizeof(correctHeader) / sizeof(bool)];
bool debugSequence[] = {0, 0, 0, 1, 0, 0, 1, 1};

///Signal Processing
int signalMin, signalMax;

float avgValue    = 0;
int counterSignal = 0;

const int signalThreshold = 400; // 50-1024 we may need to make this dynamic

/// PWM-ing the Solenoid will need additional test 0-255
byte const solenoid_pwm = 255;

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

  resetSequence(); //resets recording, play and head Sequence

  //signal
  signalMin = 1024;
  signalMax = 0;

  initNRF(nrf24);
}

void loop() {

  unsigned long cTime = millis();
  // updates the timeKeeper
  timeKeeper.cycle(cTime);


  //collect signal readings
  if (sequenceState == LISTEN || sequenceState == WAIT_START) {
    int micValue = analogRead(MIC_PIN);
    if (micValue < 1023 && micValue > 50) { // for weird readings??
      if (micValue > signalMax) signalMax = micValue;
      if (micValue < signalMin) signalMin = micValue;
    }

  }

  // unlocks if we recieve a TICK from the server
  // and timeFrame is more than TIMEFRAMEINTERVAL (60ms)
  uint8_t valueByte = B00000001;

  if (checkServer(nrf24, valueByte)) {
    if (valueByte == TICK && timeKeeper.getTimeTick() > TIMEFRAMEINTERVAL) {
      valueByte = TOCK;
      timeKeeper.tick();
      lock = false;
      clockCounter++;

      if (DEBUG_TIME) {
        Serial.print("MSG ");
        Serial.println(timeKeeper.getTimeTick());
      }

    }
  }
  delay(2);

  if (!lock) {
    switch (sequenceState) {

      //wait for debug time
      case WAIT:
        {
          debugTimes();
        }
        break;
      case WAIT_START: {
          /*
            initial wait for 3 cycles for a stable mic reading
          */

          if (!TimeKeeper::wait()) {

            if (DEBUG) {
              Serial.print("L: WAIT_START ");
              Serial.print(TimeKeeper::signalLimit);
              Serial.print(" ");
              Serial.print(TimeKeeper::signalCount);
              Serial.print(" ");
            }

            sequenceState = LISTEN;
            TimeKeeper::signalLimit  = 1;
          }
          TimeKeeper::signalCount++;

          if (DEBUG) Serial.println(clockCounter);

        }
        break;

      case LISTEN: {
          /*
            listens
            1. listens for the right header
            2. listens for the sequence
          */

          if (DEBUG) Serial.print("LISTEN ");

          bool valueHit = isHit();

          // recording the sequence
          //
          if (isRecord) {
            if (DEBUG) {
              Serial.print("L: ");
              Serial.print(sequenceIndex);
              Serial.print(", ");
              Serial.print(bitIndex);
              Serial.print(", ");
              Serial.print(valueHit);
              Serial.print(" ");
            }
            recording[sequenceIndex][bitIndex] = valueHit;
            bitIndex++;
            if (bitIndex >= SEQBITS) {
              sequenceState = ANALYZE;
            }
          }


          //
          // detecting the right header
          //
          if (!isRecord) {

            headerSequence[bitIndex] = valueHit;
            isHead = true;
            bitIndex ++;

            //Analyze to pass to the next stage
            int numBits =  (sizeof(correctHeader) / sizeof(bool));

            if (bitIndex == numBits) {
              for (uint8_t i = 0; i <= numBits; i++) {
                if (headerSequence[i] != correctHeader[i]) {
                  bitIndex = 0;
                  isHead = false;
                  break;
                }
              }

              // forces the head to pass if careHead is off.
              if (!careHeader) {
                isHead = true;
                bitIndex = 3;
              }

            }

            if (isHead) {

              if (DEBUG) Serial.print("B: ");
              if (DEBUG) Serial.print(bitIndex);
              if (DEBUG) Serial.print(" ");


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
                clockCounter = 3;
              }
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
            if (DEBUG) Serial.print("WAIT ANALYZE  ");
            sequenceState = LISTEN;
          } else {
            if (DEBUG) Serial.print("ANALYZING  ");
            //
            // gets the average and defines what it heard to "playSequence"
            //
            for (int i = 0; i < SEQBITS; i++) {
              float average = 0.0;
              for (int j = 0; j < SEQITER; j++) {
                average += recording[j][i];
                if (DEBUG) {
                  Serial.print(" ");
                  Serial.print(average);
                  Serial.print(" ");
                }
              }
              playSequence[i] = average >= 0.5 * SEQITER;
              if (DEBUG) Serial.println(playSequence[i]);
            }

            sequenceIndex = 0;
            sequenceState = PULSE_PLAY;

            //make sure that we are going to play the header
            isHead = true;

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
          if (DEBUG) Serial.print("PLAY  ");

          if (isHead) {
            if (headerSequence[bitIndex]) timeKeeper.hit();
            bitIndex++;

            if (DEBUG) Serial.print(headerSequence[bitIndex]);
            
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
              bitIndex = 0;
              sequenceState = WAIT_PLAY;
            }
          }

          if (DEBUG) Serial.println(clockCounter);
        }
        break;
      case WAIT_PLAY:
        {
          if (DEBUG)Serial.print("Waiting play ");

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
              Serial.print("L: playing= ");
              Serial.print(sequenceIndex);
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

          // the whole process (including the first head detection) is 61 steps.
          // head + (SEQBITS + gap)*SEQITER + head + (SEQBITS + gap) * SEQITER + RESET
          // 3 + (8+1)*3 + 3 + (8+1)*3 + 1 = 30 + 30 + 1 = 61
          // https://docs.google.com/spreadsheets/d/1OzL0YygAY_DSaA0wT8SLMD5zovmk4wVnEPwZ80VJpbA/edit#gid=0

          // starting from 0, so end is 60

          if (DEBUG) Serial.print("L: RESET ");

          clockCounter = 0;
          sequenceState = LISTEN;

          // reset values
          bitIndex = 0;
          sequenceIndex = 0;
          resetSequence();

          isRecord = false;


          if (DEBUG) Serial.println(clockCounter);

        }
        break;
    } // switch
    lock = !lock;
  }

  //update times (now - prev)
  timeKeeper.updateTimes();

  // outputs
  bool hit = timeKeeper.checkHit();
  digitalWrite(LED_PIN, timeKeeper.checkTick());

  if (hit) {
    analogWrite(SOL_PIN, solenoid_pwm);
  } else {
    analogWrite(SOL_PIN, 0);
  }

}




bool debugTimes()
{
  if (DEBUG) {
    Serial.print("T: ");
    Serial.print(timeKeeper.getTimeHit());
    Serial.print(" ");
    Serial.print(timeKeeper.getTimeTick());
    Serial.print(" ");
    Serial.print(timeKeeper.checkHit());
    Serial.print(" ");
    Serial.println(timeKeeper.checkTick());
  }
}

bool isHit() {

  bool valueHit = false;
  int peakToPeak = abs(signalMax - signalMin); // abs... weird stuff happens

  valueHit = peakToPeak > signalThreshold;
  //
  // reset signal Max and Min
  //
  signalMax = 0;
  signalMin = 1024;

  // show heartbeat
  /*
    if (DEBUG) {
    unsigned long timeFrame = timeKeeper.getTimeHit();
    Serial.print("H: ");
    Serial.print(timeFrame);
    Serial.print(", ");
    Serial.print(peakToPeak);
    Serial.print(", ");
    Serial.print(bitIndex);
    Serial.print(", ");
    Serial.println(valueHit);
    }
  */


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
