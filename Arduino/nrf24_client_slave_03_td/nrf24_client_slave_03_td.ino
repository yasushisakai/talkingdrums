#include <SPI.h>
#include <RH_NRF24.h>

// Singleton instance of the radio driver
RH_NRF24 nrf24;
// RH_NRF24 nrf24(8, 7); // use this to be electrically compatible with Mirf
// RH_NRF24 nrf24(8, 10);// For Leonardo, need explicit SS pin
// RH_NRF24 nrf24(8, 7); // For RFM73 on Anarduino Mini


#define LED_PIN 3
#define SOLENOID_PIN 4
#define MIC_PIN A0

int LED_STATE = LOW;
int SOLENOID_STATE = LOW;

//play sequence
int numSequence = 8;
int playIndex   = 0;
int playRecordCount = 0;

//Sequence to play once is finish recording the sequence
int sequencePlay[]      =  { -1, -1, -1, -1, -1, -1, -1, -1};

//Sequence which data values from the micrphone are stored.
int sequenceRecord[3][8]    = {
  { -1, -1, -1, -1, -1, -1, -1, -1},
  { -1, -1, -1, -1, -1, -1, -1, -1},
  { -1, -1, -1, -1, -1, -1, -1, -1}
};

//Hard coded sequence for error/debug correction
int sequenceCheck[]     =  {1,   0,  0,  1,  1,  0,  0,  1};

//Sequence
int sequenceState = 0;
boolean recordSequence = false;
boolean playSequence  = false;
int recordIndex = 0;
int sequenceIndex = 0;
int timerMsgCounter = 0;
int stepWait = 2;

//Times
unsigned long clockInterval = 0;
unsigned long prevTime = 0;
unsigned long diffTime = 5;

//Hit solenoid
unsigned long prevHitTime = 0;
unsigned long intervalHitTime = 100;// 50ms
unsigned long prevLEDTime = 0;

//lock
boolean lockInMsg = true;
int counterLock  = 0;

///Signal processing
int   signalMax   = 0;
int   signalMin   = 1024;
float          signalSum   = 0;
unsigned int   signalCount = 0;
unsigned long  prevSignalTime = 0;
boolean        activateSignalAnalysis = false;

//VT, 1 -> 1.1
//VT, 2 -> 1.2
//VT, 3 -> 2.0

//Signal values
float voltageIn        = 3.3;
float voltsThreshold   = 0.9;

int analyzeSignal(int & micValue, int & sMax, int & sMin, float threshold, boolean lock);
boolean timer(unsigned long & currTime, unsigned long & previousTime, unsigned long interval);

void setup()
{
  pinMode(MIC_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(SOLENOID_PIN, HIGH);
  delay(200);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOLENOID_PIN, LOW);

  Serial.begin(9600);

  int failed = 0;


  if (!nrf24.init()) {
    failed = 1;
  }

  if (!nrf24.setChannel(1)) {
    failed = 1;
  }

  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    failed = 1;
  }

  //SEND LED NOTIFICATION THAT RF FAILED
  if (failed == 1) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(SOLENOID_PIN, HIGH);
    delay(500);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(SOLENOID_PIN, LOW);
    delay(200);

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(SOLENOID_PIN, HIGH);
    delay(500);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(SOLENOID_PIN, LOW);
    delay(200);

    Serial.println("error starting RF");

  } else {
    Serial.println("started RF");

  }

  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOLENOID_PIN, LOW);

}


