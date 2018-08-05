
//test mic
void testMic()
{
  micHit = bandPassFilter.isHit(micThreshold);

  if (micHit == true) {
    //enable solenoid a single hit 30ms
    timeKeeper.hit();
  }
  
}

//Test solenoid
void testDebugSequence()
{
  //repeat three times the header
  //then repeat three times the input sequence.
  micHit = debugSequenceTap[itrj + itri];

  if (micHit == true) {
    timeKeeper.hit();
  }


  //three times the main sequence
  if (itrCounter >= 3) {
    itri++;
    itrj = 3;
    if (itri >= 8) {
      itri = 0;
      itrCounter++;
    }
  }

  //three time the header
  if (itrCounter <= 2) {
    itrj++;
    if (itrj >= 3) {
      itrj = 0;
      itrCounter++;
    }
  }

  //reset the counter
  if (itrCounter >= 6) {
    itri = 0;
    itrj = 0;
    itrCounter = 0;
  }


}

//test solenoid sequence.
void testSolenoid() 
{

  micHit = debugSequenceTap[itri];

  if (micHit == true) {
    timeKeeper.hit();
  }

  itri++;
  if (itri >= 11) {
    itri = 0;
  }

}

//debug times for the timeKeeper
bool debugTimes()
{
  if (DEBUG) {
    Serial.print("T: ");
    Serial.print(timeKeeper.getTimeHit());
    Serial.print(" ");
    Serial.print(timeKeeper.getTimeTick());
    Serial.print(" ");
    Serial.print(timeKeeper.checkHit());
    Serial.print(" ");
    Serial.println(timeKeeper.checkTick());
  }
}

//reset sequence values
void resetSequence() 
{
  //reset sequences
  for (itri = 0; itri < SEQBITS; itri++) {
    playSequence[itri] = false;
    for (itrj = 0; itrj < SEQITER; itrj++) {
      recording[itrj][itri] = false;
    }
  }

  //reset header
  for (itri = 0; itri < numHeaderBits; itri++) {
    headerSequence[itri] = false;
  }

}
