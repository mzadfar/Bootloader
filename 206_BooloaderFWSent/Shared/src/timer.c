#include "timer.h"
#include "delay.h"

void Simple_Timer_Setup(simpleTimer_t *timer, uint32_t waitTime,
                        bool autoReset) {
  timer->waitTime = waitTime;
  timer->autoReset = autoReset;
  timer->targetTime = waitTime + getTicks();
  timer->hasElapsed = false;
}

bool Simple_Timer_Elapsed(simpleTimer_t *timer) {
  uint32_t nowTime = getTicks();
  bool hasElapsed = nowTime >= timer->targetTime;

  if (timer->hasElapsed) {
    return false;
  }

  if (hasElapsed) {
    if (timer->autoReset) {
      uint32_t driftTime = nowTime - timer->targetTime;
      timer->targetTime = (nowTime + timer->waitTime) - driftTime;
    } else {
      timer->hasElapsed = true;
    }
  }

  return hasElapsed;
}

void Simple_Timer_Reset(simpleTimer_t *timer) {
  Simple_Timer_Setup(timer, timer->waitTime, timer->autoReset);
}