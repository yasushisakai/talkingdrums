void clockMode(uint8_t inByte) {

  if (bitRead(inByte, 0) == 1) {
    if (timeKeeper.getTimeTick() >= TIME_MIN_INTERVAL) {
      timeKeeper.tick();
      timeKeeperNRF.tick();

      /*Serial.print("MSG ");
        Serial.print(valueByte);
        Serial.print(" ");
        Serial.println(timeKeeper.getTimeTick());
      */
      lock = false;
      clockCounter++;
      valueByte = TOCK;

      //Serial.println(tempConter);
      //tempConter=0;
    }
  }
  
  switch (inByte) {
    case TICK:
      break;
    case MIC:
      sequenceState = TEST_MIC;
      break;
    case STOP:
      sequenceState = STOP
      break;
    case PWM255:
      break;
    case PWM200:
      break;
    case PWM150:
      break;
    case PWM50:
      break;
    case PWM0:
      break;
  }


}
