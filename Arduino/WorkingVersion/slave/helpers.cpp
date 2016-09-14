#include "helpers.h"

bool initNRF(RH_NRF24 &_nrf) {
  if (!_nrf.init()) {
    Serial.println("E:(RF)failed to init");
    return false;
  }

  if (!_nrf.setChannel(1)) {
    Serial.println("E:(RF)failed to set CHNL");
    return false;
  }

  if (!_nrf.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    Serial.println("E:(RF)failed to set RF");
  }
  Serial.println("L:nrf connected");
  delay(10);
  return true;
}

bool checkServer(RH_NRF24 &_nrf, uint8_t & value) {
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof buf;

  if (_nrf.recv(buf, &len)) {
    value = B00000001;
    return true;
  } else {
    return false;
  }
}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}
