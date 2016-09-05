#include <RH_NRF24.h>


boolean initNRF(RH_NRF24 &_nrf);
boolean checkServer(RH_NRF24 &_nrf, char &value);
boolean timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval);


//
// memo:
// LOW = false = 0
// HIGH = true = 1
//
