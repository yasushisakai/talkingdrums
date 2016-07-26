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

int STATE = LOW;
int LED_STATE = LOW;

//play sequence
int numSequence = 8;
int playIndex   = 0;

unsigned int sequencePlay[]      = {1, 0, 0, 1, 1, 0, 0, 1};
unsigned int sequenceRecord[]    = {0, 0, 0, 0, 0, 0, 0, 0};



//Times
unsigned long clockInterval = 0;
unsigned long prevTime = 0;
unsigned long diffTime = 5;

//Hit solenoid
unsigned long prevHitTime = 0;
unsigned long intervalHitTime = 60;// 50ms
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
float voltsThreshold   = 0.7;


int analyzeSignal(int & micValue, int & sMax, int & sMin, float threshold, boolean lock);
boolean timer(unsigned long & currTime, unsigned long & previousTime, unsigned long interval);

void setup()
{

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

  //Serial.println(microphoneValue);


  if (microphoneValue < 1024)  //This is the max of the 10-bit ADC so this loop will include all readings
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

  if (timer(currentTime, prevLEDTime, 500)) {

    float peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    double volts = (peakToPeak * voltageIn) / 1024;  // convert to volts

    Serial.println(volts);

    signalMax   = 0;
    signalMin   = 1024;

    prevLEDTime = currentTime;
  }

  /*

    uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    // Should be a reply message for us now
    if (nrf24.recv(buf, &len))
    {
    int value = buf[0] - 48;

    if (value == 1) {

      prevLEDTime = currentTime;

      // read only once
      //lock incomming
      if (lockInMsg) {

        diffTime = currentTime - prevTime;
        if (diffTime > 10) {
          prevHitTime    = currentTime;
          prevSignalTime = currentTime;
          prevTime       = currentTime;
          lockInMsg = false;
          activateSignalAnalysis = true;
        }
      }


    } else {
    }
    }




    // Serial.print(" M: ");
    // Serial.print(microphoneValue);

    //int micValue = analyzeSignal(microphoneValue, signalMax, signalMin,  voltsThreshold, lockInMsg);
    //Serial.print(" V: ");
    // Serial.print(micValue);

    if ( !lockInMsg) {

    float peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    double volts = (peakToPeak * voltageIn) / 1024;  // convert to volts

    //play sequence
    unsigned int  value = sequencePlay[playIndex];

    Serial.print("New msg: ");
    Serial.print(diffTime);
    Serial.print(" ");
    Serial.print(value);
    Serial.print(" max ");
    Serial.print(signalMax);
    Serial.print(" min ");
    Serial.print(signalMin);
    Serial.print(" mic: ");

    Serial.println(volts);

    signalMax   = 0;
    signalMin   = 1024;

    //clock
    if (value == 1) {
      STATE = HIGH;
      LED_STATE = HIGH;
    }

    counterLock = 0;
    playIndex++;

    if (playIndex >= numSequence) {
      playIndex = 0;
    }

    lockInMsg = true;
    }

    digitalWrite(LED_PIN, LED_STATE);
    digitalWrite(SOLENOID_PIN, LED_STATE);

    if (LED_STATE == HIGH) {
    if (timer(currentTime, prevLEDTime, intervalHitTime)) {
      LED_STATE = LOW;
    }
    }

    if (STATE == HIGH) {
    if (timer(currentTime, prevHitTime, intervalHitTime)) {
      STATE = LOW;
    }
    }


  */
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

    /*


        Serial.print(micValue);
        Serial.print(",c ");
        Serial.print(currTimer);
        Serial.print(",p ");
        Serial.print(previousTime);
        Serial.print(", ");


        Serial.print(prevVoltage);



      Serial.print("Max ");
      Serial.print(signalMax);
      Serial.print(" Min ");
      Serial.print(signalMin);

    */
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


