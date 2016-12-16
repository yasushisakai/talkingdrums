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
int currentMode = 1;
String  readStr = "";


uint8_t dataOut [] = {B00000001};

//MODES
uint8_t dataMode [] = {B00000001, // 1  Default sequence

                       B00000010, // 2  Default sequence
                       B00000011, // 3  Test MIC
                       B00000100, // 4  Stop
                       B00000101, // 5  PWM Value  255
                       B00000110, // 6  PWM Value  200
                       B00000111, // 7  PWM Value  150
                       B00001000, // 8  PWM Value  100
                       B00001001, // 9  PWM Value  50
                       B00001010  // 10  PWM Value  0
                      };

void setup() {

  Serial.begin(9600);

  /*
    for (int i = 0; i < 9; i++) {
      Serial.println(data[i]);
    }
  */

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
    currentMode = inByte;
    dataOut[0] = dataMode[currentMode];
    Serial.print(currentMode);
    Serial.print(" ");
    Serial.println(dataOut[0], BIN);

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
    dataOut[0] = B00000001;
  }


  digitalWrite(LED_PIN, LED_STATE);

}
