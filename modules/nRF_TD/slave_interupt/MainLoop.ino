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

    if (useHeader) {
      setSequenceState(CALIBRATE_MIC);
    } else {
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

  //get if from the bandpass filter
  micHit = bandPassFilter.isHit(micThreshold);

  if (DEBUG) printSequenceIndex();

  if (bitIndex < SEQBITS ) {
    recording[sequenceIndex][bitIndex] = micHit;
  }

  bitIndex++;
  if (bitIndex > SEQBITS) {// >=
    setSequenceState(ANALYZE);
  }

  if (DEBUG) Serial.println(clockCounter);


}

//analyze the for sequence
void analyzeSequence() {
  sequenceIndex++;
  bitIndex = 0;

  if (sequenceIndex < SEQITER) {
    if (DEBUG) Serial.print("WAIT.. ");
    sequenceState = LISTEN_SEQUENCE;
  } else {
    if (DEBUG) Serial.print("FINAL  ");
    //
    // gets the average and defines what it heard to "playSequence"
    //
    for (itri = 0; itri < SEQBITS; itri++) {
      float average = 0.0;
      for (itrj = 0; itrj < SEQITER; itrj++) {
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

    if (useHeader) {
      setSequenceState(HEADER_PLAY);
    } else {
      setSequenceState(PULSE_PLAY);
    }

    //make sure that we are going to play the header
    isHead = true;

    //
    //
    // IMPORTANT!! this line is for the ImageReciever app!!
    //
    //
    writeToReceiver(playSequence);

    //
    // prints the recordings
    //
    if (DEBUG)printRecordings();

  }

  if (DEBUG) Serial.println(clockCounter);
}


//play a pulse dependeing on the microphone readings
void pulsePlay() {
  if (DEBUG) Serial.print(playSequence[bitIndex]);
  if (DEBUG) Serial.print(" ");
  if (DEBUG) Serial.print(bitIndex);
  if (DEBUG) Serial.print("  ");

  if (playSequence[bitIndex] && bitIndex < SEQBITS ) timeKeeper.hit();

  bitIndex++;
  if (bitIndex > SEQBITS) { //==
    bitIndex = 0;
    if (DEBUG) Serial.print("  go to wait play ");
    setSequenceState(WAIT_PLAY);
  }

  if (DEBUG) Serial.println(clockCounter);

}

//wait play function between seequence readings
void waitPlay() {
  bitIndex = 0;
  sequenceIndex++;

  // did it play it for enough times??
  if (sequenceIndex == SEQITER) {
    // yes, proceed to reset

    //if its the slave go to read input instead of reset values

    setSequenceState(RESET);


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


  clockCounter = 0;
  resetCounter = 0;

  if (useHeader) {
    setSequenceState(LISTEN_HEADER);
  } else {
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

