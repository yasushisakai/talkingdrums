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
int ledPin = 3;
int solenoidPin = 4;
float voltage = 3.3;
float voltsThres = 0.5;


void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
}

void loop()
{
  unsigned long start = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for 250 miliseconds
  while (millis() - start < sampleWindow)
  {
    knock = analogRead(0);
    if (knock < 1024)  //This is the max of the 10-bit ADC so this loop will include all readings
    {
      if (knock > signalMax)
      {
        signalMax = knock;  // save just the max levels
      }
      else if (knock < signalMin)
      {
        signalMin = knock;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  double volts = (peakToPeak * voltage) / 1024;  // convert to volts

  
  Serial.println(volts);
  if (volts >= voltsThres)
  {
    //turn on LED
    digitalWrite(ledPin, HIGH);
    digitalWrite(solenoidPin, HIGH);
    delay(500);
    Serial.println("Knock Knock");
  }
  else
  {
    //turn LED off
    digitalWrite(ledPin, LOW);
    digitalWrite(solenoidPin, LOW);
  }
}
