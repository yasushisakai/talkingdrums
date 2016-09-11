#include "Arduino.h"
#include "TimeKeeper.h"


uint8_t  TimeKeeper::signalCount = 0;
uint8_t  TimeKeeper::signalLimit = 3;

TimeKeeper::TimeKeeper() {
  this->currentTime = 0;
  this->timeFrame   = 0;
  this->lastTick    = 0; // last time when '1' came from server
  this->lastHit     = 0;
}

bool static TimeKeeper::wait() {
  bool flag = signalCount >= signalLimit;
  if (!flag) {
    signalCount = 0; // refresh the signal Count
  }
  return flag;
}

void TimeKeeper::cycle(unsigned long t) {
  this->currentTime = t;
}

void TimeKeeper::updateTimeFrame()
{
  this->timeFrame = this->currentTime - this->lastHit;
}

void TimeKeeper::hit() {
  this->lastHit = this->currentTime;
}

void TimeKeeper::tick() {
  this->lastTick = this->currentTime;
}


bool TimeKeeper::checkHit() {
  return (this->timeFrame) < this->interval;
}

bool TimeKeeper::checkTick() {
  return (this->currentTime - this->lastTick) < this->interval;
}

unsigned long TimeKeeper::getTimeFrame(){
  return this->timeFrame;
}





