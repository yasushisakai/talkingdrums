/*
   Input functions
*/


void readFromSender(bool * readSender, byte serialBytes) {
  for (itri = 0; itri < 8; itri++) {
    readSender[itri] = (bitRead(serialBytes, itri ) == 1 ? true : false);
  }

}



void readInputArray() {
  if (clockCounter > LIMIT_READ_COUNTER) {

    if (readInBytes) {
      if (DEBUG) Serial.println(" ");
      if (DEBUG) Serial.println("incoming bytes");

      int val = Serial.readBytes(byteMSG8, 3);

      // Reset values when an array of bits is received
      if (val > 0 ) {

        if (DEBUG) {
          Serial.print("Number cycles ");
          Serial.println(clockCounter);
        }


        //clean
        if (DEBUG) Serial.println("clean Serial");
        Serial.flush();
        for (itri = 0; itri < 10; itri++) {
          char f = Serial.read();
        }

        //byte from the sender
        readFromSender(playSequence, byteMSG8[1]);

        //copy
        inCommingMSg[0] = byteMSG8[1];

        if (SERVER_SLAVE == 2) {
          if (DEBUG_IN) {
            Serial.print("sent  ");
            Serial.println(inCommingMSg[0]);
          }
        }

        if (DEBUG_IN) {
          Serial.print("read in: ");
          for (itri = 0; itri < 8; itri++) {
            Serial.print( playSequence[itri] );
            Serial.print(" " );
            // playSequence[itri] = (bitRead(byteMSG8[0], 7 - itri ) == 1 ? true : false);
          }
          Serial.println(" - ");
        }

        // 7 - itri
        readInBytes = true;
        requestByte = false;
        bitIndex = 0;
        sequenceIndex = 0;
        clockCounter = 0;

        if (DEBUG) Serial.print(clockCounter);
        if (DEBUG) Serial.print(" ");


        setSequenceState(SEND_INPUT);

        //fill header
        for (itri = 0; itri < 3; itri++) {
          headerSequence[itri] = correctHeader[itri];
        }

        //make sure that we are going to play the header
        isHead = true;

      } //got msg
    }

    //send byte request and read
    if (requestByte) {
      if (DEBUG) Serial.println("request bytes");

      //change to println() to enable easier read with readString() in processing
      //Serial.write('s');

      byte value  = B00000000;
      byte readyValue[3] = {READY_BYTE, value, FINAL_BYTE};
      Serial.write(readyValue, 3);

      requestByte = false;
      readInBytes = true;
    }
  }
  if (DEBUG) Serial.print(clockCounter);
}

