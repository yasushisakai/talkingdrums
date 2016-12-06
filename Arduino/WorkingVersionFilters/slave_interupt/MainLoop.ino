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

    if(useHeader){
      setSequenceState(CALIBRATE_MIC);
    }else{
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

  if (DEBUG) Serial.print("LISTEN SEQUENCE");

  //get if from the bandpass filter
  micHit = bandPassFilter.isHit();

  if (DEBUG) printSequenceIndex();

  recording[sequenceIndex][bitIndex] = micHit;
  bitIndex++;
  if (bitIndex >= SEQBITS) {
    setSequenceState(ANALYZE);
  }

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
    setSequenceState(PULSE_PLAY);

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

  if (DEBUG) Serial.print("PLAY  ");
  if (DEBUG) Serial.print(playSequence[bitIndex]);

  if (playSequence[bitIndex]) timeKeeper.hit();
  bitIndex++;

  if (bitIndex == SEQBITS) {
    bitIndex = 0;
    setSequenceState(WAIT_PLAY);
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

    //if its the slave go to read input instead of reset values
    if (SERVER_SLAVE == 1) {
      setSequenceState(READ_INPUT);
      readInBytes = false;
      requestByte = true;
    } else {
      setSequenceState(RESET);
    }

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

  if (DEBUG) Serial.print("L: RESET ");

  clockCounter = 0;

  if(useHeader){
    setSequenceState(LISTEN_HEADER);
  }else{
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
  if (readInBytes) {
    if (DEBUG) Serial.println("incoming bytes");

    int val = Serial.readBytes(byteMSG8, 1);

    // Reset values when an array of bits is received
    if (val > 0) {
      if (DEBUG) Serial.println("clean Serial");


      if (DEBUG) {
        Serial.print("Number cycles");
        Serial.println(clockCounter);
      }
      //clean

      Serial.flush();

      for (itri = 0; itri < 10; itri++) {
        char f = Serial.read();
      }

      for (itri = 0; itri < 8; itri++) {
        playSequence[itri] = (bitRead(byteMSG8[0], 7 - itri ) == 1 ? true : false);
      }

      readInBytes = true;
      requestByte = false;
      bitIndex = 0;
      sequenceIndex = 0;
      clockCounter   = 0;

      if (useHeader) {
        setSequenceState(HEADER_PLAY);
      } else {
        setSequenceState(LISTEN_SEQUENCE);
      }
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

    Serial.write('s');
    requestByte = false;
    readInBytes = true;
  }

}

//---------------Main loop
//----------------------------------------------------
void acticateSequenceLoop() {
  if (!lock) {
    switch (sequenceState) {
      case TEST_TIMERS:
        debugTimes();     //debug timers
        break;

      case TEST_MIC:  //debug mic
        testMic();
        break;

      case TEST_SOLENOID: //test for the solenoid
        testSolenoid();
        break;
      //finish with the test

      //calibration for initial values for the MIC
      case CALIBRATE_MIC: //Calibrate value from Mic
        calibrateMic();
        break;

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

      case HEADER_PLAY:
        headerPlay();
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
      case READ_INPUT:
        readInputArray();
        break;

    } // switch
    lock = !lock;
  }
}

