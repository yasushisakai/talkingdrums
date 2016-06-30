#define LED_PIN        3
#define SOLENOID_PIN   4
#define MICROPHONE_PIN A0

unsigned int  numValueSequence =  8;
unsigned int  numValueInit     =  4;

unsigned long intervalRecord   = 500;
unsigned long intervalPlay     = 500;
unsigned long intervalHit      = 30;
unsigned long intervalInit     = 1000;

unsigned int sequenceMaxTime = numValueSequence * intervalPlay;
unsigned int intMaxTime = numValueInit * intervalInit;

//sequence values
unsigned int sequencePlay[]      = {1, 0, 1, 1, 1, 0, 1, 0};
unsigned int sequenceRecord[]    = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned int sequenceInit[]      = {1, 0, 0, 1};
unsigned int sequenceInitCheck[] = { -1, -1, -1, -1};

unsigned int indexRecord  = 0;
unsigned int indexPlay    = 0;
unsigned int indexInit    = 0;
unsigned int indexInitCheck = 0;

unsigned long prevTimeRecord = 0;
unsigned long prevTimePlay   = 0;
unsigned long prevTimeInit   = 0;

unsigned int signalMax = 0;
unsigned int signalMin = 1024;

//Signal values
float voltage        = 5.0;
float voltsThreshold = 1.5;

//Events
unsigned int events   = 4;  //0 hear start 2 play start
boolean   startEvents = false;

//hit solenoid actions and events
unsigned long prevTimeHit = 0;
boolean actionHit = false;


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
  unsigned int  peakToPeak      = 0;   // peak-to-peak level
  unsigned int  microphoneValue = analogRead(MICROPHONE_PIN);


  if (startEvents) {
    prevTimeRecord = currentTime;
    prevTimeInit   = currentTime;
    startEvents    = false;
  }

  switch (events) {

    case 0: // hear start protocol
      {
        unsigned int  signalValue   = analyzeSignal(microphoneValue, currentTime, prevTimeInit, intervalInit);

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
              }
            }
            if (checkInit == true) {
              Serial.print("Same init Sequence!!");

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

unsigned int analyzeSignal(unsigned int micValue, unsigned long currTimer, unsigned long & previousTime, unsigned long interval) {

  double volts = 0;
  unsigned int outputValue = 2;

  if (micValue < 1024)  //This is the max of the 10-bit ADC so this loop will include all readings
  {
    if (micValue > signalMax)
    {
      signalMax = micValue;  // save just the max levels
    }
    else if (micValue < signalMin)
    {
      signalMin = micValue;  // save just the min levels
    }
  }

  // collect data for 40 miliseconds
  if (timer(currTimer, previousTime, interval))
  {
    unsigned int peak = signalMax - signalMin;  // max - min = peak-peak amplitude
    volts = (peak * voltage) / 1024.0;  // convert to volts

    Serial.print(signalMax);
    Serial.print(" ");
    Serial.print(signalMin);
    Serial.print(" ");
    Serial.print(micValue);
    Serial.print(" ");
    Serial.print(currTimer);
    Serial.print(" ");
    Serial.print(previousTime);
    Serial.print(" ");
    Serial.println(volts);

    //reset values;
    signalMax  = 0;
    signalMin  = 1024;
    previousTime = currTimer;

    if (volts > voltsThreshold) {
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

