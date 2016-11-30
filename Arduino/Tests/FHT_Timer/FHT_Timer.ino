/*
  fht_adc.pde
  guest openmusiclabs.com 9.5.12
  example sketch for testing the fht library.
  it takes in data on ADC0 (Analog0) and processes them
  with the fht. the data is sent out over the serial
  port at 115.2kb.  there is a pure data patch for
  visualizing the data.
*/
#include <SPI.h>
#include <RH_NRF24.h>


#define LOG_OUT 1 // use the log output function
#define FHT_N 64 // set to 256 point fht

#include <FHT.h> // include the library

RH_NRF24 nrf24;

int numberCycles = 100;
int counterCycles = 0;

uint8_t fht_init[FHT_N];
float fhtSt = 0;

//timer
unsigned long pTime = 0L;
unsigned long duration = 100L;

void setup() {
  Serial.begin(115200); // use the serial port

  //TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
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
  while (1) { // reduces jitter

    unsigned long start = millis();

    // Now wait for a reply
    uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);


    cli();  // UDRE interrupt slows this way down on arduino1.0

    //clean buffer data
    if (counterCycles < numberCycles) {
      fillInitBuffer();
    } else {
      fillBuffer();
    }

    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht
    sei();




    //Serial.println(start - prev);
    // prev =

    if (nrf24.recv(buf, &len)) {
      Serial.println(start - pTime);
      pTime = start;

    }

    //Serial.write(255); // send a start byte
    //Serial.write(fht_log_out, FHT_N / 2); // send out the data
    // Serial.write(fht_init, FHT_N / 2); // send out the data

    // Serial.println(counterCycles);
  }
}

bool timer(unsigned long const & currentTime, unsigned long const & previousTime, unsigned long const &interval) {
  return (currentTime - previousTime) >= interval;
}


void fillBuffer() {
  for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int

    if (k > fhtSt) {
      fht_input[i] = k; // put real data into bins
    }

  }
}



void fillInitBuffer() {
  for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fht_init[i] = k; // put real data into bins
  }

  // sum avg
  int sum = 0;
  for (int i = 0 ; i < FHT_N ; i++) {
    sum += fht_init[i];
  }
  //SUM
  float mean  = sum / (FHT_N);
  float squaredSum = 0;

  for (int i = 0 ; i < FHT_N ; i++) {
    squaredSum += pow( fht_init[i] - mean, 2);
  }

  fhtSt = sqrt(squaredSum / FHT_N);
}

