const int sampleWindow = 50; // Sample window width in mS (250 mS = 4Hz)
unsigned int knock;
int ledPin = 3; //
int solenoidPin = 5;

unsigned long pTime = 0L;
unsigned long duration = 100L;

unsigned long  pTimeHit = 0L;
unsigned long  durationHit = 20L;

boolean solenoidHit = false;

int squenceSolenoid[] = {1, 1, 0, 0, 1, 1, 0, 0};
int squenceIndex = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);

  digitalWrite(solenoidPin, HIGH);
  digitalWrite(solenoidPin, HIGH);
  delay(200);
  digitalWrite(solenoidPin, LOW);
  digitalWrite(solenoidPin, LOW);

  pTime = millis();
  pTimeHit = millis();
}

void loop()
{
  unsigned long start = millis(); // Start of sample window

  if (timer(start, pTime, duration)) {

    pTime = millis();
    pTimeHit = pTime;

    int hit = squenceSolenoid[squenceIndex];

    if (hit == 1) {
      solenoidHit = true;
    } else {
      solenoidHit = false;
    }
    squenceIndex++;

    if (squenceIndex >= 8)
      squenceIndex = 0;

    //Serial.println("hit");
  }

  if (solenoidHit) {

    if (timer(start, pTimeHit, durationHit)) {
      solenoidHit = false;
      // Serial.println("return");
    }

  }

  if (solenoidHit) {
    digitalWrite(solenoidPin, HIGH);
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(solenoidPin, LOW);
    digitalWrite(ledPin, LOW);
  }


}

bool timer(unsigned long  currentTime, unsigned long  previousTime, unsigned long  interval) {
  return (currentTime - previousTime) >= interval;
}


