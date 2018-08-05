#include "helpers.h"



boolean initNRF(RH_NRF24 &_nrf, bool DEBUG) {
  if (!_nrf.init()) {
    if (DEBUG) Serial.println("E:(RF)failed to init");
    return false;
  }

  if (!_nrf.setChannel(1)) {
    if (DEBUG) Serial.println("E:(RF)failed to set CHNL");
    return false;
  }

  if (!_nrf.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    if (DEBUG)Serial.println("E:(RF)failed to set RF");
  }
  if (DEBUG) Serial.println("L: nrf connected");
  delay(10);
  return true;
}

boolean checkServer(RH_NRF24 &_nrf, char & value) {
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (_nrf.recv(buf, &len)) {
    value = buf[0];
    return true;
  } else {
    return false;
  }
}

boolean timer(unsigned long const & cTime, unsigned long const & pTime, unsigned long const &interval) {
  return (cTime - pTime) >= interval;
}
