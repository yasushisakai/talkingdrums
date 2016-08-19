#include "define.h"
#include "helpers.h"
#include "TimeKeeper.h"

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


// Objects
RH_NRF24 nrf24;

//TIMERS
TimeKeeper timeKeeper;

///DEBUG
bool const DEBUG = true;
bool const DEBUG_PORT = true;

///Sequence
byte sequenceState, sequenceIndex, bitIndex;
bool lock, isRecord;
bool recording[SEQITER][SEQBITS];
bool playSequence[SEQBITS];
bool debugSequence[] = {1, 0, 0, 1, 1, 0, 0, 1};

///Signal Processing
int signalMin, signalMax;
const int signalThreshold = 800; // 50-1024 we may need to make this dynamic

/// PWM-ing the Solenoid will need additional test 0-255
byte const solenoid_pwm = 200;

//Serial Port
bool requestByte = false;

//incoming msg, keep it as an array in case we need to
//read values bigger than a byte
byte byteMSG8[] = {
  0B00000000
};

void setup() {

  Serial.begin(19200);

  //enable pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);
  pinMode(LED_PIN_00, OUTPUT);
  pinMode(LED_PIN_01, OUTPUT);
  pinMode(LED_PIN_02, OUTPUT);

  //turn on pin test
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(SOL_PIN, HIGH);

  //LEDS
  digitalWrite(LED_PIN_00, HIGH);
  digitalWrite(LED_PIN_01, HIGH);
  digitalWrite(LED_PIN_02, HIGH);

  analogWrite(LED_PIN_03, 255);
  analogWrite(LED_PIN_04, 255);
  analogWrite(LED_PIN_05, 255);
  analogWrite(LED_PIN_06, 255);
  analogWrite(LED_PIN_07, 255);
  delay(1000);

  //turn off
  digitalWrite(LED_PIN, LOW);
  digitalWrite(SOL_PIN, LOW);

  //LEDs
  digitalWrite(LED_PIN_00, LOW);
  digitalWrite(LED_PIN_01, LOW);
  digitalWrite(LED_PIN_02, LOW);
  analogWrite(LED_PIN_03, 0);
  analogWrite(LED_PIN_04, 0);
  analogWrite(LED_PIN_05, 0);
  analogWrite(LED_PIN_06, 0);
  analogWrite(LED_PIN_07, 0);

  //sequence
  sequenceState = WAIT;
  bitIndex = sequenceIndex = 0;
  lock = true;
  isRecord = false;
  for (char i = 0; i < SEQBITS; i++) {
    playSequence[i] = false;
    for (char j = 0; j < SEQITER; j++) {
      recording[j][i] = false;
    }
  }

  //signal
  signalMin = 1024;
  signalMax = 0;

  timeKeeper.setInterval(INTERVAL);

  initNRF(nrf24);
}


void loop() {
 // unsigned long currentTime = millis();

  // updates the timeKeeper
 // timeKeeper.cycle(currentTime);

  // unlocks if we recieve a TICK from the server
  // and timeFrame is more than TIMEFRAMEINTERVAL (60ms)
  uint8_t value = 0B00000001;
  //Serial.println("");//timeKeeper.getTimeFrame());

  Serial.println("hell");
  if (checkServer(nrf24, value) ){//&& timeKeeper.getTimeFrame() > TIMEFRAMEINTERVAL) {
    value = TOCK;
    timeKeeper.tick();
    timeKeeper.flash();
    lock = false;
    requestByte = true;
    Serial.println("thomas");
    // Serial.flush();
  }

}