void loop()
{

  unsigned long currentTime     = millis(); // Start time

  if (sequenceState == 0 || sequenceState == 1) {
    int  microphoneValue = analogRead(MIC_PIN);

    if (microphoneValue < 1023 && microphoneValue > 50)  //This is the max of the 10-bit ADC so this loop will include all readings
    {
      if (microphoneValue > signalMax)
      {
        signalMax = microphoneValue;  // save just the max levels
      }
      else if (microphoneValue < signalMin)
      {
        signalMin = microphoneValue;  // save just the min levels
      }
    }
  }

  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  // Should be a reply message for us now
  if (nrf24.recv(buf, &len))
  {
    int value = buf[0] - 48;

    if (value == 1) {

      diffTime = currentTime - prevTime;
      if (diffTime > 60) {
        lockInMsg = false;
        prevTime = currentTime;
      }

    } else {

    }
  }

  if ( !lockInMsg) {

    switch (sequenceState) {

      //Record sequence from the microphone
      case 0:
        {
          //wait 3 clock steps to recorded sequence from the microphone

          timerMsgCounter++;
          if (timerMsgCounter == stepWait) {
            timerMsgCounter = 0;
            sequenceState = 1;
            Serial.println("0: waiting 3 clock");
          }

        }
        break;
      case 1:
        {

          int valueHit = 0;

          float peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
          double volts = (peakToPeak * voltageIn) / 1024.0;  // convert to volts

          if (volts > voltsThreshold) {
            Serial.println("Knock");


            //Save values
            // New initial header


            if (!recordSequence) {
              Serial.println("1: Starting recording sequence");
              recordSequence = true;
            }

            valueHit = 1;
          }

          if (recordSequence) {
            Serial.print(valueHit);
            sequenceRecord[sequenceIndex][recordIndex] = valueHit;

            recordIndex++;
            if (recordIndex >= numSequence) {

              Serial.println("2: Finish recording");

              //reset values
              recordIndex = 0;
              recordSequence = false;
              sequenceState = 2;

              //print the recording and store the values
              Serial.println();
              for (int i = 0; i < numSequence; i++) {
                Serial.print(sequenceRecord[sequenceIndex][i] );
              }
              Serial.println();

            }

            Serial.print(diffTime);
            Serial.print(" ");
            Serial.println(volts);

            signalMax   = 0;
            signalMin   = 1024;

          }
          break;
        case 2:
          {

            //wait 3 clock steps to play the recorded sequence
            timerMsgCounter++;
            if (timerMsgCounter == stepWait) {
              timerMsgCounter = 0;
              sequenceState = 1;
              sequenceIndex++;
              Serial.print("Next Sequence Index: ");
              Serial.println(sequenceIndex);
            }

            if (sequenceIndex == 3) {

              Serial.println("Avg values");
              //calculate average between each recorder sequence
              for (int i = 0; i < numSequence; i++) {
                float avgValue  = 0.0;
                for (int j = 0; j < 3; i++) {
                  avgValue += sequenceRecord[j][i];
                }
                avgValue /= 3.0;
                if (avgValue >= 0.6) {
                  sequencePlay[i] = 1;
                } else {
                  sequencePlay[i] = 0;
                }
                Serial.print(sequencePlay[i]);
              }

              Serial.println();

              //check if its the same sequence
              boolean equal = true;
              for (int i = 0; i < numSequence; i++) {
                if ( sequenceCheck[i] != sequencePlay[i] ) {
                  equal = false;
                  break;
                }
              }
              if (equal) {
                Serial.println("same");
              } else {
                Serial.println("not same");
              }
            }

            sequenceState = 3;
            sequenceIndex = 0;

          }




        }
      case 3:
        {
          //play the recorded sequence three times

          int value  = sequencePlay[playIndex];

          // Serial.print(playIndex);
          //Serial.print(": ");
          Serial.print(value);
          Serial.print(" ");

          if (value == 1) {
            SOLENOID_STATE = HIGH;
          } else {
            SOLENOID_STATE = LOW;
          }

          prevHitTime = currentTime;
          playIndex++;
          if (playIndex >= numSequence) {
            playIndex = 0;
            sequenceState  = 4;
            Serial.println();
            Serial.println("3: Finish Playing recording");
          }

        }

        break;
      case 4:
        {

          timerMsgCounter++;
          if (timerMsgCounter == stepWait) {
            timerMsgCounter = 0;

            //play again the recorded sequence
            sequenceState = 3;
            playRecordCount++;
            Serial.println("4: Go to Play recording");

            if (playRecordCount == 3) {
              playRecordCount = 0;
              sequenceState = 1;
              Serial.println("5: Go to Listen");
            }
          }

        }
        break;
      case 5:
        {

        }
        break;
    }

    prevLEDTime    = currentTime;
    lockInMsg      = true;
    LED_STATE      = HIGH;
  }

  digitalWrite(LED_PIN, LED_STATE);
  digitalWrite(SOLENOID_PIN, SOLENOID_STATE);

  if (LED_STATE == HIGH) {
    if (timer(currentTime, prevLEDTime, intervalHitTime)) {
      LED_STATE = LOW;
    }
  }

  if (SOLENOID_STATE == HIGH) {
    if (timer(currentTime, prevHitTime, intervalHitTime)) {
      SOLENOID_STATE = LOW;
    }
  }

}

boolean timer(unsigned long & currTime, unsigned long & previousTime, unsigned long interval) {
  if (currTime - previousTime >= interval) {
    return true;
  }
  return false;
}



int analyzeSignal(int & micValue, int & sMax, int & sMin, float  threshold, boolean lock) {

  int outputValue = 2;

  //signalSum += micValue;
  // signalCount++;

  //if (signalCount  >= 5 ) {
  ///  float signalAvrg = signalSum / signalCount;

  if (micValue < 1024)  //This is the max of the 10-bit ADC so this loop will include all readings
  {
    if (micValue > sMax)
    {
      sMax = micValue;  // save just the max levels
    }
    else if (micValue < sMin)
    {
      sMin = micValue;  // save just the min levels
    }
  }

  // collect data for interval time
  if (!lock) {
    float peak = sMax - sMin;  // max - min = peak-peak amplitude
    double voltage = (peak * 3.3) / 1024;  // convert to volts

    Serial.print("Max");
    Serial.print(sMax);
    Serial.print(", Min ");
    Serial.print(sMin);

    Serial.print(" v: ");
    Serial.print(voltage);
    Serial.print(" micV: ");
    Serial.print(micValue);
    Serial.print(" ");

    //reset values;
    sMax  = 0;
    sMin  = 1024;


    if (voltage > threshold) {
      outputValue = 1;
    } else {
      outputValue = 0;
    }

  }


  return outputValue;
}


