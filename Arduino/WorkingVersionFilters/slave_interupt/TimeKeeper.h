
#ifndef TimeKeeper_h
#define TimeKeeper_h

/*
  Time Keeper

  this class takes care of time keeping the whole cycle and timing the outputs
  hit() and flash() sets the new timer, were check gives a bool whether it should be
  in HIGH(true) or LOW(false)
*/

#include "Arduino.h"
#include "define.h"

class TimeKeeper {
  private:
    unsigned long interval; //ms
    // the number of cycles to wait for aggregate data readings

    unsigned long currentTime;
    
    unsigned long lastHit;
    unsigned long lastTick;

  public:
    uint8_t static signalCount;

    // the number of cycles to wait for aggregate data readings
    uint8_t static  signalLimit;

    bool static wait();
    // waits untill signalCount hits the signalLimit
    // resets the signalCount once it does.

    unsigned long timeTick;
    unsigned long timeHit;

    TimeKeeper();

    void setInterval(unsigned long duration);
    
    void cycle(unsigned long t);
    void updateTimes();
    
    void hit();
    void tick();
    
    bool checkHit();
    bool checkTick();

    bool isTick();

    void resetHit();
    
    unsigned long getTimeHit();
    unsigned long getTimeTick();
};
#endif
