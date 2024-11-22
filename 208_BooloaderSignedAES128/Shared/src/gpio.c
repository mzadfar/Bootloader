#include "gpio.h"
#include "main.h"

void GPIO_Setup() {
  /* Initialize GPIOB*/
  GPIO_InitTypeDef GPIO_InitStruct;

  /*Reset every member element of the structure*/
  memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_7 | GPIO_Pin_14;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Initialize GPIOD*/
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

  GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void GPIO_Teardown(void) {
  /* Initialize GPIOB*/
  GPIO_InitTypeDef GPIO_InitStruct;

  /*Reset every member element of the structure*/
  memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_7 | GPIO_Pin_14;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;

  GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Initialize GPIOD*/
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;

  GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* Deinitialize GPIOB and GPIOD*/
  GPIO_DeInit(GPIOB);
  GPIO_DeInit(GPIOD);
}
