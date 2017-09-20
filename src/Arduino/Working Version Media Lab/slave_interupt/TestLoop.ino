
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
void resetSequence() {
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
