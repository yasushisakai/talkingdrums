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

unsigned long cTime =0;

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
      //Serial.println("reset");
    }

  }


  if (!lock) {
    switch (sequenceState) {

      //debug timers
      case WAIT_DEBUG:
        {
          debugTimes();
        }
        break;
      //debug mic
      case TEST_MIC:
        {
          micHit = bandPassFilter.isHit();

          if (micHit == true) {
            //enable solenoid a single hit 30ms
            timeKeeper.hit();
          }
          
        }
        break;
      case TEST_SOLENOID:
        {

          micHit = debugSequence[itri];
          if (micHit == true) {
            timeKeeper.hit();
          }

          //reset counter
          itri++;
          if (itri >= 8)
            itri = 0;
        }
        break;
      //init values sequence
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

      //Calibrate value from Mic
      case CALIBRATE_MIC:
        {
          if (DEBUG) Serial.println("CALIBRATING");


          sequenceState = LISTEN_HEADER;

        }
        break;
      /*
        obtain the max  time tick from the RF. then only call RF methods only at the las 10ms,
        to save energy and process time.
      */
      case CALIBRATE_TIME:
        {

        }
        break;

      case LISTEN_HEADER:
        {
          if (DEBUG) Serial.print("LISTEN HEADER ");

          //micHit = isHit();

          if (isRecordHeader) {

            headerSequence[bitIndex] = micHit;
            bitIndex++;

            isHead = true;

            //Analyze to pass to the next stage
            if (bitIndex == numHeaderBits) {
              if (DEBUG)Serial.print("AH ");
              Serial.println("");
              for (itri = 0; itri < numHeaderBits; itri++) {
                Serial.print(headerSequence[itri]);
              }
              Serial.println("");


              double headers[SEQITER];
              for (itri = 0; itri < SEQITER; itri++) headers[itri] = 0.0;

              for (itri = 0; itri < SEQITER; itri++) {
                for (itrj = 0; itrj < 3; itrj++) {
                  headers[itri] +=  headerSequence[itri + itrj * SEQITER];
                }
              }

              int countCheck = 0;
              for (itri = 0; itri < SEQITER; itri++) {
                if (correctHeader[itri] != (headers[itri] / SEQITER) > 0.5 ) {
                  countCheck++;
                }
              }

              //OK to have one error, its the header..
              isHead = (countCheck <= 1) ? true : false;

              //RESET HEADER
              //if didn't found the header then reset the values of headerSequence
              if (isHead == false) {
                for (itri = 0; itri < numHeaderBits; itri++) {
                  headerSequence[itri] = false;
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
                for (itri = 0; itri < numHeaderBits; itri++)
                  Serial.print(headerSequence[itri]);
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

          //micHit = isHit();

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
            for (itri = 0; itri < SEQBITS; itri++) {
              float average = 0.0;
              for (itrj = 0; itrj < SEQITER; itrj ++) {
                average += recording[itrj][itri];
                if (DEBUG) {
                  Serial.print(" ");
                  Serial.print(average);
                  Serial.print(" ");
                }
              }
              playSequence[itri] = average >= 0.5 * SEQITER;
              if (DEBUG) Serial.println(playSequence[itri]);
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
            for (itri = 0; itri < SEQBITS; itri++) {
              Serial.print(playSequence[itri]);
            }
            Serial.println();

            //
            // prints the recordings
            //
            if (DEBUG) {

              Serial.println("L: Done Analyze");

              for (itri = 0; itri < SEQITER; itri++) {
                Serial.print("L: ");
                Serial.print(itri);
                Serial.print('=');
                for (itrj = 0; itrj < SEQBITS; itrj++) {
                  Serial.print(recording[itri][itrj]);
                }
                Serial.println();
              }

              // check sequence if its correct

              bool flag = true;
              for (itri = 0; itri < SEQBITS; itri++) {
                if (debugSequence[itri] != playSequence[itri]) {
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
  timeKeeperNRF.updateTimes();

  if (timeKeeper.checkHit()) {
    analogWrite(SOL_PIN, solenoid_pwm);
    digitalWrite(LED_PIN, HIGH);
  } else {
    analogWrite(SOL_PIN, 0);
    digitalWrite(LED_PIN, 0);
  }

  //digitalWrite(LED_PIN, tick);


  //use LED for feedback clock
  //bool tick =  timeKeeper.checkTick();
  //digitalWrite(LED_PIN, tick);

  //  use mic has feedback device.
  //digitalWrite(LED_PIN, micHit);


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

void resetSequence() {
  //reset sequences
  for (itri = 0; itri < SEQBITS; itri++) {
    playSequence[itri] = false;
    for (itrj = 0; itrj < SEQITER; itrj++) {
      recording[itrj][itri] = false;
    }
  }

  //reset header
  for (itri = 0; itri < numHeaderBits; itri++) {
    headerSequence[itri] = false;
  }

}
