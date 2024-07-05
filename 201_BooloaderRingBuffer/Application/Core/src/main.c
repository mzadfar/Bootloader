#include "main.h"
#include "delay.h"
#include "gpio.h"
#include "rcc.h"
#include "ring_buffer.h"
#include "usart.h"

#define BOOTLOADER_SIZE (0x10000U)

static void Vector_Setup(void);

/*Global variables*/
uint8_t dataBuffer[RING_BUFFER_SIZE] = {0U};
ringBuffer_t ringBuffer = {0U};

int main(void)
{
    Vector_Setup(); /*Tell main code here to start from main vector table, and not bootloader
                       vector table*/
    RCC_Setup();
    GPIO_Setup();
    VirtualCOMPort_Setup();
    SysTick_Setup();

    Ring_Buffer_Setup(&ringBuffer, dataBuffer, RING_BUFFER_SIZE);

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