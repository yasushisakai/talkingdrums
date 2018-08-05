//Clock mode 
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


}
