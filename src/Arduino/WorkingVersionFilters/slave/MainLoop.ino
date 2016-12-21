void listenHeader() {
  if (DEBUG) Serial.print("LISTEN HEADER ");

  //micHit = isHit();

  if (isRecordHeader) {

    headerSequence[bitIndex] = micHit;
    bitIndex++;

    isHead = true;

    //Analyze to pass to the next stage
    if (bitIndex == numHeaderBits) {
      if (DEBUG) {
        Serial.print("AH ");
        Serial.println("");
        for (itri = 0; itri < numHeaderBits; itri++) {
          Serial.print(headerSequence[itri]);
        }
        Serial.println("");
      }

      double headers[SEQITER];
      for (itri = 0; itri < SEQITER; itri++) headers[itri] = 0.0;

      for (itri = 0; itri < SEQITER; itri++) {
        for (itrj = 0; itrj < 3; itrj++) {
          headers[itri] +=  headerSequence[itri + itrj * SEQITER];
        }
      }

      int countCheck = 0;
      for (itri = 0; itri < SEQITER; itri++) {
        if (correctHeader[itri] != (headers[itri] / SEQITER) > 0.5 ) {
          countCheck++;
        }
      }

      //OK to have one error, its the header..
      isHead = (countCheck <= 1) ? true : false;

      //RESET HEADER
      //if didn't found the header then reset the values of headerSequence
      if (isHead == false) {
        for (itri = 0; itri < numHeaderBits; itri++) {
          headerSequence[itri] = false;
        }
        isRecordHeader = false;
        bitIndex = 0;
        isFirstHit = true;
        micHit = false;
        if (DEBUG) Serial.print("RH ");
      }

      // forces the head to pass if careHead is off.
      if (!careHeader) {
        isHead = true;
      }
    }


    if (DEBUG) Serial.print("B: ");
    if (DEBUG) Serial.print(bitIndex);
    if (DEBUG) Serial.print(" ");

    if (isHead && bitIndex == numHeaderBits) {
      Serial.println("L: found head"); // notify head detection to ImageReciever

      if (DEBUG) {
        Serial.print("L: h=");
        for (itri = 0; itri < numHeaderBits; itri++)
          Serial.print(headerSequence[itri]);
        Serial.println();
      }

      //go to listen the sequence  and RESET values
      bitIndex = 0; //reset!
      clockCounter = 3;
      micHit = false;
      isFirstHit = true;
      sequenceState = LISTEN_SEQUENCE;

    }
  }

  //if we found a hit then we can start anaylzing the header
  if (isFirstHit && micHit) {
    headerSequence[bitIndex] = micHit;
    bitIndex ++;
    isRecordHeader = true;
    isFirstHit     = false;
    if (DEBUG) Serial.print("FH ");
  }


  if (DEBUG) Serial.println(clockCounter);
}

//listen to the incomming date from the microphone
void listenSequence() {

  if (DEBUG) Serial.print("LISTEN SEQUENCE");

  //micHit = isHit();

  if (DEBUG) {
    Serial.print("L: ");
    Serial.print(sequenceIndex);
    Serial.print(", ");
    Serial.print(bitIndex);
    Serial.print(", ");
    Serial.print(micHit);
    Serial.print(" ");
  }
  recording[sequenceIndex][bitIndex] = micHit;
  bitIndex++;
  if (bitIndex >= SEQBITS) {
    sequenceState = ANALYZE;
  }


  if (DEBUG) Serial.println(clockCounter);
}

//wait to start
void waitStart() {
  /*
    initial wait for 3 cycles for a stable mic reading
  */
  if (!TimeKeeper::wait()) {

    if (DEBUG) {
      Serial.print("L: WAIT_START ");
      Serial.print(TimeKeeper::signalLimit);
      Serial.print(" ");
      Serial.print(TimeKeeper::signalCount);
      Serial.print(" ");
    }
    sequenceState = CALIBRATE_MIC;

    TimeKeeper::signalLimit  = 2;
  }
  TimeKeeper::signalCount++;

  if (DEBUG) Serial.println(clockCounter);
}



