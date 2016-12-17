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
    case START:
      sequenceState = WAIT_START;
      break;
    case MIC:
      sequenceState = TEST_MIC;
      if (DEBUG)Serial.println("TEST_MIC");
      break;
    case STOP:
      sequenceState = STOP;
      if (DEBUG)Serial.println("STOP");
      break;
    case PWM255:
      if (DEBUG)Serial.println("PWM 255");
      solenoid_pwm = 255;
      break;
    case PWM200:
      if (DEBUG)Serial.println("PWM 200");
      solenoid_pwm = 200;
      break;
    case PWM150:
      if (DEBUG)Serial.println("PWM 150");
      solenoid_pwm = 150;
      break;
    case PWM50:
      if (DEBUG)Serial.println("PWM 50");
      solenoid_pwm = 50;
      break;
    case PWM0:
      if (DEBUG)Serial.println("PWM 0");
      solenoid_pwm = 0;
      break;
    case DUBUG_C:
      DEBUG = !DEBUG;
      Serial.println("DEBUG");
      break;
  }


}
