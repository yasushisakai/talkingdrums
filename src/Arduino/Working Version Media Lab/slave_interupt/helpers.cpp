#include "helpers.h"


bool initNRF(RH_NRF24 &_nrf, bool printDebug = true) {
  if (!_nrf.init()) {
    Serial.println("E: (RF)failed to init");
    return false;
  }

  if (!_nrf.setChannel(1)) {
    Serial.println("E: (RF)failed to set CHNL");
    return false;
  }

  if (!_nrf.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    Serial.println("E: (RF)failed to set RF");
  }

  if (printDebug) Serial.println("L: nrf connected");
  delay(10);
  return true;
}

//returns TICK
uint8_t  checkServer(RH_NRF24 &_nrf, uint8_t & clockIn, uint8_t & moduleId, int8_t & changeValues) {
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof buf;

  uint8_t id_val     = B00000000;
  uint8_t change_val = B00000000;
  uint8_t value      = TOCK;

  if (_nrf.recv(buf, &len)) {
    clockIn = TICK;
    value   = TICK;
    
    moduleId     = buf[0];
    changeValues = buf[1];
    
    //Serial.println(value, BIN);
  }

  return value;

}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}
