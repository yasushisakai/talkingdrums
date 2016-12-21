
#ifndef TimeKeeper_h
#define TimeKeeper_h

#include "Arduino.h"
#include "define.h"

class TimeKeeper {
  private:
    static const unsigned long interval=INTERVAL; //ms
    static const char signalLimit=3; // wait for 3 cycles to read
    
    unsigned long currentTime;
    unsigned long lastTick;
    
    unsigned long lastHit;
    unsigned long lastFlash;
    
  public:
    static char signalCount;
    bool static wait(); // waits for this.signalLimit cycles 

    unsigned long timeFrame;
    
    TimeKeeper();
    void cycle();
    void tick();
    void hit();
    void flash();
    bool checkHit();
    bool checkFlash();
    void timeFrameChar();
};


#endif
