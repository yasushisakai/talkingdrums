#define LED_PIN        3
#define SOLENOID_PIN   4
#define MICROPHONE_PIN A0

unsigned int  numValueSequence =  8;
unsigned int  numValueInit     =  4;

unsigned long intervalRecord   = 500;
unsigned long intervalPlay     = 500;
unsigned long intervalHit      = 20;
unsigned long intervalInit     = 400;
unsigned long intervalTmp      = 1000;

unsigned int sequenceMaxTime = numValueSequence * intervalPlay;
unsigned int intMaxTime = numValueInit * intervalInit;

//sequence values
unsigned int sequencePlay[]      = {1, 0, 1, 1, 1, 0, 1, 0};
unsigned int sequenceRecord[]    = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned int sequenceInit[]      = {1, 1, 1, 0};
unsigned int sequenceInitCheck[] = { -1, -1, -1, -1};

unsigned int indexRecord  = 0;
unsigned int indexPlay    = 0;
unsigned int indexInit    = 0;
unsigned int indexInitCheck = 0;

unsigned long prevTimeRecord = 0;
unsigned long prevTimePlay   = 0;
unsigned long prevTimeInit   = 0;
unsigned long prevTimeTmp    = 0;

///Signal processing
unsigned int  signalMax   = 0;
unsigned int  signalMin   = 1024;
float         signalSum   = 0;
unsigned int  signalCount = 0;

//Signal values
float voltageIn        = 5.0;
float voltsThreshold   = 1.8;
float prevVoltage      = 0.0;
float voltage          = 0;

//Events
unsigned int events   = 0;  //0 hear start 2 play start
boolean   startEvents = false;

//hit solenoid actions and events
unsigned long prevTimeHit = 0;
boolean actionHit = false;
boolean startHearing = false;

void setup()
{
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);

  //digitalWrite(LED_PIN, HIGH);
  //digitalWrite(SOLENOID_PIN, HIGH);
  //delay(500);

  //digitalWrite(LED_PIN, LOW);
  //digitalWrite(SOLENOID_PIN, LOW);

  prevTimeRecord = millis();
  prevTimeInit   =  millis();

  Serial.println("starting module");
}

void loop()
{
  unsigned long currentTime     = millis(); // Start time

  if (startEvents) {
    prevTimeRecord = currentTime;
    prevTimeInit   = currentTime;
    prevTimeTmp    = currentTime;
    startEvents    = false;
  }

  unsigned int  peakToPeak      = 0;   // peak-to-peak level
  unsigned int  microphoneValue = analogRead(MICROPHONE_PIN);

  switch (events) {

    case 0: // hear start protocol
      {
        unsigned int  signalValue  = analyzeSignal(microphoneValue, currentTime, prevTimeInit, intervalInit);


        if (!startHearing) {
          if (timer(currentTime, prevTimeTmp, intervalTmp)) {
            startHearing = true;
            Serial.println("start");
          }
        }

        if (startHearing) {
          if (signalValue != 2) {
            Serial.println(signalValue);

            if (indexInit < numValueInit) {

              //always the first one is 1
              if (signalValue == 1 || indexInit >= 1) {
                sequenceInitCheck[indexInit] = signalValue;
                indexInit++;
                prevTimeInit =  currentTime;

                Serial.print("read first hit: ");
                Serial.print(signalValue);
                Serial.print(indexInit);
                Serial.print(" ");
                Serial.println(prevTimeInit);
              }


            } else {
              //when done reading the sequence print out the values
              Serial.print("done reading init: ");
              Serial.println(indexInit);

              for (int i = 0; i < numValueInit; i++) {
                Serial.print(sequenceInitCheck[i]);
                Serial.print(", ");
              }
              Serial.println("");


              //check if the sequence heard is the same as the input
              boolean checkInit = true;
              for (int i = 0; i < numValueInit; i++) {
                if (sequenceInitCheck[i] != sequenceInit[i]) {
                  checkInit = false;
                  break;
                }
              }
              if (checkInit == true) {
                Serial.print("Same init Sequence!!");

              }
            }
          }
        }
      }
      break;
    case 1: // hear sequence
      {
        unsigned int  signalValue     = analyzeSignal(microphoneValue, currentTime, prevTimeRecord, intervalRecord);

        if (signalValue != -1) {
          if (indexRecord < numValueSequence) {
            sequenceRecord[indexRecord] = signalValue;
            indexRecord++;
          } else {
            //when done reading the sequence print out the values
            Serial.print("done reading: ");
            Serial.println(indexRecord);

            for (int i = 0; i < numValueSequence; i++) {
              Serial.print(sequenceRecord[i]);
              Serial.print(", ");
            }
            Serial.println("");
          }
        }
      }
      break;
    case 2: // play start sequence
      if (indexInit  < numValueInit) {

        unsigned int  initValue = sequenceInit[indexInit];

        if (timer(currentTime, prevTimeInit, intervalInit)) {
          indexInit++;

          prevTimeInit  = currentTime;
          prevTimeHit   =  currentTime;
          actionHit = true;

          if (indexInit  >= numValueInit) {
            break;
          }
        }

        if (initValue == 1) {
          int state = hitSolenoid(currentTime, prevTimeHit, intervalHit);

          if (state == 1) {
            digitalWrite(SOLENOID_PIN, HIGH);
          } else {
            digitalWrite(SOLENOID_PIN, LOW);
          }

        } else {
          digitalWrite(SOLENOID_PIN, LOW);
        }
        Serial.print(initValue);
        Serial.print(" ");
        Serial.println(prevTimeInit);
      }
      break;
    case 3: // play sequence

      //play the sequence in a given timestep
      if (indexPlay < numValueSequence) {
        unsigned int  playValue = sequencePlay[indexPlay];

        if (timer(currentTime, prevTimePlay, intervalPlay)) {
          indexPlay++;

          prevTimeHit  = currentTime;
          prevTimePlay = currentTime;
          actionHit = true;
        }

        if ( playValue == 1) {

          int state = hitSolenoid(currentTime, prevTimeHit, intervalHit);

          if (state == 1) {
            digitalWrite(SOLENOID_PIN, HIGH);
          } else {
            digitalWrite(SOLENOID_PIN, LOW);
          }

        } else {
          digitalWrite(SOLENOID_PIN, LOW);
        }

      }
      break;

    case 4:
      {
        unsigned int  signalValue   = analyzeSignal(microphoneValue, currentTime, prevTimeInit, intervalInit);

        if (signalValue != 2) {
          Serial.println(signalValue);
        }
      }
      break;
  }
}


