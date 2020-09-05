#include "RlTimer.h"

const unsigned char base=10;

RlTimer::RlTimer()
{
}

RlTimer::RlTimer(String name, timer_type handler, time_t timeInterval, int repeats)
{
  _name = name;
  _handler = handler;
  _timeInterval = timeInterval;
  _fireDate = Time.now() + timeInterval;
  _repeats = repeats;
  _valid = true;
  _stopped = false;
}

RlTimer::RlTimer(String name, timer_type handler, time_t fireDate, time_t timeInterval, int repeats)
{
  _name = name;
  _handler = handler;
  _timeInterval = timeInterval;
  _fireDate = fireDate;
  _repeats = repeats;
  _valid = true;
  _stopped = false;
}

void RlTimer::fire()
{
#ifdef RlTimer_debug
   Serial.println("RlTimer::fire() {_name:" + name() + "}");
#endif
   (*_handler)();
}

void RlTimer::processTimer()
{
#ifdef RlTimer_debug
  Serial.println("Timer::processTimer() {_name:"+_name+"_valid:"+String(_valid)+",_fireDate:"+String(_fireDate)+",Time.now():"+String(Time.now())+"}");
#endif
  if (_stopped) return;

  if (_valid && Time.now() >= _fireDate) {
    this->fire();

#ifdef RlTimer_debug
    Serial.println("Timer::processTimer() {_repeats:"+String(_repeats)+",REPEATS:"+String(REPEATS)+"}");
#endif
    if (_repeats > 0 || _repeats==REPEATS) {
      if (_repeats != REPEATS) _repeats--;
      _fireDate = Time.now() + _timeInterval;
#ifdef RlTimer_debug
      Serial.println("Timer::processTimer() update timer: {_fireDate:"+String(_fireDate)+",Time.now():"+String(Time.now())+"}");
#endif
    } else {
      _valid = false;
    }
  }
}

void RlTimer::validate() {
  _valid = true;
}

void RlTimer::invalidate() {
  _valid = false;
}

bool RlTimer::isValid() {
  return _valid;
}

String RlTimer::name() {
  return _name;
}

void RlTimer::stop() {
  _stopped = true;
}

bool RlTimer::stopped() {
  return _stopped;
}

void RlTimer::reset(time_t newTimeInterval, int repeats) {
  if (newTimeInterval > 0) {
    _timeInterval = newTimeInterval;
  }
  _fireDate = Time.now() + _timeInterval;
  _repeats = repeats;
  _stopped = false;
}

time_t RlTimer::makeInterval(int seconds, int minutes, int hours, int days) {
  time_t interval = seconds;
  interval += minutes * 60;
  interval += hours * 3600;
  interval += days * 86400;
  return interval;
}


String RlTimer::status() {
  String s = "{name:{name}, valid:{valid}, stopped:{stopped}, fireDate:{fireDate}, timeInterval:{timeInterval}, repeats:{repeats}}";
  s.replace("{name}",_name);
  //if (_valid) s.replace("{valid}","true"); else s.replace("{valid}","false");
  //if (_stopped) s.replace("{stopped}","true"); else s.replace("{stopped}","false");

  s.replace("{valid}",String(_valid));
  s.replace("{stopped}",String(_stopped));
  s.replace("{fireDate}",String((double)_fireDate,10));
  s.replace("{timeInterval}",String((double)_timeInterval,10));
  s.replace("{repeats}",String(_repeats));
  return s;

}
