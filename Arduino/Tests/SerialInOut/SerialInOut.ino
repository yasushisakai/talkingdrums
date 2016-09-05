
bool isRead;
byte data;

#define LED_PIN 13

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  isRead = true;

  Serial.print('*');

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_PIN,LOW);

  if (isRead) {
    // read
    byte data_in[1];
    if (Serial.readBytes(data_in, 1) ){
      data = data_in[0];
      isRead = false;
      Serial.flush();

      digitalWrite(LED_PIN,HIGH);
      delay(50);
      digitalWrite(LED_PIN,LOW);
      delay(50);   
      
    }
  } else {
    Serial.print("got data: ");
    Serial.print(data);
    Serial.print("\n");
    isRead = true;
  }

}
