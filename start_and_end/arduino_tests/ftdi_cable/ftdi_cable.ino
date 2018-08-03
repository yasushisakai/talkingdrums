byte value;
const int baud=115200;

void setup() {
  Serial.begin(baud);
}

void loop() {
  if(Serial.available() > 0) {
     value = (byte) Serial.read();
     Serial.flush();
     Serial.write(value);
  }
}
