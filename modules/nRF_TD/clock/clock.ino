#include <RH_NRF24.h>
#include "helpers.h"
#include "defines.h"

/* This is the
      ___           ___       ___           ___           ___
     /\  \         /\__\     /\  \         /\  \         /\__\
    /::\  \       /:/  /    /::\  \       /::\  \       /:/  /
   /:/\:\  \     /:/  /    /:/\:\  \     /:/\:\  \     /:/__/
  /:/  \:\  \   /:/  /    /:/  \:\  \   /:/  \:\  \   /::\__\____
  /:/__/ \:\__\ /:/__/    /:/__/ \:\__\ /:/__/ \:\__\ /:/\:::::\__\
  \:\  \  \/__/ \:\  \    \:\  \ /:/  / \:\  \  \/__/ \/_|:|~~|~
  \:\  \        \:\  \    \:\  /:/  /   \:\  \          |:|  |
   \:\  \        \:\  \    \:\/:/  /     \:\  \         |:|  |
    \:\__\        \:\__\    \::/  /       \:\__\        |:|  |
     \/__/         \/__/     \/__/         \/__/         \|__|
*/


RH_NRF24 nrf24;

bool DEBUG = true;
bool isSend = true;
unsigned long previousTime;
int counter;
bool sendActivation;

bool LED_STATE;

// incoming messeage from Serial
String strBuf = "";

// the dataOut is the data broadcasted to the slave devices
uint8_t dataOut [3];

void setup() {

  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);

  initNRF(nrf24, DEBUG);

  initByteCommand();

  previousTime = millis();
  counter = 0;
  isSend = LED_STATE = false;

}

void loop() {

  unsigned long currentTime = millis();

  if (timer(currentTime, previousTime, DURATION)) {
    previousTime = currentTime;
    LED_STATE = HIGH;
    isSend = true;

  }

  // this part changes the MODE
  if (Serial.available() > 0) {
    strBuf = Serial.readString();
    msgToByteCommand(strBuf);
    Serial.println(dataOut[0], BIN);
    Serial.println(dataOut[1], BIN);
    Serial.println(dataOut[2], BIN);
  }


  if (isSend) {

    nrf24.send((uint8_t*)dataOut, sizeof(dataOut));
    nrf24.waitPacketSent();

    if (counter >= COUNTER_LIMIT) {
      counter = 0;
      isSend = false;
      LED_STATE = LOW;
    }
    counter ++;

    //sent it only once
    if (dataOut[0] != B00000001) {
      Serial.println("back to default seq.");
      Serial.println("");
      initByteCommand();
    }
  }


  digitalWrite(LED_PIN, LED_STATE);

}

void initByteCommand () {
  dataOut[0] = B00000001;
  dataOut[1] = B00000000;
  dataOut[2] = B00000000;
}
