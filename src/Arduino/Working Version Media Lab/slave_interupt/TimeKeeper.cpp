#include "Arduino.h"
#include "TimeKeeper.h"

/*
 * Function for time keeping 
 */


uint8_t  TimeKeeper::signalCount = 0;
uint8_t  TimeKeeper::signalLimit = 4;

TimeKeeper::TimeKeeper() {
  this->currentTime = 0;
  this->timeHit     = 0;
  this->timeTick    = 0;

  this->lastTick    = 0; // last time when '1' came from server
  this->lastHit     = 0;
}


void TimeKeeper::setInterval(unsigned long duration) {
  interval = duration;
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

void TimeKeeper::updateTimes() {
  this->timeTick = this->currentTime - this->lastTick;
  this->timeHit  = this->currentTime - this->lastHit;
}

void TimeKeeper::hit() {
  this->lastHit = this->currentTime;
}

void TimeKeeper::tick() {
  this->lastTick = this->currentTime;
}

bool TimeKeeper::checkHit() {
  return (this->timeHit  < this->interval);
}

bool TimeKeeper::checkTick() {
  return (this->timeTick < this->interval);
}

bool TimeKeeper::isTick() {
  return (this->timeTick > this->interval);
}

unsigned long TimeKeeper::getTimeTick() {
  return this->timeTick;
}

unsigned long TimeKeeper::getTimeHit() {
  return this->timeHit;
}






