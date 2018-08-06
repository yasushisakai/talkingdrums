/*
   Output functions
*/

//output to serial
void writeToReceiver(bool * outputSequence) {
  if (DEBUG) {
    Serial.print("L: r=");
    for (itri = 0; itri < SEQBITS; itri++) {
      Serial.print(outputSequence[itri]);
    }
    Serial.println();
  }
  //
  byte value;
  for (itri = 0; itri < SEQBITS; itri++) {
    value |= outputSequence[itri] << itri;
  }

  if(DEBUG){
    Serial.print("value Receiver ");
    Serial.println(value);
  }
  
  //print the final output
  byte receiverValue[3] = {WRITE_BYTE, value, FINAL_BYTE};
  Serial.write(receiverValue, 3);
}


