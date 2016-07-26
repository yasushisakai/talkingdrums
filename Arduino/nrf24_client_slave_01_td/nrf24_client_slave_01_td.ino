#include <SPI.h>
#include <RH_NRF24.h>

// Singleton instance of the radio driver
RH_NRF24 nrf24;
// RH_NRF24 nrf24(8, 7); // use this to be electrically compatible with Mirf
// RH_NRF24 nrf24(8, 10);// For Leonardo, need explicit SS pin
// RH_NRF24 nrf24(8, 7); // For RFM73 on Anarduino Mini


#define LED_PIN 3
#define SOLENOID_PIN 4
#define MIC_PIN A5

int STATE = LOW;

//play sequence
int numSequence = 12;
int playIndex   = 0;

unsigned int sequencePlay[]      =  {1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1};
unsigned int sequenceRecord[]    =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//Sequence
boolean recordSequence = false;
boolean playSequence  = false;
int recordIndex = 0;
int knockCounter = 0;

//Times
unsigned long clockInterval = 0;
unsigned long prevTime = 0;
unsigned long diffTime = 5;

//Hit solenoid
unsigned long prevHitTime = 0;
unsigned long intervalHitTime = 100;// 50ms
unsigned long prevLEDTime = 50;

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

//Signal values
float voltageIn        = 3.3;
float voltsThreshold   = 1.2;


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
      }

    } else {

    }

  }

  if ( !lockInMsg) {

    int valueHit = 0;

    float peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    double volts = (peakToPeak * voltageIn) / 1024.0;  // convert to volts

    if (volts > voltsThreshold) {
      Serial.println("Knock");


      //Save values
      // New initial header

      knockCounter++;

      if (knockCounter > 1) {
        if (!recordSequence) {
          recordSequence = true;
          Serial.println("Starting recording sequence");
        }
      }
      valueHit = 1;
    }

    if (playSequence) {


    }


    if (recordSequence) {
      Serial.print(valueHit);
      sequenceRecord[recordIndex] = valueHit;

      recordIndex++;
      if (recordIndex >= numSequence) {
        recordIndex = 0;

        recordSequence = false;


        //print the recording
        Serial.println();
        for (int i = 0; i < numSequence; i++) {
          Serial.print(sequenceRecord[i] );
        }
        Serial.println();

        boolean equal = true;
        for (int i = 0; i < numSequence; i++) {
          if ( sequencePlay[i] != sequenceRecord[i] ) {
            equal = false;
            break;
          }
        }
        if (equal) {
          Serial.println("same");


          //if done recording play Sequence
          playSequence = true;

        } else {
          Serial.println("not same");
        }
      }
    }


    /*

          Serial.print(signalMax);
          Serial.print(" ");
          Serial.print(signalMin);
          Serial.print(" ");
          Serial.print("reset ");
          Serial.print(diffTime);
          Serial.print(" ");

    */
    Serial.print(" ");
    Serial.println(volts);

    signalMax   = 0;
    signalMin   = 1024;

    //play sequence
    //unsigned int  value = sequencePlay[playIndex];

    //playIndex++;

    //if (playIndex >= numSequence) {
    //  playIndex = 0;
    // }

    prevTime       = currentTime;
    STATE          = HIGH;
    lockInMsg      = true;
  }

  digitalWrite(LED_PIN, STATE);
  digitalWrite(SOLENOID_PIN, STATE);

  if (STATE == HIGH) {
    if (timer(currentTime, prevTime, intervalHitTime)) {
      STATE = LOW;
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


