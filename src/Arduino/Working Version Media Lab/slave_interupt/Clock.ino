void clockMode(uint8_t inByte) {

  if (inClock == TICK) {
    if (timeKeeper.getTimeTick() >= TIME_MIN_INTERVAL) {
      timeKeeper.tick();
      timeKeeperNRF.tick();

      //Serial.print("MSG ");
      //Serial.print(valueByte);
      //Serial.print(" ");
      //Serial.println(timeKeeper.getTimeTick());

      lock = false;
      clockCounter++;
      inClock = TOCK;

      //Serial.println(tempConter);
      //tempConter=0;
    }
  }

  switch (inByte) {

    //default sequence
    case (CHANGE_MODULE ^ START_MODULE):
      sequenceState = WAIT_START;
      break;
    case (CHANGE_MODULE ^ STOP_MODULE):
      sequenceState = STOP;
      if (DEBUG)Serial.println("STOP");
      break;
    case (CHANGE_MODULE ^ PWM_MODULE):
      if (DEBUG)Serial.println("PWM");
      solenoid_pwm = 255;
      break;
    case (CHANGE_MODULE ^ MIC_MODULE):
      if (DEBUG)Serial.println("MIC");
      solenoid_pwm = 200;
      break;
    case (CHANGE_MODULE ^ DEBUG_MODULE):
      if (DEBUG)Serial.println("DEBUG");
      solenoid_pwm = 150;
      break;
  }


}
