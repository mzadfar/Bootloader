#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BOOTLOADER_SIZE (0x10000U)
#define BAUD_RATE       (115200)

uint32_t SystemCoreClock = 168000000;

static void Vector_Setup(void);

void RCC_Setup(void);
void GPIO_Setup(void);
void VirtualCOMPort_Setup(void);

/*USART prototypes*/
void USART_Write(USART_TypeDef* USARTx, uint8_t* data, const uint32_t length);
void USART_Write_Byte(USART_TypeDef* USARTx, uint8_t data);
uint32_t USART_Read(USART_TypeDef* USARTx, uint8_t* data, const uint32_t length);
uint8_t USART_Read_Byte(USART_TypeDef* USARTx);
bool USART_Data_Available(USART_TypeDef* USARTx);

/*Delay prototypes*/
static uint32_t getTicks(void);
void delayMS(uint32_t ms);
void delayUS(uint32_t us);

/*Global variables*/
static uint8_t dataBuffer = 0U;
static bool dataAvailable = false;
int i = 0;
uint32_t n;
uint16_t adcData[2];
float temp;
uint16_t g_nZAx;
volatile uint32_t g_nSysTick;

int main(void)
{
    Vector_Setup(); /*Tell main code here to start from main vector table, and not bootloader
                       vector table*/
    RCC_Setup();
    GPIO_Setup();
    VirtualCOMPort_Setup();

    /*Config global system timer to interrupt every 1us*/
    SysTick_Config(SystemCoreClock / 1000000);

    uint32_t startTime = getTicks();

    while (1)
    {
        if ((getTicks() - startTime) > 1000000)
        {
            GPIO_ToggleBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_7 | GPIO_Pin_14);
            startTime = getTicks();
        }

        // void USART_Write(USART_TypeDef * USARTx, uint8_t * data, const uint32_t length);
        // uint32_t USART_Read(USART_TypeDef * USARTx, uint8_t * data, const uint32_t length);

        if (USART_Data_Available(USART3))
        {
            uint8_t data = USART_Read_Byte(USART3);
            USART_Write_Byte(USART3, data + 1);
        }
    }
    return 0;
}

static void Vector_Setup(void)
{
    SCB->VTOR = BOOTLOADER_SIZE;
}

void SysTick_Handler(void)
{
    g_nSysTick++;
}

static uint32_t getTicks(void)
{
    return g_nSysTick;
}

void delayUS(uint32_t us)
{
    g_nSysTick = 0;
    while (g_nSysTick < us)
    {
        __NOP();
    }
}

void delayMS(uint32_t ms)
{
    g_nSysTick = 0;
    while (g_nSysTick < (ms * 1000))
    {
        __NOP();
    }
}

void RCC_Setup()
{
    /*Initialize GPIOB for toggling LEDs*/
    /*USART_3 (USART_A_TX: PD8, USART_A_RX: PD9)*/
    /* Initiate clock for GPIOB and GPIOD*/
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD, ENABLE);

    /*Initialize USART3 clock*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
}

void GPIO_Setup()
{
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
        dataBuffer = USART_ReceiveData(USART3);
        dataAvailable = true;
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
    if ((length > 0) && dataAvailable)
    {
        *data = dataBuffer;
        dataAvailable = false;
        return 1;
    }
    return 0;
}

uint8_t USART_Read_Byte(USART_TypeDef* USARTx)
{
    dataAvailable = false;
    return dataBuffer;
}

bool USART_Data_Available(USART_TypeDef* USARTx)
{
    return dataAvailable;
}
