
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
    static const uint8_t signalLimit = 2;
    // the number of cycles to wait for aggregate data readings

    unsigned long currentTime;
    unsigned long lastTick;
    unsigned long lastHit;
    unsigned long lastFlash;
    unsigned long timeFrame;

  public:

    uint8_t static signalCount;

    bool static wait();
    // waits untill signalCount hits the signalLimit
    // resets the signalCount once it does.

    TimeKeeper();
    void cycle(unsigned long & cTime);
    void setInterval(unsigned long const & inter);
    void tick();
    void hit();
    void flash();
    bool checkHit();
    bool checkFlash();
    unsigned long getTimeFrameLimit();

    unsigned long getTimeFrame();
};
#endif
