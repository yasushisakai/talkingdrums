void setupInterrupt() {
  OCR0A = 0xAF;            // use the same timer as the millis() function
  TIMSK0 |= _BV(OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
  if (sequenceState <= 8) {
    micValue = analogRead(MIC_PIN);
    bandPassFilter.fillWindow(cTime, micValue);
  }
  
}
