#include "usart.h"
#include "ring_buffer.h"

extern uint8_t dataBuffer[RING_BUFFER_SIZE];
extern ringBuffer_t ringBuffer;

void VirtualCOMPort_Setup(void)
{
    /*USART_3 (USART_C_TX: PD8, USART_C_RX: PD9) has virtual COM port capability*/
    /*Configure USART3*/
    USART_InitTypeDef USART_InitStruct;

    /*Reset every member element of the structure*/
    memset(&USART_InitStruct, 0, sizeof(USART_InitStruct));

    /*Connect GPIOD pins to AF for USART3*/
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

    /*Configure USART3*/
    USART_InitStruct.USART_BaudRate = BAUD_RATE;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    /*Initialize USART3*/
    USART_Init(USART3, &USART_InitStruct);

    /*Enable USART3*/
    USART_Cmd(USART3, ENABLE);

    /*Enable interrupt for UART3*/
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    /*Enable interrupt to UART3*/
    NVIC_EnableIRQ(USART3_IRQn);
}

void USART3_IRQHandler(void)
{
    const bool overrunOccurred = USART_GetITStatus(USART3, USART_IT_ORE) == SET;
    const bool receivedData = USART_GetITStatus(USART3, USART_IT_RXNE) == SET;
    if (overrunOccurred || receivedData)
    {
        if (Ring_Buffer_Write(&ringBuffer, (uint8_t)USART_ReceiveData(USART3)))
        {
            /*handle failure? TBD*/
        }
    }
}

void USART_Write(USART_TypeDef* USARTx, uint8_t* data, const uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        USART_Write_Byte(USARTx, data[i]);
    }
}

void USART_Write_Byte(USART_TypeDef* USARTx, uint8_t data)
{
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) != SET)
        ;
    USART_SendData(USARTx, (uint16_t)data);
}

uint32_t USART_Read(USART_TypeDef* USARTx, uint8_t* data, const uint32_t length)
{
    if (length == 0)
    {
        return 0;
    }

    for (uint32_t numberByteread = 0; numberByteread < length; numberByteread++)
    {
        if (!Ring_Buffer_Read(&ringBuffer, &data[numberByteread]))
        {
            return numberByteread;
        }
    }

    return length;
}

uint8_t USART_Read_Byte(USART_TypeDef* USARTx)
{
    uint8_t byte = 0;
    (void)USART_Read(USART3, &byte, 1);
    return byte;
}

bool USART_Data_Available(USART_TypeDef* USARTx)
{
    return !Ring_Buffer_Empty(&ringBuffer);
}