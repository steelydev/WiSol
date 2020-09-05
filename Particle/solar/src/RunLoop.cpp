#include "RunLoop.h"

RunLoop::RunLoop()
{
  _timerCount = 0;
}

bool RunLoop::addTimer(RlTimer *t, Immediate fireNow)
{
  if (_timerCount >= MAX_TIMERS)
    return false;

  _timers[_timerCount] = t;
  _timerCount++;
  t->validate();

#ifdef RunLoop_debug
  Serial.println("RunLoop::addTimer {_name:"+t->name()+",_timerCount:" + String(_timerCount) + "}");
#endif

  if (fireNow==FIRE_WHEN_ADDED) {
    t->fire();
  }
  return true;
}

void RunLoop::processTimers()  // run through timers, process, remove invalid timers
{
  for (int i=0; i < _timerCount; i++){
    _timers[i]->processTimer();
  }

  // Delete all invalid timers
  RlTimer *validTimers[MAX_TIMERS];
  int validCount = 0;

  for (int i=0; i < _timerCount; i++) {
    if (_timers[i]->isValid()) {
      validTimers[validCount++] = _timers[i];
    }
  }

  for (int i=0; i < validCount; i++) {
    _timers[i] = validTimers[i];
  }

  _timerCount = validCount;
}

bool RunLoop::invalidate(String name)
{
  for (int i=0; i < _timerCount; i++) {
    if (_timers[i]->name() == name) {
      _timers[i]->invalidate();
      return true;
    }
  }
  return false;
}

bool RunLoop::fireByName(String name)
{
  for (int i=0; i < _timerCount; i++) {
    if (_timers[i]->name() == name) {
      _timers[i]->fire();
      return true;
    }
  }
  return false;
}
