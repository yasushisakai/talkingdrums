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

import processing.serial.*;
import controlP5.*;

ControlP5 cp5;

float EMA_a_low = 0.1;     //initialization of EMA alpha (cutoff-frequency)
float EMA_a_high = 0.4;

float EMA_S_low = 0;          //initialization of EMA S
float EMA_S_high = 0;

float highpass = 0;
float bandpass = 0;
float bandstop = 0;

//recursive band-pass filter
int BUFFER_SIZE = 25; //25 -> 5ms 45 ->10ms
float buf[] = new float[BUFFER_SIZE]; // Analog readings & stored here
float out[] = new float[BUFFER_SIZE];  // output of filter stored here.

int buffer_index;         // Interupt increments buffer
boolean buffer_full;      // Flag for when complete.

float a0, a1, a2, b1, b2; // filter kernel poles
float f, bw;          // frequency cutoff and bandwidth
float r, k;           // filter coefficients

//calculate window time
int duraction = 100;
int pTime     = 0;

//Serial
int lf = 10;    // Linefeed in ASCII
int value;
String myString = null;
Serial myPort;  // The serial port

ArrayList sensorPoints = new ArrayList();
ArrayList pointsEMS = new ArrayList();

float sensorValue =0;

void setup() {
  size(1000, 1000);

  // List all the available serial ports
  println(Serial.list());
  // I know that the first port in the serial list on my mac
  // is always my  Keyspan adaptor, so I open Serial.list()[0].
  // Open whatever port is the one you're using.
  
  myPort = new Serial(this, Serial.list()[1], 115200);
  myPort.clear();
  // Throw out the first reading, in case we started reading 
  // in the middle of a string from the sender.
  myString = myPort.readStringUntil(lf);
  myString = null;

  for (int i =0; i < 20; i++) {
    sensorPoints.add(0.0);
    pointsEMS.add(0.0);
  }

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
  bw = 0.005;

  // Band-pass filter.
  r = 1 - ( 3 * bw );
  k = 1 - ( 2 * r * cos(2 * PI * f ) ) + ( r * r );
  k = k / (2 - ( 2 * cos( 2 * PI * f ) ) );

  a0 = 1 - k;
  a1 = (2 * ( k - r ) ) * ( cos( 2 * PI * f ) );
  a2 = ( r * r ) - k;
  b1 = 2 * r * cos( 2 * PI * f );
  b2 = 0 - ( r * r );

  //Calibrate the EMA
  EMA_S_low = 0;        //set EMA S for t=1
  EMA_S_high = 0;


  //control
  cp5 = new ControlP5(this);
  cp5.addSlider("EMA_a_low")
    .setPosition(20, 50)
    .setSize(300, 20)
    .setRange(0.0, 0.5)
    .setValue(EMA_a_low);

  cp5.addSlider("EMA_a_high")
    .setPosition(20, 80)
    .setSize(300, 20)
    .setRange(0.0, 1.0)
    .setValue(EMA_a_high);

  cp5.addSlider("f")
    .setPosition(20, 110)
    .setSize(300, 20)
    .setRange(0.0, 0.2)
    .setValue(f);

  cp5.addSlider("bw")
    .setPosition(20, 140)
    .setSize(300, 20)
    .setRange(0.0, 0.1)
    .setValue(bw);

  pTime = millis();
}

void draw() {

  //in values
  //int inValue = getCurrentSensorValue();

  background(0);

  int cTime = millis();

  if ( buffer_full == true ) {
    float output = doFilter();

    pointsEMS.remove(0);
    pointsEMS.add(output);

    //println(cTime - pTime); //avg 10 windows in 100

    // Reset our buffer and interupt routine
    buffer_index = 0;
    buffer_full = false;

   // println(cTime - pTime+" filter done");
    pTime = millis();
  }

  //draw smooth signal
  noFill();
  strokeWeight(2);
  stroke(255);
  beginShape();
  for (int i = 0; i < sensorPoints.size(); i++) {
    float yVal = (Float)sensorPoints.get(i);
    
    vertex(i*30 + 200, 800 - yVal);
  }
  endShape();

  //draw smooth signal
  noFill();
  strokeWeight(2);
  stroke(255, 0, 0);
  beginShape();
  for (int i = 0; i < pointsEMS.size(); i++) {
    float yVal = (Float)pointsEMS.get(i);
    println(yVal);
    vertex(i*30 + 200, 500 - yVal);
  }
  endShape();
}

