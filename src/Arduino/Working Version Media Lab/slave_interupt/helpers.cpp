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
uint8_t  checkServer(RH_NRF24 &_nrf, uint8_t & clockIn, uint8_t & moduleId, uint8_t & modeId, uint8_t & changeValue, uint8_t & activateNRFChange, uint8_t & inMSG, int mode) {

  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof buf;

  uint8_t id_val     = B00000000;
  uint8_t enableMode = 0;
  uint8_t tick_tock  = TOCK;
  uint8_t allMode    = 0;

  uint8_t enableChange = 1;



  if (_nrf.recv(buf, &len)) {

    //first byte is the TICK
    if ( bitRead(buf[0], 0) == TICK) {
      tick_tock   = TICK;
      clockIn     = TICK;
      //Serial.println("TICK");

      if (mode == 2) {
       // _nrf.send(inMSG, sizeof(inMSG));
      //  _nrf.waitPacketSent();
      }
    }

    //module ID
    enableMode = bitRead(buf[1], 0);
    allMode    = bitRead(buf[1], 1);

    if (len >= 3) {
      if ( enableMode ==  1) {

        //second byte is the module
        modeId =  buf[1] >> 2;

        //store the third byte
        changeValue  = buf[2];

        enableChange = 1;
        moduleId     = buf[0] >> 1;

        /*
          if (DEBUG) {
          Serial.print(changeValue);
          Serial.print(" ");
          Serial.print(buf[1]);
          Serial.print(" ");
          Serial.println(modeId);
          }
        */
      }

      //check moduel ID
      if ( moduleId == TD_ID && allMode == 0 && enableMode == 1 ) {

        /*
          if (DEBUG) {
          Serial.print("Change one same ID: ");
          Serial.print(moduleId);
          Serial.print(" ");
          Serial.println(TD_ID);
          }
        */


        activateNRFChange = 1;
      }

      if (allMode == 1 && enableMode == 1) {
        //if (DEBUG) Serial.println("All");
        activateNRFChange = 1;
      }

    }

  }

  return tick_tock;

}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}