//analyze the for sequence
void analyzeSequence() {
  sequenceIndex++;
  bitIndex = 0;

  if (sequenceIndex < SEQITER) {
    if (DEBUG) Serial.print("WAIT ANALYZE  ");
    sequenceState = LISTEN_SEQUENCE;
  } else {
    if (DEBUG) Serial.print("ANALYZING  ");
    //
    // gets the average and defines what it heard to "playSequence"
    //
    for (itri = 0; itri < SEQBITS; itri++) {
      float average = 0.0;
      for (itrj = 0; itrj < SEQITER; itrj ++) {
        average += recording[itrj][itri];
        if (DEBUG) {
          Serial.print(" ");
          Serial.print(average);
          Serial.print(" ");
        }
      }
      playSequence[itri] = average >= 0.5 * SEQITER;
      if (DEBUG) Serial.println(playSequence[itri]);
    }

    sequenceIndex = 0;
    sequenceState = PULSE_PLAY;

    //make sure that we are going to play the header
    isHead = true;

    //
    //
    // IMPORTANT!! this line is for the ImageReciever app!!
    //
    //
    Serial.print("L: r=");
    for (itri = 0; itri < SEQBITS; itri++) {
      Serial.print(playSequence[itri]);
    }
    Serial.println();

    //
    // prints the recordings
    //
    if (DEBUG) {

      Serial.println("L: Done Analyze");

      for (itri = 0; itri < SEQITER; itri++) {
        Serial.print("L: ");
        Serial.print(itri);
        Serial.print('=');
        for (itrj = 0; itrj < SEQBITS; itrj++) {
          Serial.print(recording[itri][itrj]);
        }
        Serial.println();
      }

      // check sequence if its correct

      bool flag = true;
      for (itri = 0; itri < SEQBITS; itri++) {
        if (debugSequence[itri] != playSequence[itri]) {
          flag = !flag;
          break;
        }
      }
      if (flag) {
        Serial.println("L:sequence correct");
      } else {
        Serial.println("L:sequence incorrect");
      }

      Serial.print("L: playing=");
      Serial.print(sequenceIndex);
      Serial.print(", ");
    }

  }

  if (DEBUG) Serial.println(clockCounter);
}

//play a pulse dependeing on the microphone readings
void pulsePlay() {

  if (DEBUG) Serial.print("PLAY  ");

  if (isHead) {
    if (headerSequence[bitIndex]) timeKeeper.hit();
    bitIndex++;

    if (DEBUG) Serial.print(headerSequence[bitIndex]);

    if (bitIndex >= sizeof(correctHeader) / sizeof(bool)) {
      isHead = false;
      bitIndex = 0;
      // even if it ends playing the header,
      // it won't go to WAIT_PLAY
    }
  } else {
    if (DEBUG) Serial.print(playSequence[bitIndex]);

    if (playSequence[bitIndex]) timeKeeper.hit();

    bitIndex++;
    if (bitIndex == SEQBITS) {
      bitIndex = 0;
      sequenceState = WAIT_PLAY;
    }
  }

  if (DEBUG) Serial.println(clockCounter);
}

//wait play function between seequence readings
void waitPlay() {

  if (DEBUG)Serial.print("Waiting play ");

  bitIndex = 0;
  sequenceIndex++;

  // did it play it for enough times??
  if (sequenceIndex == SEQITER) {
    // yes, proceed to reset
    sequenceState = RESET;
  } else {
    // nope go back playing
    sequenceState = PULSE_PLAY;
    if (DEBUG) {
      Serial.print("L: playing= ");
      Serial.print(sequenceIndex);
      Serial.print(" ");
    }
  }

  if (DEBUG) Serial.println(clockCounter);
}

//Rest main loop
void resetLoop() {

  // the whole process (including the first head detection) is 61 steps.
  // head + (SEQBITS + gap)*SEQITER + head + (SEQBITS + gap) * SEQITER + RESET
  // 3 + (8+1)*3 + 3 + (8+1)*3 + 1 = 30 + 30 + 1 = 61
  // https://docs.google.com/spreadsheets/d/1OzL0YygAY_DSaA0wT8SLMD5zovmk4wVnEPwZ80VJpbA/edit#gid=0

  // starting from 0, so end is 60

  if (DEBUG) Serial.print("L: RESET ");

  clockCounter = 0;
  sequenceState = LISTEN_HEADER;

  // reset values
  bitIndex = 0;
  sequenceIndex = 0;

  isFirstHit     = true;
  isRecordHeader = false;
  isHead         = false;

  resetSequence();

  if (DEBUG) Serial.println(clockCounter);
}

//--Main loop
void acticateSequenceLoop() {
  if (!lock) {
    switch (sequenceState) {
      case WAIT_DEBUG:
        debugTimes();     //debug timers
        break;

      case TEST_MIC:  //debug mic
        testMic();
        break;

      case TEST_SOLENOID: //test for the solenoid
        testSolenoid();
        break;

      case WAIT_START:  //init values sequence
        waitStart();
        break;

      case CALIBRATE_MIC: //Calibrate value from Mic
        calibrateMic();
        break;

      case CALIBRATE_TIME:
        /*
          obtain the max  time tick from the RF. then only call RF methods only at the las 10ms,
          to save energy and process time.
        */
        break;
        
      case LISTEN_HEADER:
        listenHeader();
        break;
        
      case LISTEN_SEQUENCE:
        /*
          listens
          1. listens for the right header
          2. listens for the sequence
        */
        listenSequence();
        break;
        
      case ANALYZE:
        analyzeSequence(); // collects and analyses the readings, 
        break;
        
      case PULSE_PLAY:
        pulsePlay(); // plays single pulse
        break;
        
      case WAIT_PLAY:
        waitPlay();
        break;
        
      case RESET:
        /*
          returns to PULSE_PLAY if there is iterations left to play
          (may not need this phase though)
        */
        resetLoop();
        break;
        
    } // switch
    lock = !lock;
  }
}

