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

//define what sequence or process to execute
bool isTestMic = false;

///DEBUG
bool const DEBUG = false;
bool const DEBUG_TIME = true;
bool const careHeader = true; // cares about the header or not


///Sequence
byte sequenceState = 0;
byte sequenceIndex = 0;
byte bitIndex      = 0;

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

///Signal Processing
int signalMin, signalMax;
float hitSensitivity = 0.99; //signal clean

float avgValue        = 0;
uint8_t counterSignal = 0;
uint8_t indexBuffer   = 0;
bool firstCalibration = true;

float buffSignal[30];
uint8_t maxBuffer = sizeof(buffSignal) / sizeof(float);

bool ledTick = false;

float signalThreshold = 2.2; // 50-1024 we may need to make this dynamic

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
  sequenceState = WAIT_DEBUG;//WAIT_START; // new wait!!!
  bitIndex = sequenceIndex = 0;


  numHeaderBits =  ((sizeof(correctHeader) / sizeof(bool)) * SEQITER);

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
  if (sequenceState <= 5) {
    int micValue = analogRead(MIC_PIN);
    if (micValue > 50 && micValue < 1023) {
      if (micValue > signalMax) signalMax = micValue;
      if (micValue < signalMin) signalMin = micValue;
    }
  }



  // unlocks if we recieve a TICK from the server
  // and timeFrame is more than TIMEFRAMEINTERVAL (60ms)
  uint8_t valueByte = B00000001;
  valueByte = checkServer(nrf24);
  if (valueByte == TICK && timeKeeper.getTimeTick() > TIMEFRAMEINTERVAL) {
    timeKeeper.tick();
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
  delay(1);



  if (!lock) {
    switch (sequenceState) {

      //wait for debug time
      case WAIT_DEBUG:
        {
          debugTimes();
        }
        break;
      case TEST_MIC:
        {
          micHit = isHit();
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
            sequenceState = CALIBRATE_MIC;

            TimeKeeper::signalLimit  = 2;
          }
          TimeKeeper::signalCount++;

          if (DEBUG) Serial.println(clockCounter);

        }
        break;
      case CALIBRATE_MIC:
        {
          if (DEBUG) Serial.println("CALIBRATING");
          float pTp = getPeak();
          bool finishCalibration = micCalibration(pTp);
          if (finishCalibration) {
            if (isTestMic) {
              sequenceState =  TEST_MIC;
            } else {
              sequenceState = LISTEN_HEADER;
            }
          }
        }
        break;

      case LISTEN_HEADER:
        {
          if (DEBUG) Serial.print("LISTEN HEADER ");

          micHit = isHit();

          if (isRecordHeader) {

            headerSequence[bitIndex] = micHit;
            bitIndex++;

            isHead = true;

            //Analyze to pass to the next stage
            if (bitIndex == numHeaderBits) {
              if (DEBUG)Serial.print("AH ");
              Serial.println("");
              for (int i = 0; i < numHeaderBits; i++) {
                Serial.print(headerSequence[i]);
              }
              Serial.println("");
              
              
              double headers[SEQITER];
              for (uint8_t i = 0; i < SEQITER; i++) headers[i] = 0.0;

              for (uint8_t i = 0; i < SEQITER; i++) {
                for (uint8_t j = 0; j < 3; j++) {
                  headers[i] +=  headerSequence[i + j*SEQITER];
                }
              }

              int countCheck = 0;
              for (uint8_t i = 0; i < SEQITER; i++) {
                if (correctHeader[i] != (headers[i] / SEQITER) > 0.5 ) {
                countCheck++;
                }
              }

              //OK to have one error, its the header
              isHead = (countCheck <= 1) ? true : false;

              //RESET HEADER
              //if didn't found the header then reset the values of headerSequence
              if (isHead == false) {
                for (uint8_t i = 0; i < numHeaderBits; i++) {
                  headerSequence[i] = false;
                }
                isRecordHeader = false;
                bitIndex = 0;
                isFirstHit = true;
                micHit = false;
                if (DEBUG) Serial.print("RH ");
              }

              // forces the head to pass if careHead is off.
              if (!careHeader) {
                isHead = true;
              }
            }


            if (DEBUG) Serial.print("B: ");
            if (DEBUG) Serial.print(bitIndex);
            if (DEBUG) Serial.print(" ");

            if (isHead && bitIndex == numHeaderBits) {
              Serial.println("L: found head"); // notify head detection to ImageReciever

              if (DEBUG) {
                Serial.print("L: h=");
                for (uint8_t i = 0; i < numHeaderBits; i++)
                  Serial.print(headerSequence[i]);
                Serial.println();
              }

              //go to listen the sequence  and RESET values
              bitIndex = 0; //reset!
              clockCounter = 3;
              micHit = false;
              isFirstHit = true;
              sequenceState = LISTEN_SEQUENCE;

            }
          }



          //if we found a hit then we can start anaylzing the header
          if (isFirstHit && micHit) {
            headerSequence[bitIndex] = micHit;
            bitIndex ++;
            isRecordHeader = true;
            isFirstHit     = false;
            if (DEBUG) Serial.print("FH ");
          }


          if (DEBUG) Serial.println(clockCounter);

        }
        break;
      case LISTEN_SEQUENCE: {
          /*
            listens
            1. listens for the right header
            2. listens for the sequence
          */

          if (DEBUG) Serial.print("LISTEN SEQUENCE");

          micHit = isHit();

          if (DEBUG) {
            Serial.print("L: ");
            Serial.print(sequenceIndex);
            Serial.print(", ");
            Serial.print(bitIndex);
            Serial.print(", ");
            Serial.print(micHit);
            Serial.print(" ");
          }
          recording[sequenceIndex][bitIndex] = micHit;
          bitIndex++;
          if (bitIndex >= SEQBITS) {
            sequenceState = ANALYZE;
          }


          if (DEBUG) Serial.println(clockCounter);
        }

        break;

      case ANALYZE:
        {
          /*
            collects and analyses the readings
            gets the average
          */
          sequenceIndex++;
          bitIndex = 0;

          if (sequenceIndex < SEQITER) {
            if (DEBUG) Serial.print("WAIT ANALYZE  ");
            sequenceState = LISTEN_SEQUENCE;
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
          if (sequenceIndex == SEQITER) {
            // yes, proceed to reset
            sequenceState = RESET;
          } else {
            // nope go back playing
            sequenceState = PULSE_PLAY;
            if (DEBUG) {
              Serial.print("L: playing= ");
              Serial.print(sequenceIndex);
              Serial.print(" ");
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
          sequenceState = LISTEN_HEADER;

          // reset values
          bitIndex = 0;
          sequenceIndex = 0;

          isFirstHit     = true;
          isRecordHeader = false;
          isHead         = false;

          resetSequence();

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

  //use LED for feedback clock
  bool tick =  timeKeeper.checkTick();
  digitalWrite(LED_PIN, tick);

  //use mic has feedback device.
 // digitalWrite(LED_PIN, micHit);

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
  float peakToPeak =  ((signalMax - signalMin) * 5.0) / 1024.0; // abs... weird stuff happens


  valueHit = (peakToPeak * hitSensitivity ) > signalThreshold;

  // show heartbeat

  if (DEBUG) {
    unsigned long timeFrame = timeKeeper.getTimeTick();
    Serial.print("H: ");
    Serial.print(timeFrame);
    Serial.print(", ");
    Serial.print(peakToPeak);
    Serial.print(", ");
    Serial.print(bitIndex);
    Serial.print(", ");
    Serial.print(valueHit);
    Serial.print(" ");
    Serial.print(signalMax);
    Serial.print(" ");
    Serial.println(signalMin);
  }

  signalMax = 0;
  signalMin = 1024;

  return valueHit;
}

float getPeak() {

  float peakToPeak =  ((signalMax - signalMin) * 5.0) / 1024.0; // abs... weird stuff happens

  // show heartbeat
  if (DEBUG) {
    unsigned long timeFrame = timeKeeper.getTimeTick();
    Serial.print("H: ");
    Serial.print(timeFrame);
    Serial.print(", ");
    Serial.print(peakToPeak);
    Serial.print(" ");
    Serial.print(signalMax);
    Serial.print(" ");
    Serial.println(signalMin);
  }

  signalMax = 0;
  signalMin = 1024;

  return peakToPeak;
}

bool micCalibration(float & pTp)
{
  bool finish = false;
  //dynamically change threshold based on the avg of maxBuffer steps
  buffSignal[indexBuffer] = pTp * hitSensitivity;
  indexBuffer++;

  //dont read the first 5 values
  if (firstCalibration) {
    if ( indexBuffer >= 6) {
      indexBuffer = 0;
      firstCalibration = false;
    }

  }

  if (indexBuffer >= maxBuffer) {
    indexBuffer = 0;

    float minThres = 5.0;
    float maxThres = 0.0;
    float avgThres = 0.0;

    for (int i = 0;  i < maxBuffer; i++) {
      float val = buffSignal[i];
      if (val > maxThres) maxThres = val;
      if (val < minThres) minThres = val;
      avgThres += val;
    }
    avgThres /= (float)maxBuffer;
    float diffThres = maxThres - minThres;
    if (DEBUG) {

      Serial.print(minThres);
      Serial.print("  ");
      Serial.print(maxThres);
      Serial.print("  ");
      Serial.print(signalThreshold);
      Serial.print("  ");
      Serial.print(diffThres);
      Serial.print("  new: ");
    }

    //new threshold
    signalThreshold =  avgThres + (diffThres) * 0.82; //78% sensitive
    if (DEBUG)Serial.println(signalThreshold);
    finish = true;
  }

  return finish;
}



void resetSequence() {
  //reset sequences
  for (uint8_t i = 0; i < SEQBITS; i++) {
    playSequence[i] = false;
    for (uint8_t j = 0; j < SEQITER; j++) {
      recording[j][i] = false;
    }
  }

  //reset header
  for (uint8_t i = 0; i < numHeaderBits; i++) {
    headerSequence[i] = false;
  }

}
