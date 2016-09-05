#include "Arduino.h"
#include "TimeKeeper.h"

TimeKeeper::TimeKeeper() {
  this->currentTime = 0;
  this->timeFrame = 0;
  this->lastTick = 0; // last time when '1' came from server
  this->lastHit = 0;
  this->lastFlash = 0;
}

char static TimeKeeper::signalCount = 0;

bool static TimeKeeper::wait() {
  bool flag = signalCount >= signalLimit;
  if (!flag) {
    TimeKeeper::signalCount = 0; // refresh the signal Count
  }
  return flag;
}

void TimeKeeper::cycle() {
  this->currentTime = millis();
  this->timeFrame = this->currentTime - this->lastTick;
}

void TimeKeeper::tick() {
  this->lastTick = this->currentTime;
}

void TimeKeeper::hit() {
  this->lastHit = this->currentTime;
}

void TimeKeeper::flash() {
  this->lastFlash = this->currentTime;
}

bool TimeKeeper::checkHit() {
  return (this->currentTime - this->lastHit) < this->interval;
}

bool TimeKeeper::checkFlash() {
  return (this->currentTime - this->lastFlash) < this->interval;
}

void TimeKeeper::timeFrameChar() {
  String num = String(3);
  if (this->timeFrame < 100) {
    Serial.print('0');
  }
  Serial.print(this->timeFrame);
}





