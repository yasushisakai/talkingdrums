#include <RH_NRF24.h>

/*
  Helper Functions

  handy project agnostic functions
*/

bool initNRF(RH_NRF24 &_nrf, bool printDebug);
uint8_t checkServer(RH_NRF24 &_nrf, uint8_t & clockIn);
bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval);

// memo:

//
// LOW = false = 0
// HIGH = true = 1
//


// ISP (looking from the top)
//
// 1:MISO   2:VCC
// 3:SCK    4:MOSI
// 5:RESET  6:GND
//

// importatnt pins for ATMEGA
//
// RESET = 1
// RX = 2
// TX = 3
// VCC = 7,20(AVCC),21(AREF)
// GND = 8,22
// XTAL1 = 9
// XTAL2 = 10
// SCK = 19
// MISO = 18
// MOSI = 17
//


