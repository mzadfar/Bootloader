#include "delay.h"
#include "main.h"

volatile uint32_t g_nSysTick;

void SysTick_Setup(void) {
  /*Config global system timer to interrupt every 1us*/
  SysTick_Config(SystemCoreClock / 1000000);
}

void SysTick_Handler(void) { g_nSysTick++; }

uint32_t getTicks(void) { return g_nSysTick; }

void delayUS(uint32_t us) {
  g_nSysTick = 0;
  while (g_nSysTick < us) {
    __NOP();
  }
}

void delayMS(uint32_t ms) {
  g_nSysTick = 0;
  while (g_nSysTick < (ms * 1000)) {
    __NOP();
  }
}