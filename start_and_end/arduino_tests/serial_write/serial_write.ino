const long baud = 115200;
uint8_t value;
uint8_t message [3];

void setup() {
  value = 0;
  Serial.begin(baud);
  delay(100);
}

void loop() {
  
  message[0] = 2; // B00000010
  message[1] = value;
  message[2] = 10; // B00001010

  Serial.write(message, 3);
  delay(150000); 

  value ++;
}
