#include <SPI.h>
#include <RH_NRF24.h>

// Singleton instance of the radio driver
RH_NRF24 nrf24;
// RH_NRF24 nrf24(8, 7); // use this to be electrically compatible with Mirf
// RH_NRF24 nrf24(8, 10);// For Leonardo, need explicit SS pin
// RH_NRF24 nrf24(8, 7); // For RFM73 on Anarduino Mini

int STATE = LOW;

//play sequence
int numSequence = 8;
int playIndex   = 0;

unsigned int sequencePlay[]      = {1, 0, 1, 1, 1, 0, 1, 0};
unsigned int sequenceRecord[]    = {0, 0, 0, 0, 0, 0, 0, 0};



//Times
unsigned long clockInterval = 0;
unsigned long prevTime = 0;
unsigned long diffTime = 0;

//Hit solenoid
unsigned long prevHitTime = 0;
unsigned long intervalHitTime = 60;// 50ms

//lock
boolean lockInMsg = true;
int counterLock  = 0;

void setup()
{

  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);

  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  delay(200);


  digitalWrite(3, LOW);
  digitalWrite(4, LOW);

  Serial.begin(9600);

  int failed = 0;


  if (!nrf24.init()) {
    failed = 1;
  }

  if (!nrf24.setChannel(1)) {
    failed = 1;
  }

  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    failed = 1;
  }

  //SEND LED NOTIFICATION THAT RF FAILED
  if (failed == 1) {
    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    delay(500);

    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    delay(200);

    digitalWrite(3, HIGH);
    digitalWrite(4, HIGH);
    delay(500);

    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    delay(200);

    Serial.println("error starting RF");

  } else {
    Serial.println("started RF");

  }

  digitalWrite(3, LOW);
  digitalWrite(4, LOW);

}


void loop()
{

  unsigned long currentTime     = millis(); // Start time

  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  // Should be a reply message for us now
  if (nrf24.recv(buf, &len))
  {
    int value = buf[0] - 48;

    if (value == 1) {
      // read only once
      //lock incomming
      if (lockInMsg) {

        diffTime = currentTime - prevTime;
        prevTime = currentTime;
        lockInMsg = false;
      }


    } else {
    }
  }

  if ( !lockInMsg) {

    if (counterLock > 10) {
      Serial.print("New msg: ");
      Serial.println(diffTime);

      //play sequence
      unsigned int  value = sequencePlay[playIndex];
      if (value == 1) {
        STATE = HIGH;
        prevHitTime = currentTime;
      } else {
        STATE = LOW;
      }

      counterLock = 0;
      lockInMsg = true;
      playIndex++;

      if(playIndex >= numSequence){
        playIndex = 0;
      }

    }

    counterLock++;
  }


  digitalWrite(3, STATE);
  digitalWrite(4, STATE);

  if(STATE == HIGH){
    if(timer(currentTime, prevHitTime, intervalHitTime)){
      STATE = LOW;
    }
  }

}

boolean timer(unsigned long currTime, unsigned long previousTime, unsigned long interval) {
  if (currTime - previousTime >= interval) {
    return true;
  }
  return false;
}




