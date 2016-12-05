
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
    static const unsigned long interval = INTERVAL; //ms
    // the number of cycles to wait for aggregate data readings

    unsigned long currentTime;
    unsigned long lastTick;

    unsigned long lastHit;
    unsigned long lastFlash;

  public:
    uint8_t static signalCount;

    // the number of cycles to wait for aggregate data readings
    uint8_t static  signalLimit;

    bool static wait();
    // waits untill signalCount hits the signalLimit
    // resets the signalCount once it does.

    unsigned long timeFrame;

    TimeKeeper();
    void cycle();
    void tick();
    void hit();
    void flash();
    bool checkHit();
    bool checkFlash();
    unsigned long timeFrameChar();
};
#endif