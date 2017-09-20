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

// the dataOut is the data  broadcasted to the slave devices
uint8_t dataOut [2];

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
      initByteCommand();
    }
  }


  digitalWrite(LED_PIN, LED_STATE);

}

void  msgToByteCommand (String msg) {
  // Serial.print("no: ");

  uint8_t  no = msg.substring(0,3).toInt() << 1 ^ 0x1;
	
  // Serial.println(no,BIN); 

  uint8_t  val = msg.substring(4,6).toInt() << 3 ^ 0x1; 
  // last digit should be always 1

  char mode = msg[3];
  
  switch (mode) {
    case 'P': // PWM
    // do nothing
    break;
    case 'M': // MICTHRESHOLD
    // add 2
    val ^= 0x2;
    break;
    default:
    break;
  }

  // an example of a string
  // 018P45
  // means
  // change no. 18's PWM to 25 out of 32
  // and the command will be
  // { B00100101, B11001001 }

  dataOut[0] = no;
  dataOut[1] = val;

  return;
}

void initByteCommand () {
  dataOut[0] = B00000001;
  dataOut[1] = B00000000;
}
