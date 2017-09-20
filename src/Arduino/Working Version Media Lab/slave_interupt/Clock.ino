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

      //default sequence
      case (START_MODULE):
        sequenceState = WAIT_START;
        break;
      case (STOP_MODULE):
        sequenceState = STOP;
        if (DEBUG)Serial.println("STOP");
        break;


      case (PWM_MODULE):
        solenoid_pwm = int(map(inValue, 0, 32, 0, 255));
        if (DEBUG)Serial.println("new PWM: ");
        if (DEBUG)Serial.println(solenoid_pwm);
        break;
      case (MIC_MODULE):
        micThreshold = int(map(inValue, 0, 32, 0, 255));
        if (DEBUG)Serial.println("New MIC: ");
        if (DEBUG)Serial.println(micThreshold);
        break;
    }
    activateNRFMode = 0;
  }


}
