
const long baud = 115200;
const byte bitsCount = 8;

const char header[ ] = "L: r=";
char message[14];
byte value = 255;

void setup() {
  Serial.begin(baud);
  Serial.println();
}

void loop() {
  delay(1000);
  
  // char str[bitsCount+1];

  char str[9];
  
  for(int i=0;i<8;i++){
     str[7-i] = bitRead(value, i) + 48;
  }
  // Serial.println(value);
  str[8] = '\0';
  

  sprintf(message, "%s%s", header, str);
  Serial.println(message);

  value--;

}
