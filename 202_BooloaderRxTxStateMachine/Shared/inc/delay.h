/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_H
#define __DELAY_H

#include "main.h"

/* Prototypes
 * ------------------------------------------------------------------*/
void SysTick_Setup(void);
uint32_t getTicks(void);
void delayMS(uint32_t ms);
void delayUS(uint32_t us);

#endif /* __DELAY_H */
