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

    if (useHeader) {
      setSequenceState(CALIBRATE_MIC);
    } else {
      setSequenceState(LISTEN_SEQUENCE);
    }

    TimeKeeper::signalLimit  = 2;
  }
  TimeKeeper::signalCount++;

  if (DEBUG) Serial.println(clockCounter);
}

//calibrate Microphone
void calibrateMic() {
  if (DEBUG) Serial.println("CALIBRATING");
  sequenceState = LISTEN_HEADER;
}



//listen to the incomming date from the microphone
void listenSequence() {

  //get if from the bandpass filter
  micHit = bandPassFilter.isHit(micThreshold);

  if (DEBUG) printSequenceIndex();

  if (bitIndex < SEQBITS )
    recording[sequenceIndex][bitIndex] = micHit;

  bitIndex++;
  if (bitIndex > SEQBITS) {// >=
    setSequenceState(ANALYZE);
  }

  if (DEBUG) Serial.println(clockCounter);


}







//analyze the for sequence
void analyzeSequence() {
  sequenceIndex++;
  bitIndex = 0;

  if (sequenceIndex < SEQITER) {
    if (DEBUG) Serial.print("WAIT.. ");
    sequenceState = LISTEN_SEQUENCE;
  } else {
    if (DEBUG) Serial.print("FINAL  ");
    //
    // gets the average and defines what it heard to "playSequence"
    //
    for (itri = 0; itri < SEQBITS; itri++) {
      float average = 0.0;
      for (itrj = 0; itrj < SEQITER; itrj++) {
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

    if (useHeader) {
      setSequenceState(HEADER_PLAY);
    } else {
      setSequenceState(PULSE_PLAY);
    }

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
    if (DEBUG)printRecordings();

  }

  if (DEBUG) Serial.println(clockCounter);
}


//play a pulse dependeing on the microphone readings
void pulsePlay() {
  if (DEBUG) Serial.print(playSequence[bitIndex]);
  if (DEBUG) Serial.print(" ");
  if (DEBUG) Serial.print(bitIndex);
  if (DEBUG) Serial.print("  ");

  if (playSequence[bitIndex] && bitIndex < SEQBITS ) timeKeeper.hit();

  bitIndex++;
  if (bitIndex > SEQBITS) { //==
    bitIndex = 0;
    if (DEBUG) Serial.print("  go to wait play ");
    setSequenceState(WAIT_PLAY);
  }

  if (DEBUG) Serial.println(clockCounter);

}

//wait play function between seequence readings
void waitPlay() {
  bitIndex = 0;
  sequenceIndex++;

  // did it play it for enough times??
  if (sequenceIndex == SEQITER) {
    // yes, proceed to reset

    //if its the slave go to read input instead of reset values

    setSequenceState(RESET);


  } else {
    // nope go back playing
    setSequenceState(PULSE_PLAY);

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


  clockCounter = 0;
  resetCounter = 0;

  if (useHeader) {
    setSequenceState(LISTEN_HEADER);
  } else {
    setSequenceState(LISTEN_HEADER);
  }

  // reset values
  bitIndex = 0;
  sequenceIndex = 0;

  isFirstHit     = true;
  isRecordHeader = false;
  isHead         = false;

  resetSequence();

  if (DEBUG) Serial.println(clockCounter);
}

void readInputArray() {
  if (clockCounter > LIMIT_READ_COUNTER) {


    if (readInBytes) {
      if (DEBUG) Serial.println(" ");
      if (DEBUG) Serial.println("incoming bytes");

      int val = Serial.readBytes(byteMSG8, 1);

      // Reset values when an array of bits is received
      if (val > 0 || DEBUG == true) {
        if (DEBUG) Serial.println("clean Serial");


        if (DEBUG) {
          Serial.print("Number cycles ");
          Serial.println(clockCounter);
        }
        //clean

        Serial.flush();
        for (itri = 0; itri < 10; itri++) {
          char f = Serial.read();
        }

        for (itri = 0; itri < 8; itri++) {
          playSequence[itri] = (bitRead((byte)byteMSG8[0], itri ) == 1 ? true : false);
          //send it
          // playSequence[itri] = (bitRead(byteMSG8[0], 7 - itri ) == 1 ? true : false);
        }

        inCommingMSg[0] = byteMSG8[0];


        if (SERVER_SLAVE == 2) {
          Serial.print("sent  ");
          Serial.println(inCommingMSg[0]);
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

      Serial.println("s");

      requestByte = false;
      readInBytes = true;
    }
  }
  if (DEBUG) Serial.print(clockCounter);
}

//---------------Main loop
//----------------------------------------------------
void activateSequenceLoop() {
  if (!lock) {
    switch (sequenceState) {
      case STOP:
        //debugTimes();     //debug timers
        break;

      case TEST_MIC:  //debug mic
        testMic();
        break;

      case TEST_SOLENOID: //test for the solenoid

        //test a individual sequence
        //testSolenoid();

        //test the sequence including header repetetitions
        testDebugSequence();
        break;
      //finish with the test

      //calibration for initial values for the MIC
      case CALIBRATE_MIC: //Calibrate value from Mic
        calibrateMic();
        break;

      //hear the NRF
      case CALIBRATE_TIME:
        /*
          obtain the max  time tick from the RF. then only call RF methods only at the las 10ms,
          to save energy and process time.
        */
        break;

      //start the sequence
      /*
          WAIT_START
          LISTEN_HEADER
          LISTEN_SEQUENCE
          ANALYZE

          HEADER * 3

          PULSE_PLAY
          WAIT_PLAY

          PULSE_PLAY
          WAIT_PLAY

          PULSE_PLAY
          WAIT_PLAY

          RESET -> go to  LISTEN_HEADER
      */
      case WAIT_START:  //init values sequence
        waitStart();
        break;

      /*
        listens
        1. listens for the right header
        2. listens for the sequence
      */

      case LISTEN_HEADER:
        if (DEBUG) Serial.print("LISTEN HEADER ");
        listenHeader();
        break;

      case LISTEN_SEQUENCE:
        if (DEBUG) Serial.print("LISTEN SEQUENCE ");
        listenSequence();
        break;

      case ANALYZE:
        if (DEBUG) Serial.print("ANALYZING  ");
        analyzeSequence(); // collects and analyses the readings,
        break;

      case PULSE_PLAY:
        if (DEBUG) Serial.print("PULSE PLAY  ");
        pulsePlay(); // plays single pulse
        break;

      case HEADER_PLAY:
        if (DEBUG) Serial.print("HEADER PLAY  ");
        headerPlay();
        break;



      case WAIT_PLAY:
        if (DEBUG)Serial.print("WAINTING PLAY ");
        waitPlay();
        break;

      case RESET:
        if (DEBUG) Serial.print("L: RESET ");
        /*
          returns to PULSE_PLAY if there is iterations left to play
          (may not need this phase though)
        */

        //if its the slave go to read input instead of reset values
        if (SERVER_SLAVE == 1 || SERVER_SLAVE == 2 ) {
          if (resetCounter >= 10) {
            setSequenceState(READ_INPUT);
            readInBytes = false;
            requestByte = true;
            resetCounter = 0;
          }
        } else {
          if (resetCounter >= 3) {
            resetLoop();
            resetCounter = 0;
          }
        }
        resetCounter++;
        break;
      case READ_INPUT:
        if (DEBUG) Serial.println("READ INPUT ");
        readInputArray();
        break;
      case SEND_INPUT:
        if (DEBUG) Serial.println("SEND INPUT ");
        activateSend = true;
        break;

    } // switch
    
    
    //clean bandpass
    //Serial.print("clean");
    bandPassFilter.resetSignalMinMax();
    bandPassFilter.resetBuffer();
    lock = !lock;
  }
}

