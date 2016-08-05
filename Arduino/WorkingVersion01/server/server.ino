#include <RH_NRF24.h>
#include "helpers.h"
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

#define LED_PIN 3
#define DURATION 100
#define COUNTER_LIMIT 3

#define TICK 49
#define TOCK 48

RH_NRF24 nrf24;

unsigned long previousTime;
int counter;
bool sendActivation;

bool LED_STATE;
char data;

void setup() {
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  initNRF(nrf24);

  previousTime = millis();
  counter = 0;
  isSend = LED_STATE = false;

  data = TICK;

}

void loop() {
  unsigned long currentTime = millis();

  if (timer(currentTime, previousTime, DURATION)) {
    previousTime = currentTime;
    LED_STATE = HIGH;
  }

  if (isSend) {
    nrf24.send(data, sizeof(data));
    nrf24.waitPacketSent();
    
    if (counter >= COUNTER_LIMIT) {
      counter = 0;
      isSend = false;
      LED_STATE = LOW;
    }
    counter ++;
  }
  digitalWrite(LED_PIN,LED_STATE);
}
