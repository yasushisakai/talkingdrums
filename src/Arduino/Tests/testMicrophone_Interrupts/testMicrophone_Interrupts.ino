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
#include <RH_NRF24.h>

RH_NRF24 nrf24;

int ledPin = 3;

unsigned long pTime = 0L;
unsigned long duration = 100L;


void setup()
{
  Serial.begin(9600);

  while (!Serial)
    ; // wait for serial port to connect. Needed for Leonardo only
  if (!nrf24.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("setRF failed");

  pinMode(ledPin, OUTPUT);

  pTime = millis();

  OCR0A = 0xAF;            // use the same timer as the millis() function
  TIMSK0 |= _BV(OCIE0A);
}

void loop()
{
  unsigned long start = millis(); // Start of sample window

  // Now wait for a reply
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);


  if (nrf24.recv(buf, &len)) {
    Serial.println(start - pTime);
    pTime = start;
  }

}

ISR(TIMER0_COMPA_vect) {
  int sensorVal = analogRead(A0);
  if (sensorVal > 600){
    digitalWrite(ledPin, HIGH);
  }else{
    digitalWrite(ledPin, LOW);
  }
}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}


