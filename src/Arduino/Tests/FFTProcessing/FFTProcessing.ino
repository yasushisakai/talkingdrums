/***************************************************
  This example shows an implementation of the Fast Fourier Transform (FFT), 
  sampled by the analog PIN 0.
  
  Pines used:
    * Audio:
               - Audio in:  A0
    * Display:
               - CS:        5
               - RST        4
               - SCLK       27
               - SDAT       26
  
  
  Author: Tapia Favio <technicc@gmail.com>
  It depends on the following libraries:
   - ST7628: https://github.com/kr4fty/ST7628
   - arduinoFFT: https://github.com/kosme/arduinoFFT
   - Adafruit-GFX: https://github.com/adafruit/Adafruit-GFX-Library
   
   Video:
   
   * https://www.youtube.com/watch?v=IAP3RYwZHVA
   * https://www.youtube.com/watch?v=7KjgSyl-0yg
  
****************************************************/
#include <arduinoFFT.h> // include the library

arduinoFFT FFT = arduinoFFT(); // Create FFT object
/* 
These values can be changed in order to evaluate the functions 
*/
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2

double vReal[samples];
double vImag[samples];

uint8_t amp_previa[samples];
uint8_t samplingCont=0;
uint8_t peak[samples];
uint8_t diffPeak;
#define fallPeakSpeed 1

#define TFT_CS     5
#define TFT_RST    4
#define TFT_SCLK   27
#define TFT_MOSI   26

void setup() {
  Serial.begin(9600);
  // Configure the port interrupt 0, analog, to take audio samples for later analysis by FFT.
  cli();        //disable interrupts  
  //set up continuous sampling of analog pin 0
  ADCSRA = 0;  //clear ADCSRA and ADCSRB registers
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0);  //set reference voltage
  ADMUX |= (1 << ADLAR);  //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE);  //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN);  //enable ADC
  ADCSRA |= (1 << ADSC);  //start ADC measurements
  
  sei();//enable interrupts
}

void loop() {
  while(1) { 
    
    while(ADCSRA & _BV(ADIE));  // Wait for the buffer to fill
    
    FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
    FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
    
    //send values to Processing
    for(int i = 0; i < samples; i++){
      Serial.write((int)vReal[i]);
      delay(5);
    }

    
    samplingCont=0;
    ADCSRA |= _BV(ADIE);      // Interrupt on;
    delay(5000);
  }
  
}


ISR(ADC_vect) {//when new ADC value ready
  if(samplingCont<samples){
    vReal[samplingCont] = (uint8_t)ADCH;
    vImag[samplingCont] = 0.0;
    samplingCont++;
  }
  else
    ADCSRA &= ~_BV(ADIE);  // Buffer full, interrupt off
}