//hit solenoid action
int hitSolenoid( unsigned long currTimer, unsigned long prevTime, unsigned long interval) {
  int state = -1;
  if (actionHit == true) {
    state = 1;
    if (timer(currTimer, prevTime, interval)) {
      actionHit = false;
    }
  } else {
    state = 0;
  }

  return state;
}

unsigned int analyzeSignal(unsigned int & micValue, unsigned long currTimer, unsigned long & previousTime, unsigned long interval) {

  unsigned int outputValue = 2;

  signalSum += micValue;
  signalCount++;

  if (signalCount  >= 5 ) {
    float signalAvrg = signalSum / signalCount;

    if (signalAvrg < 1024)  //This is the max of the 10-bit ADC so this loop will include all readings
    {
      if (signalAvrg > signalMax)
      {
        signalMax = signalAvrg;  // save just the max levels
      }
      else if (signalSum < signalMin)
      {
        signalMin = signalAvrg;  // save just the min levels
      }
    }

    if (signalMin == 1024) {
      signalMin = 0;
    }

    signalSum = 0;
    signalCount = 0;
  }



  // collect data for 40 miliseconds
  if (timer(currTimer, previousTime, interval))
  {
    prevVoltage = voltage;
    float peak = abs(signalMax - signalMin);  // max - min = peak-peak amplitude
    voltage = (peak * voltageIn) / 1024.0;  // convert to volts

    /*
        Serial.print(signalMax);
        Serial.print(" ");
        Serial.print(signalMin);
        Serial.print(" ");
        Serial.print(micValue);
        Serial.print(" ");
        Serial.print(currTimer);
        Serial.print(" ");
        Serial.print(previousTime);
        Serial.print(", ");
    */

    Serial.print(prevVoltage);
    Serial.print(" ");
    Serial.println(voltage);

    //reset values;
    signalMax  = 0;
    signalMin  = 0;
    previousTime = currTimer;

    if (voltage -  prevVoltage > 0.5) {
      digitalWrite(LED_PIN, HIGH);
      outputValue = 1;
    } else {
      digitalWrite(LED_PIN, LOW);
      outputValue = 0;
    }

  }


  return outputValue;
}

boolean timer(unsigned long currTime, unsigned long previousTime, unsigned long interval) {
  if (currTime - previousTime >= interval) {
    return true;
  }
  return false;
}

