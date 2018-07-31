
const long baud = 115200;
uint8_t test = B00000000;

byte value = test;

void setup() {
  Serial.begin(baud);
  Serial.println();
}

void loop() {

  Serial.write(value);

  // if(value == test) {
  //    value = B00000000;
  //  } else {
  //    value = test;
  //  }

  value++;

    if(Serial.available() > 0) {
       String message = Serial.readString();
       // Serial.println(message);
       Serial.flush();
    }
    delay(18000);
}
