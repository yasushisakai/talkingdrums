#ifndef BandPassFilter_h
#define BandPassFilter_h

#include "Arduino.h"
#include "define.h"

class BandPassFilter {
  public:

    BandPassFilter(float _f, float _bw, float EMA_LOW, float EMA_HIGH, int samples) {

      EMA_a_low  = EMA_LOW;
      EMA_a_high = EMA_HIGH;
      EMA_S_low  = 0;
      EMA_S_high = 0;

      for ( int i = 0; i < BUFFER_SIZE; i++ ) {
        buf[i] = 0;
        out[i] = 0;
      }
      buffer_index = 0;
      buffer_full = false;

      // Lets sort out the filter variables before we end setup.
      // https://www.sparkfun.com/products/12758
      // Electret Microphone Breakout (100Hz–10kHz)
      // Cut-off frequency.
      //
      // F is fraction of the sample frequency
      // It has to be between 0 and 0.5.  Therefore, the interupt

      // I picked 10 khz sample rate o
      // I will try 2khz = 0.01

      // To use these equations, first select the center
      // frequency, f, and the bandwidth, BW. Both
      // of these are expressed as a fraction of the
      // sampling rate, and therefore in the range of
      // 0 to 0.5

      f = _f; //0 -0.5
      bw = _bw;

      // Band-pass filter.
      float r = 1 - ( 3 * bw );
      float k = 1 - ( 2 * r * cos(2 * PI * f ) ) + ( r * r );
      k = k / (2 - ( 2 * cos( 2 * PI * f ) ) );

      a0 = 1 - k;
      a1 = (2 * ( k - r ) ) * ( cos( 2 * PI * f ) );
      a2 = ( r * r ) - k;
      b1 = 2 * r * cos( 2 * PI * f );
      b2 = 0 - ( r * r );

      signalMax = 0;
      signalMin = 9999;
      itr = 0;
      sum = 0;
      highpass = 0;
      bandpass = 0;

      output = 0;
    }

    void fillWindow(unsigned long cTime, float sensorValue) {
      if ( buffer_index >= BUFFER_SIZE ) {
        buffer_index = 0;
        buffer_full = true;
      } else if ( buffer_full == false ) {
        EMA_S_low  = (EMA_a_low  * sensorValue) + ((1 - EMA_a_low) * EMA_S_low);    //run the EMA
        EMA_S_high = (EMA_a_high * sensorValue) + ((1 - EMA_a_high) * EMA_S_high);

        highpass = sensorValue - EMA_S_low;     //find the high-pass as before (for comparison)
        bandpass = EMA_S_high  - EMA_S_low;

        buf[ buffer_index ] = bandpass;//(float)sensorValue;
        buffer_index++;
      }
    }

    void filterSignal(bool printOut = false) {
      if ( buffer_full == true ) {
        output = doFilter(); //convert to int

        if (output > signalMax) signalMax = output;
        if (output < signalMin) signalMin = output;

        if (printOut)
          Serial.println(output);
        // Serial.println(output);
        //Serial.println(cTime - pBTime); //avg 10 windows in 100
        //pBTime = millis();

        // Reset our buffer and interupt routine
        buffer_index = 0;
        buffer_full = false;
      }
    }



    int doFilter() {
      // Convolute the input buffer with the filter kernel
      // We work from 2 because we read back by 2 elements.
      // out[0] and out[1] are never set, so we clear them.
      out[0] = out[1] = 0;
      sum = 0;
      for ( itr = 2; itr < BUFFER_SIZE; itr++ ) {
        out[itr] =  a0 * buf[itr];
        out[itr] += a1 * buf[itr - 1];
        out[itr] += a2 * buf[itr - 2];
        out[itr] += b1 * out[itr - 1];
        out[itr] += b2 * out[itr - 2];

       if ( out[itr] < 0 ) out[itr] *= -1;
        sum += out[itr];
      }

      sum /= (BUFFER_SIZE - 2);
      //Serial.println(sum);
      return int(sum + 0.55);
    }


    //return if there is hit
    bool isHit() {
      if (signalMax - signalMin > THRESHOLD_PEAK) { //TODO create thresholdPeak dynamically
        // Serial.print(signalMax);
        // Serial.print(" ");
        //  Serial.println(signalMin);
        resetSignalMinMax();
        return true;
      }
      return false;
    }

    void resetSignalMinMax() {

      signalMax = 0;
      signalMin = 1024;
    }

  private:

    float a0, a1, a2, b1, b2; // filter kernel poles
    float f, bw;          // frequency cutoff and bandwidth

    //BUFFER_SIZE = 25 -> 5ms 45 ->10ms
    float buf[BUFFER_SIZE];  // Analog readings & stored here
    float out[BUFFER_SIZE];  // output of filter stored here.
    int buffer_index;         // Interupt increments buffer
    boolean buffer_full;      // Flag for when complete.

    float EMA_a_low;     //initialization of EMA alpha (cutoff-frequency)
    float EMA_a_high;

    float EMA_S_low;          //initialization of EMA S
    float EMA_S_high;

    int signalMax;
    int signalMin;

    int output;

    //temporal values
    uint8_t itr;
    float sum;
    float highpass;
    float bandpass;
    unsigned long pBTime;

};
#endif




