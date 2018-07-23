
const long baud = 115200;
// const uint8_t messageLength = 14;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(baud);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  if (Serial.available() > 0){

    String message;

    message = Serial.readString();

    // Serial.println(Serial.available());
    Serial.println(message);
    Serial.flush();
  } 
}
