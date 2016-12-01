/*
 * Timers
Pins 5 and 6: controlled by timer0
Pins 9 and 10: controlled by timer1
Pins 11 and 3: controlled by timer2

Solenoid timer 0

To calculate the timer frequency (for example 2Hz using timer1) you will need:

CPU frequency 16Mhz for Arduino
maximum timer counter value (256 for 8bit, 65536 for 16bit timer)
Divide CPU frequency through the chosen prescaler (16000000 / 256 = 62500)
Divide result through the desired frequency (62500 / 2Hz = 31250)
Verify the result against the maximum timer counter value (31250 < 65536 success) if fail, choose bigger prescaler.

*/

const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);


#include <SPI.h>
#include <RH_NRF24.h>

RH_NRF24 nrf24;

uint8_t numberCycles = 100;
uint8_t counterCycles = 0;

uint8_t numSamples = 0;

bool collectSamples = true;

//timer
unsigned long pTime = 0L;
unsigned long duration = 100L;

void setup() {
  Serial.begin(115200); // use the serial port

  //TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADCSRA &= ~PS_128;
  ADCSRA |= PS_64;
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  pTime = millis();

  while (!Serial)
    ; // wait for serial port to connect. Needed for Leonardo only
  if (!nrf24.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("setRF failed");

  pinMode(3, OUTPUT);

  Serial.println("nft connected, starting");
}

void loop() {

  unsigned long start = millis();

  // Now wait for a reply
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (nrf24.recv(buf, &len)) {
    Serial.println(start - pTime);
    pTime = start;

  }

  //Serial.write(255); // send a start byte
  //Serial.write(fht_log_out, FHT_N / 2); // send out the data
  // Serial.write(fht_init, FHT_N / 2); // send out the data

  // Serial.println(counterCycles);

}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}


void fillBuffer() {
  for (int i = 0 ; i < numSamples ; i++) { // save 256 samples
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int

    fht_input[i] = k; // put real data into bins


  }
}



void fillInitBuffer() {
  for (int i = 0 ; i < numSamples ; i++) { // save 256 samples
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
  }

}

