#define LED_PIN        3
#define SOLENOID_PIN   4
#define MICROPHONE_PIN A0

unsigned int  numValueSequence =  8;
unsigned long intervalRecord   = 40;
unsigned long intervalPlay     = 50;
unsigned long intervalHalft    = 25;



unsigned int timeMax = numValueSequence * intervalPlay;

//sequence values
unsigned int sequencePlay[]    = {1, 1, 0, 1, 0, 1, 1, 0};
unsigned int sequenceRecord[]  = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned int sequenceStart[]   = {1, 1};

unsigned int indexRecord  = 0;
unsigned int indexPlay = 0;

unsigned long prevTimeRecord = 0;
unsigned long prevTimePlay = 0;

unsigned int signalMax = 0;
unsigned int signalMin = 1024;

//Signal values

float voltage        = 3.3;
float voltsThreshold = 1.5;

unsigned events = 0;



void setup()
{
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);


  digitalWrite(LED_PIN, HIGH);
  digitalWrite(SOLENOID_PIN, HIGH);
  delay(500);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOLENOID_PIN, LOW);


  prevTimeRecord = millis();
}

void loop()
{
  unsigned long currentTime = millis(); // Start time
  unsigned int  peakToPeak = 0;   // peak-to-peak level

  unsigned int  microphoneValue = analogRead(MICROPHONE_PIN);

  unsigned int  signalValue = analyzeSignal(microphoneValue, currentTime, prevTimeRecord, intervalRecord);


  switch (events) {

    case 0: // hear start protocol
      break;
    case 1: // hear sequence
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

      break;
    case 2: // hear final protocol
      break;
    case 3: // play sequence

     //play the sequence in a given timestep
     if(indexPlay <  numValueSequence){
      unsigned int  playValue = sequencePlay[indexPlay];

      if(timer(currentTime, prevTimePlay, intervalPlay)){
        indexPlay++;
      }

     }
    
      break;
  }
}

unsigned int analyzeSignal(unsigned int micValue, unsigned long currtimer, unsigned long previousTime, unsigned long interval) {

  double volts = 0;
  unsigned int outputValue = -1;

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
  if (timer(currtimer, prevTimeRecord, intervalRecord))
  {
    unsigned int peak = signalMax - signalMin;  // max - min = peak-peak amplitude
    volts = (peak * voltage) / 1024.0;  // convert to volts


    //reset values;
    signalMax  = 0;
    signalMin  = 1024;
    prevTimeRecord = currtimer;

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

