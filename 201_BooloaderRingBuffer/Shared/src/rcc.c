#include "rcc.h"
#include "main.h"

void RCC_Setup()
{
    /*Initialize GPIOB for toggling LEDs*/
    /*USART_3 (USART_A_TX: PD8, USART_A_RX: PD9)*/
    /* Initiate clock for GPIOB and GPIOD*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD, ENABLE);

    /*Initialize USART3 clock*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
}