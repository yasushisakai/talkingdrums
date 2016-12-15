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

bool DEBUG = false;
bool isSend = true;
unsigned long previousTime;
int counter;
bool sendActivation;

bool LED_STATE; // is always same with isSend?

// 1
uint8_t currentMode = 1;
String  readStr = "";

//MODES
uint8_t data [] = {B10000001,  // 1  Clock
                   B10000010, // 2  Default sequence
                   B10000011, // 3  Test MIC
                   B00000100, // 4  Stop
                   B10000101, // 5  PWM Value  255
                   B10000110, // 6  PWM Value  200
                   B10000111, // 7  PWM Value  150
                   B10001000, // 8  PWM Value  100
                   B10001001, // 9  PWM Value  50
                   B10001010  // 10  PWM Value  0
                  };

void setup() {

  if (DEBUG) {
    Serial.begin(9600);
  }

  for (int i = 0; i < 9; i++) {
    Serial.println(data[i]);
  }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);

  initNRF(nrf24, DEBUG);

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

  if (Serial.available() > 0) {
    byte inByte = Serial.read();
    readStr += inByte;
  }

  if (readStr.length() == 2) {
    currentMode = readStr.toInt();
    readStr = "";
  }

  if (isSend) {

    if (currentMode != 0) {
      nrf24.send((uint8_t*)data[currentMode], sizeof(data));
      nrf24.waitPacketSent();
    }

    if (currentMode == 1) {
      nrf24.send((uint8_t*)data[currentMode], sizeof(data));
      nrf24.waitPacketSent();
      currentMode = 0;
    }



    if (counter >= COUNTER_LIMIT) {
      counter = 0;
      isSend = false;
      LED_STATE = LOW;
    }
    counter ++;
  }


  digitalWrite(LED_PIN, LED_STATE);

}
