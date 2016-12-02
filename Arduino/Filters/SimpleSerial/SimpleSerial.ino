void setup() {
  // start serial port at 115200 bps:
  Serial.begin(115200); //230400
}

void loop() {

  Serial.println(analogRead(A0));
  //delay(2);
}



