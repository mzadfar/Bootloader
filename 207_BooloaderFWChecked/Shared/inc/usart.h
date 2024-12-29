/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H
#define __USART_H

#include "main.h"

#define BAUD_RATE (115200)

/* Prototypes
 * ------------------------------------------------------------------*/
void VirtualCOMPort_Setup(void);
void USART_Write(USART_TypeDef *USARTx, uint8_t *data, const uint32_t length);
void USART_Write_Byte(USART_TypeDef *USARTx, uint8_t data);
uint32_t USART_Read(USART_TypeDef *USARTx, uint8_t *data,
                    const uint32_t length);
uint8_t USART_Read_Byte(USART_TypeDef *USARTx);
bool USART_Data_Available(USART_TypeDef *USARTx);
void VirtualCOMPort_Teardown(void);

#endif /* __USART_H */
