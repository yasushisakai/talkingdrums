/*
   The Circuit:
   Connect AUD to analog input 0
   Connect GND to GND
   Connect VCC to 3.3V (3.3V yields the best results)

    To adjust when the LED turns on based on audio input:
    Open up the serial com port (Top right hand corner of the Arduino IDE)
    It looks like a magnifying glass. Perform several experiments
    clapping, snapping, blowing, door slamming, knocking etc and see where the
    resting noise level is and where the loud noises are. Adjust the if statement
    according to your findings.

    You can also adjust how long you take samples for by updating the "SampleWindow"

   This code has been adapted from the
   Example Sound Level Sketch for the
   Adafruit Microphone Amplifier

*/

const int sampleWindow = 50; // Sample window width in mS (250 mS = 4Hz)
unsigned int knock;
int ledPin = 3; //
int solenoidPin = 4;
float voltage = 5.0;
float voltsThres = 1.5;

unsigned long pTime = 0L;

unsigned long duration = 100L;

unsigned int signalMax = 0;
unsigned int signalMin = 1024;

double volts;

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);

  pTime = millis();
}

void loop()
{
  unsigned long start = millis(); // Start of sample window

  knock = analogRead(0);
  if (knock < 1024)  //This is the max of the 10-bit ADC so this loop will include all readings
  {
    if (knock > signalMax) signalMax = knock;  // save just the max levels
    if (knock < signalMin) signalMin = knock;  // save just the min levels
  }


  if (timer(start, pTime, duration)) {

    unsigned int  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    volts = (peakToPeak * voltage) / 1024;  // convert to volts

    Serial.print("new ");
    Serial.print(pTime);
    Serial.print(" ");
    Serial.println(volts);

    if (volts >= voltsThres)
    {
      //turn on LED
      digitalWrite(ledPin, HIGH);
    }
    else
    {
      //turn LED off
      digitalWrite(ledPin, LOW);
    }

    pTime = start;
    signalMax = 0;
    signalMin = 1024;

  }

  if (volts >= voltsThres)
  {
    //turn on LED
    digitalWrite(ledPin, HIGH);
    Serial.println("on");
  }
  else
  {
    //turn LED off
    digitalWrite(ledPin, LOW);
  }


}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}


