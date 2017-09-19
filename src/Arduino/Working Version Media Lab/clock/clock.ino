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

bool LED_STATE; // is always same with isSend?

// 1
int currentMode = 1;
byte buf [2];
int intBuf = 0;

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
                       B00001010,  // 10  PWM Value  0
                       B00001011   //11
                      };

String dataModeString [] = {
  "Default seq",
  "Default seq",
  "Test MIC",
  "Stop",
  "PWM Value 255",
  "PWM Value 200",
  "PWM Value 150",
  "PWM Value 100",
  "PWM Value 50",
  "PWM Value 0"
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

// this part changes the MODE
  if (Serial.available() > 0) {
    intBuf = Serial.readString().toInt();
    Serial.print("recieved: ");
    Serial.println(intBuf);
    if (intBuf < sizeof(dataMode)/sizeof(dataMode[0]) ){
      dataOut[0] = dataMode[intBuf];
      Serial.print("changed MODE to: ");
      Serial.print(dataModeString[intBuf]);
      Serial.print(" byte:(");
      Serial.print(dataOut[0],BIN);
      Serial.println(")");
    } else {
      Serial.println("index out of MODE configs, ignoring");
      Serial.println();
    }
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
    if(dataOut[0] != B00000001){
      Serial.println("back to default seq.");
      Serial.println("");
      dataOut[0] = B00000001;
    }
  }


  digitalWrite(LED_PIN, LED_STATE);

}
