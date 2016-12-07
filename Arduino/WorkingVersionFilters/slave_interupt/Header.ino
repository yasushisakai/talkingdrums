void listenHeader() {
  if (DEBUG) Serial.print("LISTEN HEADER ");

  //get hit from the bandpass filter
  micHit = bandPassFilter.isHit();

  if (isRecordHeader) {

    if (micHit) timeKeeper.hit(); //led feedback

    headerSequence[bitIndex] = micHit;
    bitIndex++;

    isHead = true;

    //Analyze to pass to the next stage
    if (bitIndex == numHeaderBits) { // 9
      if (DEBUG) {
        Serial.print("AH ");
        for (itri = 0; itri < numHeaderBits; itri++) {
          Serial.print(headerSequence[itri]);
        }
        Serial.print("");
      }

      double headers[SEQITER];
      for (itri = 0; itri < SEQITER; itri++) headers[itri] = 0.0;

      for (itri = 0; itri < SEQITER; itri++) {
        for (itrj = 0; itrj < 3; itrj++) {
          headers[itri] +=  headerSequence[itri + itrj * SEQITER];
        }
      }

      int errorCheck = 0;
      for (itri = 0; itri < SEQITER; itri++) {
        if (correctHeader[itri] != (headers[itri] / SEQITER) > 0.5 ) {
          errorCheck++;
        }
      }
      if (DEBUG) {
        Serial.print(" e ");
        Serial.print(errorCheck);
      }
      //OK to have one error, its the header..
      isHead = (errorCheck <= 1) ? true : false;

      //RESET HEADER
      //if didn't found the header then reset the values of headerSequence
      if (isHead == false) {
        for (itri = 0; itri < numHeaderBits; itri++) {
          headerSequence[itri] = false;
        }
        //reset the the initial values
        isRecordHeader = false;
        bitIndex = 0;
        isFirstHit = true;
        micHit = false;
        if (DEBUG) Serial.print("RH ");
      }
    }//finish analyzing the data,
    //if the header is correct and the number of bitIndex == 9 then pass the play the sequence


    if (DEBUG) Serial.print("B: ");
    if (DEBUG) Serial.print(bitIndex);
    if (DEBUG) Serial.print(" ");

    if (isHead && bitIndex == numHeaderBits) {
      //Serial.print("L: found head"); // notify head detection to ImageReciever

      if (DEBUG) {
        Serial.print("L: h=");
        for (itri = 0; itri < numHeaderBits; itri++)
          Serial.print(headerSequence[itri]);
      }

      //go to listen the sequence  and RESET values
      bitIndex = 0; //reset!
      //clockCounter = 3;
      micHit = false;
      isFirstHit = true;
      setSequenceState(LISTEN_SEQUENCE);

    }
  }
  //if we found a hit then we can start anaylzing the header
  if (isFirstHit && micHit) {
    headerSequence[bitIndex] = micHit;
    bitIndex ++;
    isRecordHeader = true;
    isFirstHit     = false;
    clockCounter = 1;
    timeKeeper.hit();//led feedback
    if (DEBUG) Serial.print("FH ");
  }

  if (DEBUG) Serial.println(clockCounter);
}

void headerPlay() {

  if (DEBUG) Serial.print("s ");
  if (DEBUG) Serial.print(headerSequence[bitIndex]);

  if (headerSequence[bitIndex]) timeKeeper.hit();
  bitIndex++;

  if (bitIndex == 3 ) { //110
    headerBitCounter++;
    bitIndex = 0;

    if (headerBitCounter == SEQITER) {
      headerBitCounter = 0;
      setSequenceState(PULSE_PLAY); // it won't go to WAIT_PLAY
    }

  }

  if (DEBUG) Serial.println(clockCounter);
}
