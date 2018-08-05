//pases, filters
//https://www.norwegiancreations.com/2016/03/arduino-tutorial-simple-high-pass-band-pass-and-band-stop-filtering/

//http://www.paulodowd.com/2013/06/arduino-38khz-bandpass-software-digital.html

//https://github.com/CoderDojoPotsdam/material/blob/master/tonerkennung/wave_detection/wave_detection.ino

/*




  sensorValue = analogRead(sensorPin);

  EMA_S_low = (EMA_a_low * sensorValue) + ((1 - EMA_a_low) * EMA_S_low);    //run the EMA
  EMA_S_high = (EMA_a_high * sensorValue) + ((1 - EMA_a_high) * EMA_S_high);

  highpass = sensorValue - EMA_S_low;     //find the high-pass as before (for comparison)
  bandpass = EMA_S_high - EMA_S_low;

*/

int sensorPin = A0;
int sensorValue = 0;
//0.05
//0.4
float EMA_a_low = 0.08;     //initialization of EMA alpha (cutoff-frequency)
float EMA_a_high = 0.4;

int EMA_S_low = 0;          //initialization of EMA S
int EMA_S_high = 0;

int highpass = 0;
int bandpass = 0;
int bandstop = 0;

//recursive band-pass filter
const int BUFFER_SIZE = 25; //25 -> 5ms 45 ->10ms
float buf[BUFFER_SIZE];  // Analog readings & stored here
float out[BUFFER_SIZE];  // output of filter stored here.
int buffer_index;         // Interupt increments buffer
boolean buffer_full;      // Flag for when complete.

double a0, a1, a2, b1, b2; // filter kernel poles
double f, bw;          // frequency cutoff and bandwidth
double r, k;           // filter coefficients

//calculate window time
unsigned long duraction = 100L;
unsigned long pTime     = 0L;

void setup() {
  // start serial port at 115200 bps:
  Serial.begin(115200);

  // Clear global variables before the timer
  // is activated.
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
 
  f = 0.01; //0 -0.5
  bw = 0.05;

  // Band-pass filter.
  //
  r = 1 - ( 3 * bw );
  k = 1 - ( 2 * r * cos(2 * PI * f ) ) + ( r * r );
  k = k / (2 - ( 2 * cos( 2 * PI * f ) ) );

  a0 = 1 - k;
  a1 = (2 * ( k - r ) ) * ( cos( 2 * PI * f ) );
  a2 = ( r * r ) - k;
  b1 = 2 * r * cos( 2 * PI * f );
  b2 = 0 - ( r * r );

  //Calibrate the EMA
  EMA_S_low = analogRead(sensorPin);        //set EMA S for t=1
  EMA_S_high = analogRead(sensorPin);

  pTime = millis();
}

void loop() {

  unsigned long cTime = millis();
  
  // read analog input, divide by 4 to make the range 0-255:
  if ( buffer_index >= BUFFER_SIZE ) {
    buffer_index = 0;
    buffer_full = true;
  } else if ( buffer_full == false ) {
    sensorValue = analogRead(sensorPin);

    EMA_S_low = (EMA_a_low * sensorValue) + ((1 - EMA_a_low) * EMA_S_low);    //run the EMA
    EMA_S_high = (EMA_a_high * sensorValue) + ((1 - EMA_a_high) * EMA_S_high);

    highpass = sensorValue - EMA_S_low;     //find the high-pass as before (for comparison)
    bandpass = EMA_S_high - EMA_S_low;

    buf[ buffer_index ] = bandpass;//(float)sensorValue;
    buffer_index++;
  }


  if ( buffer_full == true ) {
    float output = doFilter();

    //Serial.println(bandpass);
    
    Serial.println(int(output));
     //Serial.println(cTime - pTime); //avg 10 windows in 100

    // Reset our buffer and interupt routine
    buffer_index = 0;
    buffer_full = false;
    pTime = millis();
  }


}

// This filter looks at the previous elements in the
// input stream and output stream to compound a pre-set
// amplification.  The amplification is set by a0,a1,a2,
// b1,b2. 
float doFilter() {
  int i;
  float sum;

  // Convolute the input buffer with the filter kernel
  // We work from 2 because we read back by 2 elements.
  // out[0] and out[1] are never set, so we clear them.
  out[0] = out[1] = 0;

  for ( i = 2; i < BUFFER_SIZE; i++ ) {
    out[i] = a0 * buf[i];
    out[i] += a1 * buf[i - 1];
    out[i] += a2 * buf[i - 2];
    out[i] += b1 * out[i - 1];
    out[i] += b2 * out[i - 2];

    if ( out[i] < 0 ) out[i] *= -1;
    sum += out[i];
  }

  sum /= (BUFFER_SIZE - 2);

  return sum;
}

/*
  float filterDerivate(float inVal, float lastInput) {
  long thisUS = micros();
  float dt = 1e-6 * float(thisUS - LastUS); // cast to float here, for math
  LastUS = thisUS;                          // update this now

  float derivative = (inVal - lastInput) / dt;

  lastInput = inVal;
  return derivative;
  }
*/


