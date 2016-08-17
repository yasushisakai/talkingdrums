#include <RH_NRF24.h>
#include "helpers.h"
#include "defines.h"

/*
        ___           ___           ___           ___           ___           ___
       /\  \         /\  \         /\  \         /\__\         /\  \         /\  \
      /::\  \       /::\  \       /::\  \       /:/  /        /::\  \       /::\  \
     /:/\ \  \     /:/\:\  \     /:/\:\  \     /:/  /        /:/\:\  \     /:/\:\  \
    _\:\~\ \  \   /::\~\:\  \   /::\~\:\  \   /:/__/  ___   /::\~\:\  \   /::\~\:\  \
   /\ \:\ \ \__\ /:/\:\ \:\__\ /:/\:\ \:\__\  |:|  | /\__\ /:/\:\ \:\__\ /:/\:\ \:\__\
   \:\ \:\ \/__/ \:\~\:\ \/__/ \/_|::\/:/  /  |:|  |/:/  / \:\~\:\ \/__/ \/_|::\/:/  /
    \:\ \:\__\    \:\ \:\__\      |:|::/  /   |:|__/:/  /   \:\ \:\__\      |:|::/  /
     \:\/:/  /     \:\ \/__/      |:|\/__/     \::::/__/     \:\ \/__/      |:|\/__/
      \::/  /       \:\__\        |:|  |        ~~~~          \:\__\        |:|  |
       \/__/         \/__/         \|__|                       \/__/         \|__|

*/


RH_NRF24 nrf24;

bool isSend = true;
unsigned long previousTime;
int counter;
bool sendActivation;

bool LED_STATE;

uint8_t data [] = {0B00000001};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);

  initNRF(nrf24);

  previousTime = millis();
  counter = 0;
  isSend = LED_STATE = false;

  //data = TICK;

}

void loop() {
  unsigned long currentTime = millis();

  if (timer(currentTime, previousTime, DURATION)) {
    previousTime = currentTime;
    LED_STATE = HIGH;
    isSend = true;
  }

  if (isSend) {
    nrf24.send((uint8_t*)data, sizeof(data));
    nrf24.waitPacketSent();

    if (counter >= COUNTER_LIMIT) {
      counter = 0;
      isSend = false;
      LED_STATE = LOW;
    }
    counter ++;
  }


  digitalWrite(LED_PIN, LED_STATE);

}
