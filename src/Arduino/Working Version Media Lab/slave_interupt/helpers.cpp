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
uint8_t  checkServer(RH_NRF24 &_nrf, uint8_t & clockIn, uint8_t & moduleId, uint8_t & modeId, int8_t & changeValue) {
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof buf;

  uint8_t id_val     = B00000000;
  uint8_t change_val = B00000000;
  uint8_t tick_tock  = TOCK;

  if (_nrf.recv(buf, &len)) {
    
    //first byte is the TICK
    if ( bitRead(buf[0], 0) == TICK) {
      tick_tock   = TICK;
      clockIn     = TICK;
    }

    moduleId     = buf[0];
    changeValues = buf[1];

    //check moduel ID
    if (moduleId == clkModuleId) {
      Serial.print("same ID");
      Serial.print(moduleId);
      Serial.print(clkModuleId);
    }

    changeValues =  bitRead(buf[0], 1) &   bitRead(buf[0], 2);

    println(

    //Serial.println(value, BIN);
  }

  return tick_tock;

}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}
