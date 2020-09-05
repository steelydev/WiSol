// #define RlTimer_debug

#ifndef RlTimer_h
#define RlTimer_h

#include "application.h"

typedef void(*timer_type )(void);

#define REPEATS -1 // repeat for ever

class RlTimer
{

  private:
    timer_type _handler;
    time_t _timeInterval;
    time_t _fireDate;
    int _repeats; // fire n times
    bool _valid;
    bool _stopped;
    String _name;

  public:

    RlTimer();
    RlTimer(String name, timer_type handler, time_t timeInterval, int repeats); // runs at interval starting now
    RlTimer(String name, timer_type handler, time_t fireDate, time_t timeInterval, int repeats);  // timer runs at interval starting at fire date

    void fire();          // call handler using _name as parameter, don't update fire date
    void processTimer();  // let timer call handler and update fire date if appropriate
    void validate();      // revive timer
    void invalidate();    // kill timer
    bool isValid();       // timer still valid?
    String name();        // return name of timer
    void stop();
    bool stopped();
    void reset(time_t newTimeInterval, int repeats);
    String status();

    static time_t makeInterval(int seconds = 0, int minutes = 0, int hours = 0, int days = 0);
    //time_t makeInterval(int seconds, int hours, int days);
    //time_t makeInterval(int seconds, int hours);
    //time_t makeInterval(int seconds);
};

#endif
