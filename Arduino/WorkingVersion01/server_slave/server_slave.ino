#include "define.h"
#include "helpers.h"
#include "TimeKeeper.h"

/*
        ___           ___           ___           ___           ___           ___
       /\  \         /\  \         /\  \         /\__\         /\  \         /\  \
      /::\  \       /::\  \       /::\  \       /:/  /        /::\  \       /::\  \
     /:/\ \  \     /:/\:\  \     /:/\:\  \     /:/  /        /:/\:\  \     /:/\:\  \
    _\:\~\ \  \   /::\~\:\  \   /::\~\:\  \   /:/__/  ___   /::\~\:\  \   /::\~\:\  \
   /\ \:\ \ \__\ /:/\:\ \:\__\ /:/\:\ \:\__\  |:|  | /\__\ /:/\:\ \:\__\ /:/\:\ \:\__\
   \:\ \:\ \/__/ \:\~\:\ \/__/ \/_|::\/:/  /  |:|  |/:/  / \:\~\:\ \/__/ \/_|::\/:/  /
    \:\ \:\__\    \:\ \:\__\      |:|::/  /   |:|__/:/  /   \:\ \:\__\      |:|::/  /
     \:\/:/  /     \:\ \/__/      |:|\/__/     \::::/__/     \:\ \/__/      |:|\/__/
      \::/  /       \:\__\        |:|  |        ~~~~          \:\__\        |:|  |
       \/__/         \/__/         \|__|                       \/__/         \|__|

*/


// Objects
RH_NRF24 nrf24;

//TIMERS
TimeKeeper timeKeeper;

///DEBUG
bool const DEBUG = false;
bool const DEBUG_PORT = false;

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

//Serial Port
bool requestByte = false;
bool readInBytes = false;

//incoming msg, keep it as an array in case we need to
//read values bigger than a byte
byte byteMSG8[] = {
  B00000000
};

void setup() {

  Serial.begin(19200);

  //enable pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);
  pinMode(LED_PIN_00, OUTPUT);
  pinMode(LED_PIN_01, OUTPUT);
  pinMode(LED_PIN_02, OUTPUT);

  //turn on pin test
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(SOL_PIN, HIGH);

  //LEDS
  digitalWrite(LED_PIN_00, HIGH);
  digitalWrite(LED_PIN_01, HIGH);
  digitalWrite(LED_PIN_02, HIGH);

  analogWrite(LED_PIN_03, 255);
  analogWrite(LED_PIN_04, 255);
  analogWrite(LED_PIN_05, 255);
  analogWrite(LED_PIN_06, 255);
  analogWrite(LED_PIN_07, 255);
  delay(1000);

  //turn off
  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOL_PIN, LOW);

  //LEDs
  digitalWrite(LED_PIN_00, LOW);
  digitalWrite(LED_PIN_01, LOW);
  digitalWrite(LED_PIN_02, LOW);
  analogWrite(LED_PIN_03, 0);
  analogWrite(LED_PIN_04, 0);
  analogWrite(LED_PIN_05, 0);
  analogWrite(LED_PIN_06, 0);
  analogWrite(LED_PIN_07, 0);

  //sequence
  sequenceState = WAIT_START;
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

  timeKeeper.setInterval(INTERVAL);

  initNRF(nrf24);

  Serial.flush();
}


void loop() {
  unsigned long currentTime = millis();

  // updates the timeKeeper
  timeKeeper.cycle(currentTime);

  // unlocks if we recieve somthing from the server
  // and timeFrame is more than TIMEFRAMEINTERVAL (60ms)
  uint8_t valueByte = B00000001;
  delay(1);
  if (checkServer(nrf24, valueByte) && timeKeeper.getTimeFrame() > TIMEFRAMEINTERVAL) {
    timeKeeper.tick();
    timeKeeper.flash();
    lock = false;
    // Serial.flush();
  }


  //while not in the look read the serial port for incoming color
  if (!lock) {

    //Serial.flush();

    switch (sequenceState) {
      case WAIT_START:
        {
          TimeKeeper::signalCount++;
          if (!TimeKeeper::wait()) {

            if (DEBUG) {
              Serial.println("Waiting start");
            }
            sequenceState = LISTEN;
          }
        }
        break;

      case LISTEN:  //use it to send activating code
        {

          bitIndex++;
          if (bitIndex >= SEQBITS) {
            sequenceState = ANALYZE;


          }

          if (DEBUG) Serial.print(bitIndex);
          if (DEBUG) Serial.println(" LISTEN");



        }
        break;

      case ANALYZE:  //read the incomming msg from the computer
        {

          TimeKeeper::signalCount++;
          if (TimeKeeper::wait()) sequenceState = LISTEN;

          sequenceIndex++;
          bitIndex = 0;
          if (sequenceIndex < SEQITER) {
            sequenceState = LISTEN;
            if (DEBUG) {
              Serial.print("Analyze: ");
              Serial.println(sequenceIndex);
            }
          } else {
            isRecord = false;
            sequenceIndex = 0;
            sequenceState = WAIT_PLAY;
            if (DEBUG) Serial.println("Done Analyze");
          }



        }
        break;

      case PLAYPULSE:
        {
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

          TimeKeeper::signalCount++;
          if (!TimeKeeper::wait()) {

            if (DEBUG)Serial.println("Waiting play");

            bitIndex = 0;
            sequenceIndex ++;

            if (sequenceIndex > SEQITER) {
              sequenceState = RESET;
              requestByte = true;
              readInBytes = false;

              if (DEBUG) {
                //Serial.print("L: playing=");
              }


            } else {
              sequenceState = PLAYPULSE;
            }

          }

        }
        break;
      case RESET: {
          /*
            returns to playpulse if there is something left to play
            (may not need this phase though)
          */

          TimeKeeper::signalCount++;
          if (!TimeKeeper::wait()) {
            if (DEBUG) Serial.println("RESET");

            //send byte request and read
            if (requestByte) {
              if (DEBUG) Serial.println("request bytes");

              Serial.write('s');
              requestByte = false;
              readInBytes = true;
            }

            //if (Serial.available() > 0) {
            if (readInBytes) {
              if (DEBUG) Serial.println("incoming bytes");

              int val = Serial.readBytes(byteMSG8, 1);

              if (val > 0) {
                if (DEBUG) Serial.println("clean Serial");


                readInBytes = true;
                requestByte = false;

                sequenceIndex = 0;
                bitIndex = 0;
                sequenceState = LISTEN; 

                Serial.flush();

                //clean
                for (int i = 0; i < 10; i++) {
                  char f = Serial.read();
                }


              }
            }



            //reset listen values
            for (char i = 0; i < SEQBITS; i++) {
              playSequence[i] = false;
              for (char j = 0; j < SEQITER; j++) {
                recording[j][i] = false;
              }
            }
          }

        }
        break;
    }

    lock = !lock;
  }

  // outputs
  digitalWrite(LED_PIN, timeKeeper.checkFlash());

  if (timeKeeper.checkHit()) {
    analogWrite(SOL_PIN, solenoid_pwm);
  } else {
    analogWrite(SOL_PIN, 0);
  }

  turnOnLEDs(byteMSG8[0]);



}
