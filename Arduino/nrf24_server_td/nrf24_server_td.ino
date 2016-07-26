#include <SPI.h>
#include <RH_NRF24.h>

// Singleton instance of the radio driver
RH_NRF24 nrf24;
// RH_NRF24 nrf24(8, 7); // use this to be electrically compatible with Mirf
// RH_NRF24 nrf24(8, 10);// For Leonardo, need explicit SS pin
// RH_NRF24 nrf24(8, 7); // For RFM73 on Anarduino Mini

int LED_PIN = 3;

unsigned long pTime     = 0;
unsigned long duration = 500;

int LED_STATE = HIGH;
boolean sendActivation = false;
int counter = 0;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  //Serial.begin(9600);
  // while (!Serial)
  ; // wait for serial port to connect. Needed for Leonardo only
  if (!nrf24.init()) {
    //Serial.println("init failed");
  }
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(1)) {
    //Serial.println("setChannel failed");
  }
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    //Serial.println("setRF failed");
  }

  digitalWrite(LED_PIN, LOW);
  pTime = millis();
}

void loop()
{
  unsigned long cTime = millis();

  if (updateTimer(cTime, pTime, duration)) {
    pTime = cTime;
    sendActivation = true;
    LED_STATE = HIGH;
  }

  if (sendActivation) {
    uint8_t data[] = "1";
    nrf24.send(data, sizeof(data));
    nrf24.waitPacketSent();

    if (counter >= 3) {
      sendActivation = false;
      counter  = 0;
      LED_STATE = LOW;
    }

    counter++;
  }

  digitalWrite(LED_PIN, LED_STATE);
  digitalWrite(4, LED_STATE);

}


boolean updateTimer(unsigned long timer, unsigned long prevTime, unsigned long interval) {
  if (timer - prevTime >= interval) {
    return true;
  }
  return false;
}

