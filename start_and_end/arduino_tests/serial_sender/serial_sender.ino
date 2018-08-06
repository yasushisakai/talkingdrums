/// DONT Change this values
#define LED_PIN 13

byte byteMSG8[] = {
  B10000000, B00000000, B00001010
};

byte readyValue [] = {B00000000, B00000000, B00001010};

int counter = 0;
bool canRead = false;

void setup() {
  delay(2500);
  Serial.begin(115200);

  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:


  if (counter > 500) {

    Serial.write(readyValue[0]);
    Serial.write(readyValue[1]);
    Serial.write(readyValue[2]);
    //Serial.print("000");
    counter = 0;
    canRead = true;
  }

  if (canRead) {
    if (  Serial.readBytes(byteMSG8, 3) == 3 ) {
      if (byteMSG8[0] == B00000001 && byteMSG8[2] == B00001010) {
        digitalWrite(LED_PIN, HIGH);

      }
    }
  }

  //if (  Serial.readBytes(byteMSG8, 3) == 3 ) {
  //   digitalWrite(LED_PIN, HIGH);
  // }

  counter++;

}
