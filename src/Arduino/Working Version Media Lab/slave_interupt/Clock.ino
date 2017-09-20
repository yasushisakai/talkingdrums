void clockMode(uint8_t & inCLK, uint8_t  inMode, uint8_t  inValue ) {

  if (inCLK == TICK) {
    if (timeKeeper.getTimeTick() >= TIME_MIN_INTERVAL) {
      timeKeeper.tick();
      timeKeeperNRF.tick();

      //Serial.print("MSG ");
      //Serial.print(valueByte);
      //Serial.print(" ");
      //Serial.println(timeKeeper.getTimeTick());

      lock = false;
      clockCounter++;
      inCLK = TOCK;

      //Serial.println(tempConter);
      //tempConter=0;
    }
  }


  //set the value only once
  if (activateNRFMode == 1) {
    switch (inMode) {

      case (PWM_MODE):
        solenoid_pwm = inValue;
        if (DEBUG)Serial.println("new PWM: ");
        if (DEBUG)Serial.println(solenoid_pwm);
        break;
        
      case (MIC_MODE):
        micThreshold = inValue;
        if (DEBUG)Serial.println("New MIC: ");
        if (DEBUG)Serial.println(micThreshold);
        break;

      case (RETURN_MODE):
        bitIndex = sequenceIndex = 0;
        sequenceState = WAIT_START;
        resetSequence();
        if (DEBUG)Serial.println("Return");
        break;
        
      case (SONLENOID_MODE):
        setSequenceState(TEST_SOLENOID);

        for (itri = 0; itri < 8; itri++) {
          debugSequenceTap[3 + itri] =  bitRead(inValue, itri );
        }
        if (DEBUG)Serial.println("New SONLENOID Test: ");
        break;

      case (CALIBRATE_MODE):
        setSequenceState(CALIBRATE_MIC);
        if (DEBUG)Serial.println("Calibrate Mic");
        break;
        
      case (DEBUG_MODE):
        DEBUG = !DEBUG;
        Serial.print("DEBUG Toggle ");
        Serial.println(DEBUG);
        break;

    }
    activateNRFMode = 0;
  }


}
