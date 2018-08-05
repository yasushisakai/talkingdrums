// Arrays to save our results in
//unsigned long start_times[100];
//unsigned long stop_times[100];
int values[200];


// Define various ADC prescaler
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

int counter = 0;
// Setup the serial port and pin 2

unsigned long  duration = 100L;
unsigned long  pTime;

//normal 112us
//2Hz  -> 20000 values
void setup() {
  Serial.begin(9600);

  // set up the ADC
  //ADCSRA = 0xe5;
  //ADCSRA &= ~PS_128;  // remove bits set by Arduino library
  // you can choose a prescaler from above.
  // PS_16, PS_32, PS_64 or PS_128
 //ADCSRA |= PS_64;    // set our own prescaler to 64
  //ADMUX = 0x40; // use adc0
  //DIDR0 = 0x01;

  pTime = millis();
  counter = 0;

  Serial.println("starting");
}


void loop() {

  unsigned long cTime = millis();
  // capture the values to memory

  
  if (counter < 200) {
    //start_times[counter] = micros();
    values[counter] = analogRead(A0);
    //stop_times[counter] = micros();

  }
      counter++;
  unsigned long devTime =  cTime - pTime;

  if (devTime > duration) {

    pTime = cTime;

    // print out the results
    Serial.println("\n\n--- Results ---");
    Serial.println(devTime);
    Serial.println(counter);
    /*
    for (unsigned int i = 0; i < counter; i++) {
      Serial.print(values[i]);
      Serial.print(" elapse = ");
     // Serial.print(stop_times[i] - start_times[i]);
      Serial.print(" us\n");
    }
    */
    counter = 0;

  }
}
