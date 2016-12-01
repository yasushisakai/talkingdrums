//pases, filters
//https://www.norwegiancreations.com/2016/03/arduino-tutorial-simple-high-pass-band-pass-and-band-stop-filtering/

//http://www.paulodowd.com/2013/06/arduino-38khz-bandpass-software-digital.html

int sensorPin = A0;
int sensorValue = 0;
//0.05
//0.4
float EMA_a_low = 0.2;     //initialization of EMA alpha (cutoff-frequency)
float EMA_a_high = 0.6;

int EMA_S_low = 0;          //initialization of EMA S
int EMA_S_high = 0;

int highpass = 0;
int bandpass = 0;
int bandstop = 0;


void setup() {
  // start serial port at 9600 bps:
  Serial.begin(115200);

  //Calibrate the EMA
  EMA_S_low = analogRead(sensorPin);        //set EMA S for t=1
  EMA_S_high = analogRead(sensorPin);
}

void loop() {
  // read analog input, divide by 4 to make the range 0-255:
  sensorValue = analogRead(sensorPin);

  EMA_S_low = (EMA_a_low * sensorValue) + ((1 - EMA_a_low) * EMA_S_low);    //run the EMA
  EMA_S_high = (EMA_a_high * sensorValue) + ((1 - EMA_a_high) * EMA_S_high);

  highpass = sensorValue - EMA_S_low;     //find the high-pass as before (for comparison)
  bandpass = EMA_S_high - EMA_S_low;

  Serial.println(bandpass);
  // pause for 10 milliseconds:
  delay(1);
}



float filterDerivate(float inVal, float lastInput) {
  long thisUS = micros();
  float dt = 1e-6 * float(thisUS - LastUS); // cast to float here, for math
  LastUS = thisUS;                          // update this now

  float derivative = (inVal - lastInput) / dt;

  lastInput = inVal;
  return derivative;
}

