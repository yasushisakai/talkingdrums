//set the sequence type
void setSequenceState(byte state) 
{
  sequenceState = state;
}

//print the sequence index
void printSequenceIndex() 
{
  Serial.print("L: ");
  Serial.print(sequenceIndex);
  Serial.print(", ");
  Serial.print(bitIndex);
  Serial.print(", ");
  Serial.print(micHit);
  Serial.print(" ");
}

//print the recordings
void printRecordings() {
  for (itri = 0; itri < SEQITER; itri++) {
    Serial.print("L: ");
    Serial.print(itri);
    Serial.print('=');
    for (itrj = 0; itrj < SEQBITS; itrj++) {
      Serial.print(recording[itri][itrj]);
    }
    Serial.println();
  }

  // check sequence if its correct
  bool flag = true;
  for (itri = 0; itri < SEQBITS; itri++) {
    if (debugSequence[itri] != playSequence[itri]) {
      flag = !flag;
      break;
    }
  }
  if (flag) {
    Serial.println("L:sequence correct");
  } else {
    Serial.println("L:sequence incorrect");
  }

  //print the playing sequence
  Serial.print("L: playing=");
  Serial.print(sequenceIndex);
  Serial.print(", ");
}

