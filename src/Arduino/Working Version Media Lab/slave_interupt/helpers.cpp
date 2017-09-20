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
uint8_t  checkServer(RH_NRF24 &_nrf, uint8_t & clockIn, uint8_t & moduleId, uint8_t & modeId, uint8_t & changeValue, uint8_t & activateNRFChange) {
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof buf;

  uint8_t id_val     = B00000000;
  uint8_t change_val = B00000000;
  uint8_t tick_tock  = TOCK;

  uint8_t enableChange = B00000001;

  if (_nrf.recv(buf, &len)) {

    //first byte is the TICK
    if ( bitRead(buf[0], 0) == TICK) {
      tick_tock   = TICK;
      clockIn     = TICK;
      //Serial.println("TICK");
    }


    //module ID
    moduleId     = buf[0] >> 1;

    if (len >= 2) {
      if ( bitRead(buf[1], 0) ==  1) {
        //second byte is the module
        modeId =  bitRead(buf[1], 1) + bitRead(buf[1], 2);
        //Vlue to change ID (1 and 2)
        changeValue = buf[1] >> 3;

        Serial.print(changeValue);
        Serial.print(" ");
        Serial.println(modeId);
        activateNRFChange = 1;
      }

      //check moduel ID
      if (moduleId == TD_ID) {
        Serial.print("same ID: ");
        Serial.print(moduleId);
        Serial.print(" ");
        Serial.println(TD_ID);
      }
    } 
    
  }

  return tick_tock;

}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}
