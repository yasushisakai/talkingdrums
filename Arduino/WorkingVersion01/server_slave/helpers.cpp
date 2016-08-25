#include "helpers.h"
#include "define.h"

boolean initNRF(RH_NRF24 &_nrf) {
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

boolean checkServer(RH_NRF24 & _nrf, uint8_t & value) {
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  

  if (_nrf.recv(buf, &len)) {
    return true;
   // if(bitRead(value, 0) == 1){
   //   return true;
   // }
    
  }

  return false;
}

//process the incoming byte
void turnOnLEDs(uint8_t & inByte)
{
  digitalWrite(LED_PIN_00, (bitRead(inByte, 7)  ==  1) ? HIGH : LOW);
  digitalWrite(LED_PIN_01, (bitRead(inByte, 6)  ==  1) ? HIGH : LOW);
  digitalWrite(LED_PIN_02, (bitRead(inByte, 5)  ==  1) ? HIGH : LOW);
  analogWrite(LED_PIN_03, (bitRead(inByte, 4)  ==  1) ? 255 : 0);
  analogWrite(LED_PIN_04, (bitRead(inByte, 3)  ==  1) ? 255 : 0);
  analogWrite(LED_PIN_05, (bitRead(inByte, 2)  ==  1) ? 255 : 0);
  analogWrite(LED_PIN_06, (bitRead(inByte, 1)  ==  1) ? 255 : 0);
  analogWrite(LED_PIN_07, (bitRead(inByte, 0)  ==  1) ? 255 : 0);
}

boolean timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}
