#include <SPI.h>
#include <RH_NRF24.h>

// Singleton instance of the radio driver
RH_NRF24 nrf24;
// RH_NRF24 nrf24(8, 7); // use this to be electrically compatible with Mirf
// RH_NRF24 nrf24(8, 10);// For Leonardo, need explicit SS pin
// RH_NRF24 nrf24(8, 7); // For RFM73 on Anarduino Mini


#define LED_PIN 3
#define SOLENOID_PIN 4

int STATE_SOL = LOW;

//play sequence
int numSequence =  12;
int playIndex   = 0;

unsigned int sequencePlay[]      = {1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1};
unsigned int sequenceRecord[]    = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int timerCounter = 0;
boolean delayTimer = false;


//Times
unsigned long clockInterval = 0;
unsigned long prevTime = 0;
unsigned long diffTime = 5;

//Hit solenoid
unsigned long prevHitTime = 0;
unsigned long intervalHitTime = 100;// 50ms

//lock
boolean lockInMsg = true;
int counterLock  = 0;

void setup()
{

  pinMode(LED_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  //digitalWrite(SOLENOID_PIN, HIGH);
  delay(3000);


  digitalWrite(LED_PIN, LOW);
  //digitalWrite(SOLENOID_PIN, LOW);

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
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(SOLENOID_PIN, HIGH);
    delay(500);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(SOLENOID_PIN, LOW);
    delay(200);

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(SOLENOID_PIN, HIGH);
    delay(500);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(SOLENOID_PIN, LOW);
    delay(200);

    Serial.println("error starting RF");

  } else {
    Serial.println("started RF");

  }

  digitalWrite(LED_PIN, LOW);
  // digitalWrite(SOLENOID_PIN, LOW);

  delay(2000);
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
        if (diffTime > 10) {
          lockInMsg = false;
        }
      }


    } else {
    }
  }

  if (delayTimer) {
    /*
        timerCounter++;
        if (timerCounter >= 6) {
          timerCounter = 0;
          delayTimer = false;
        }
    */
  }

  if ( !lockInMsg) {

    //play sequence
    if (!delayTimer) {
      unsigned int  value = sequencePlay[playIndex];

      Serial.print("New msg: ");
      Serial.print(diffTime);
      Serial.print(" ");
      Serial.println(value);

      if (value == 1) {
        STATE_SOL = HIGH;
      }

      prevTime = currentTime;
      counterLock = 0;
      lockInMsg = true;
      playIndex++;

      if (playIndex >= numSequence) {
        playIndex = 0;
        delayTimer = true;
        //sequence
      }
    }

  }




  digitalWrite(LED_PIN, STATE_SOL);
  digitalWrite(SOLENOID_PIN, STATE_SOL);

  if (STATE_SOL == HIGH) {
    if (timer(currentTime, prevTime, intervalHitTime)) {
      STATE_SOL = LOW;
      // Serial.println("OFF");
    }
  }

}

boolean timer(unsigned long currTime, unsigned long previousTime, unsigned long interval) {
  if (currTime - previousTime >= interval) {
    return true;
  }
  return false;
}




