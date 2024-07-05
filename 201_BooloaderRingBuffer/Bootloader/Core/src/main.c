
#include <stdint.h>

#define BOOTLOADER_SIZE (0x10000U)
#define FLASH_BASE      (0x08000000U)
#define MAIN_APP_START  (FLASH_BASE + BOOTLOADER_SIZE)

static void jump_to_application(void);

/*const uint8_t data[0x10000] = {0};*/

int main(void)
{
    /*volatile uint8_t x = 0;
    for (uint32_t i = 0; i < 0x10000; i++)
    {
        x += data[i];
    }*/

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
