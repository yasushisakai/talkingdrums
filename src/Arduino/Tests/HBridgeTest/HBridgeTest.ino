
int i = 0;

int limit =  5 * 60 * 5 ;

int PWMPIN   = 5;
int DGIOPIN  = 4;

int ANSLEEP = 6;

int numHits = 8 * 3;
int numHitsCount = 0;
int numWaitCout  = 0;


unsigned long prevTimeCC = 0L;
unsigned long prevTimeHIT = 0L;

int eventPULLHIT = 1 ;
bool enablePULLHIT = false;

void setup() {
  // put your setup code here, to run once:
  //pinMode(9, OUTPUT);
  pinMode(DGIOPIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  unsigned long cTime = millis();

  //HIT
  if (eventPULLHIT == 1) {
    if ( abs(cTime - prevTimeHIT) >= 50) {
      analogWrite(PWMPIN, 255);
      digitalWrite(DGIOPIN, HIGH);
      analogWrite(ANSLEEP, 255);
      prevTimeHIT = cTime;
      eventPULLHIT = 2;
    }
  }

 //BACK
  if (eventPULLHIT == 2) {
    if ( abs(cTime - prevTimeHIT) >= 50) {
      analogWrite(PWMPIN, 0);
      digitalWrite(DGIOPIN, HIGH);
      analogWrite(ANSLEEP, 0);
      prevTimeHIT = cTime;
      eventPULLHIT = 3;

    }
  }

  //WAIT
  if ( eventPULLHIT == 3) {
    if ( abs(cTime - prevTimeHIT) >= 100) {
      prevTimeHIT  = cTime;
      analogWrite(PWMPIN, 0);
      digitalWrite(DGIOPIN, LOW);
      analogWrite(ANSLEEP, 0);
      eventPULLHIT = 1;
      numHitsCount++;
      if (numHitsCount >=  numHits) {
        eventPULLHIT = 4;
        numWaitCout = 0;
        numHitsCount =0;

      }
    }

  }

  //WAIT
  if (eventPULLHIT == 4) {
    analogWrite(ANSLEEP, 0);
    if ( abs(cTime - prevTimeHIT) >= 100) {

      prevTimeHIT = cTime;
      numWaitCout++;
      if ( numWaitCout > numHits) {
        numWaitCout = 0;
        eventPULLHIT = 1;
      }
    }


  }

  /*
    //    short circuit
    if ( abs(cTime - prevTimeCC)  >= 1000 * 5 ) {
      analogWrite(PWMPIN, 0);
      digitalWrite(DGIOPIN, LOW);
      prevTimeCC = cTime;
    }
  */

}
