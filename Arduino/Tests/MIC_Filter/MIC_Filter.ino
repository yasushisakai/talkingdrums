/*
  fht_adc.pde
  guest openmusiclabs.com 9.5.12
  example sketch for testing the fht library.
  it takes in data on ADC0 (Analog0) and processes them
  with the fht. the data is sent out over the serial
  port at 115.2kb.  there is a pure data patch for
  visualizing the data.
*/

//16mhz * 115.2

#define LOG_OUT 1 // use the log output function
#define FHT_N 256 // set to 256 point fht

#include <FHT.h> // include the library

int numberCycles = 100;
int counterCycles = 0;
uint8_t fht_init[FHT_N];

int fht_st[FHT_N];
float fhtSt = 0;

void setup() {
  Serial.begin(115200); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

}

void loop() {
  while (1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0

    //clean buffer data
    if (counterCycles < numberCycles) {
      fillInitBuffer();
      counterCycles++;
    } else {
      fillBuffer();
    }

    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht



    sei();
    Serial.write(255); // send a start byte
    Serial.write(fht_log_out, FHT_N / 2); // send out the data
    // Serial.write(fht_init, FHT_N / 2); // send out the data

    // Serial.println(counterCycles);
  }
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
    fht_input[i] = k;
    

    // }// else {
    //  fht_init[i] = 0; //fht_input
    // }

  }
}

void fillInitBuffer() {
  int minValue = 9999;
  int minValueIdx = 0;

  int maxValue = 0;
  int maxValueIdx = 0;

  for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int

    fht_init[i] = k;

    //cut down min and max
    if (minValue < k) {
      minValue = k;
      minValueIdx = i;
    }
    if (maxValue > k) {
      maxValue = k;
      maxValueIdx = i;
    }

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

