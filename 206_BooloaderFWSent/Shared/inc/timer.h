/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMER_H
#define __TIMER_H

#include "main.h"

/* Structs
 * ------------------------------------------------------------------*/
typedef struct simpleTimer_t {
  uint32_t waitTime;
  uint32_t targetTime;
  bool autoReset;
  bool hasElapsed;
} simpleTimer_t;
/* Prototypes
 * ------------------------------------------------------------------*/
void Simple_Timer_Setup(simpleTimer_t *timer, uint32_t waitTime,
                        bool autoReset);
bool Simple_Timer_Elapsed(simpleTimer_t *timer);
void Simple_Timer_Reset(simpleTimer_t *timer);

#endif /* __TIMER_H */