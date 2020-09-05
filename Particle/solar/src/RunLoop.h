#define RunLoop_debug

#ifndef RunLoop_h
#define RunLoop_h

#include "application.h"
#include "RlTimer.h"

typedef enum {
  FIRE_WHEN_TIME,
  FIRE_WHEN_ADDED
} Immediate;

#define MAX_TIMERS 50

class RunLoop
{

  private:
    RlTimer *_timers[MAX_TIMERS];
    int _timerCount;

  public:
    RunLoop();

    bool addTimer(RlTimer *t,Immediate fireNow); // add a timer to the run loop
    void processTimers(); // go thru timers and fire those that are ready
    bool invalidate(String timerName); // find the timer named and invalidate it
    bool fireByName(String timerName);
};
#endif
