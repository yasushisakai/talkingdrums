#include "Arduino.h"
#include "TimeKeeper.h"

uint8_t static TimeKeeper::signalCount = 0;

TimeKeeper::TimeKeeper() {
  this->currentTime = 0L;
  this->timeFrame   = 0L;
  this->lastTick    = 0L; // last time when '1' came from server
  this->lastHit     = 0L;
  this->lastFlash   = 0L;
  this->interval    = 0L;
}


void TimeKeeper::setInterval(unsigned long const & inter)
{
  interval = inter;
}


bool static TimeKeeper::wait() {
  bool flag = signalCount >= signalLimit;
  if (!flag) {
    TimeKeeper::signalCount = 0; // refresh the signal Count
  }
  return flag;
}

void TimeKeeper::cycle(unsigned long & cTime) {
  this->currentTime = cTime;
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

unsigned long TimeKeeper::getTimeFrameLimit() {
  if (this->timeFrame < 100L) {
    return 0L;
  }
  return this->timeFrame;
}

unsigned long TimeKeeper::getTimeFrame() {
  return this->timeFrame;
}





