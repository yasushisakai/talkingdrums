//---------------Main loop---------------------------
//----------------------------------------------------
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
//----------------------------
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
