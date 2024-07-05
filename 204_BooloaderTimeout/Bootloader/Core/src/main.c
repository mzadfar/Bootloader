#include "main.h"
#include "bootloaderflash.h"
#include "comms.h"
#include "delay.h"
#include "gpio.h"
#include "rcc.h"
#include "ring_buffer.h"
#include "timer.h"
#include "usart.h"

#define BOOTLOADER_SIZE (0x10000U)
// #define FLASH_BASE      (0x08000000U) /*Defined in stm32f4x.h */
#define MAIN_APP_START (FLASH_BASE + BOOTLOADER_SIZE)

static void jump_to_application(void);

/*Global variables*/
uint8_t dataBuffer[RING_BUFFER_SIZE] = {0U};
ringBuffer_t ringBuffer = {0U};

int main(void)
{
    RCC_Setup();
    //  GPIO_Setup();
    //  VirtualCOMPort_Setup();
    //  Comms_Setup();
    SysTick_Setup();

    // Ring_Buffer_Setup(&ringBuffer, dataBuffer, RING_BUFFER_SIZE);

    simpleTimer_t timer1;
    simpleTimer_t timer2;

    Simple_Timer_Setup(&timer1, 500000, false);
    Simple_Timer_Setup(&timer2, 1000000, true);

    while (true)
    {
        if (Simple_Timer_Elapsed(&timer1))
        {
            volatile int x = 0;
            x++;
        }

        if (Simple_Timer_Elapsed(&timer2))
        {
            Simple_Timer_Reset(&timer1);
        }
    }

    jump_to_application();

    // Never return
    return 0;
}

static void jump_to_application(void)
{
    typedef void (*void_func)(void);

    // uint32_t* reset_vector_entry = (uint32_t*)(MAIN_APP_START + 4U);
    /*1. first address will be SP, and sizeof(uint32_t)) move to next which reset_handler*/
    // uint32_t* reset_vector_entry = (uint32_t*)(MAIN_APP_START + sizeof(uint32_t));
    /*2. another form of representation can be as */
    uint32_t* main_vector_table = (uint32_t*)MAIN_APP_START;

    // uint32_t* reset_vector_entry = &main_vector_table[1];
    // uint32_t* reset_vector = (uint32_t*)(*reset_vector_entry);
    /*3. above 2 lines can be replaced by */
    // uint32_t* reset_vector = (uint32_t*)(main_vector_table[1]);

    // void_func jump_func = (void_func)reset_vector;
    /*4. above 2 lines can be replaced by */
    void_func jump_func = (void_func)main_vector_table[1];

    jump_func();

    /*5. if we had typdef struct vector_table_t which reset or reset handler was part of it,
     * the 3 lines of code above could be written as */
    // vector_table_t* main_vector_table = (vector_table_t*)MAIN_APP_START;
    // main_vector_table->reset();

    /*All above set of code will generate similar assembly code in assembly converted file of
     * *.elf file: arm-none-eabi-objdump -D build/bootloader.elf > asm_dump
    080001ac <main>:
    80001ac:	b580      	push	{r7, lr}
    80001ae:	af00      	add	r7, sp, #0
    80001b0:	f000 f804 	bl	80001bc <jump_to_application>
    80001b4:	2300      	movs	r3, #0
    80001b6:	4618      	mov	r0, r3
    80001b8:	bd80      	pop	{r7, pc}
     */
}
