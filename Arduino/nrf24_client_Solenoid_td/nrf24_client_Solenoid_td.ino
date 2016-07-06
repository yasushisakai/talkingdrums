#include <SPI.h>
#include <RH_NRF24.h>

// Singleton instance of the radio driver
RH_NRF24 nrf24;
// RH_NRF24 nrf24(8, 7); // use this to be electrically compatible with Mirf
// RH_NRF24 nrf24(8, 10);// For Leonardo, need explicit SS pin
// RH_NRF24 nrf24(8, 7); // For RFM73 on Anarduino Mini

int STATE = LOW;

void setup()
{

  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);

  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  delay(200);


  digitalWrite(3, LOW);
  digitalWrite(4, LOW);

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
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    delay(500);

    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    delay(200);

    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    delay(500);

    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    delay(200);

  }


  digitalWrite(3, LOW);
  digitalWrite(4, LOW);

}


void loop()
{

  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  //if (nrf24.waitAvailableTimeout(50))
  // {
  // Should be a reply message for us now
  if (nrf24.recv(buf, &len))
  {
    int value = buf[0] - 48;

    if (value == 0) {
      STATE = LOW;
    } else {
      STATE = HIGH;

    }

  }
  / / }

  digitalWrite(3, STATE);
  digitalWrite(4, STATE);

}