// This filter looks at the previous elements in the
// input stream and output stream to compound a pre-set
// amplification.  The amplification is set by a0,a1,a2,
// b1,b2. 
float doFilter() {
  float sum = 0;

  // Convolute the input buffer with the filter kernel
  // We work from 2 because we read back by 2 elements.
  // out[0] and out[1] are never set, so we clear them.
  out[0] = out[1] = 0;

  for ( int i = 2; i < BUFFER_SIZE; i++ ) {
    out[i] = a0 * buf[i];
    out[i] += a1 * buf[i - 1];
    out[i] += a2 * buf[i - 2];
    out[i] += b1 * out[i - 1];
    out[i] += b2 * out[i - 2];

    if ( out[i] < 0 ) out[i] *= -1;
    sum += out[i];
  }

  sum /= float(BUFFER_SIZE - 2);

  return sum;
}

void serialEvent (Serial myPort) {
  // get the ASCII string:
  String inString = myPort.readStringUntil('\n');

  if (inString != null) {
    // trim off any whitespace:
    inString = trim(inString);
    // convert to an int and map to the screen height:
    float inByte = float(inString);

    cleanSignal(int(inByte));

    //inByte = map(inByte, 0, 1023, 0, height);
    sensorPoints.remove(0);
    sensorPoints.add(sensorValue);
  }
}


int getCurrentSensorValue() {
  int outValue = 0;
  while (myPort.available() > 0) {
    myString = myPort.readStringUntil(lf);
    // println(myString);
    if (myString != null) {
      myString = trim(myString);
      value = int(myString);
      outValue = value;
      return  outValue;
      // println(outValue);
    }
  }
  return outValue;
}

void cleanSignal(int inValue) {

  // read analog input, divide by 4 to make the range 0-255:
  if ( buffer_index >= BUFFER_SIZE ) {
    buffer_index = 0;
    buffer_full = true;
  } else if ( buffer_full == false ) {

    // println(inValue);

    if (inValue > 0) {

      EMA_S_low  = (EMA_a_low * inValue) + ((1 - EMA_a_low) * EMA_S_low);    //run the EMA
      EMA_S_high = (EMA_a_high * inValue) + ((1 - EMA_a_high) * EMA_S_high);

      highpass = inValue  - EMA_S_low;     //find the high-pass as before (for comparison)
      bandpass = EMA_S_high  - EMA_S_low;

      buf[ buffer_index ] = (float)bandpass;
      buffer_index++;

      // println(buffer_index);

      // sensorValue = 0;
    }
  }
}

void f() {
  // Band-pass filter.
  r = 1 - ( 3 * bw );
  k = 1 - ( 2 * r * cos(2 * PI * f ) ) + ( r * r );
  k = k / (2 - ( 2 * cos( 2 * PI * f ) ) );

  a0 = 1 - k;
  a1 = (2 * ( k - r ) ) * ( cos( 2 * PI * f ) );
  a2 = ( r * r ) - k;
  b1 = 2 * r * cos( 2 * PI * f );
  b2 = 0 - ( r * r );
  
  println(f+" updated");
}

void bw() {
  // Band-pass filter.
  r = 1 - ( 3 * bw );
  k = 1 - ( 2 * r * cos(2 * PI * f ) ) + ( r * r );
  k = k / (2 - ( 2 * cos( 2 * PI * f ) ) );

  a0 = 1 - k;
  a1 = (2 * ( k - r ) ) * ( cos( 2 * PI * f ) );
  a2 = ( r * r ) - k;
  b1 = 2 * r * cos( 2 * PI * f );
  b2 = 0 - ( r * r );

  println(bw+" updated");
}