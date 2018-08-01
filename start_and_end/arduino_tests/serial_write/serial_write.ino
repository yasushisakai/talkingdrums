
const long baud = 115200;
uint8_t zero = B00000000;
uint8_t mix = B10101010;
uint8_t one = B11111111;

int cnt;

uint8_t value;

void setup() {
  Serial.begin(baud);
  Serial.println();
  value = zero;
  cnt = 0;
}

void loop() {
  switch(cnt % 3){
      case 0:
      value = zero;
      break;
      case 1:
      value = one;
      break;
      case 2:
      value = mix;
      break;
 }
  Serial.write(value);

  delay(18000);

}
