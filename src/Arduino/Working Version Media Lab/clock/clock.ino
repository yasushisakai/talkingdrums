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

  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if (nrf24.recv(buf, &len))
  {
    Serial.print("got request: ");
    Serial.println(buf[0]);
    dataOut[1] = buf[0];
  }

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

// an example of a string
// 018P45
// means
// change no. 18's PWM to 25 out of 32
// and the command will be
// { B00100101, B11001001 }
void  msgToByteCommand (String msg) {
  // Serial.print("no: ");

  uint8_t  no = msg.substring(0, 3).toInt() << 1 ^ 0x1;

  char mode = msg[3];
  uint8_t modeInt = 1;

  switch (mode) {
    case 'P': // PWM
      modeInt = B00000001;
      break;
    case 'M': // MICTHRESHOLD
      modeInt = B00000101;
      break;
    case 'R':
      modeInt = B00001001;
      break;
    case 'T':
      modeInt = B00001101;
      break;
    case 'S':
      modeInt = B00010001;
      break;
    default:
      break;
  }

  if (no > 127) {
    modeInt ^= 2;
  }

  uint8_t  val = msg.substring(4, 7).toInt();

  dataOut[0] = no;
  dataOut[1] = modeInt;
  dataOut[2] = val;

  return;
}

void initByteCommand () {
  dataOut[0] = B00000001;
  dataOut[1] = B00000000;
  dataOut[2] = B00000000;
}
