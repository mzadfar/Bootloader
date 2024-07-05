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
#define FLASH_BASE1     (0x08000000U) /*Defined in stm32f4x.h */
#define MAIN_APP_START  (FLASH_BASE1 + BOOTLOADER_SIZE)
#define MAX_FW_LENGTH   ((1024U * 2048U) - BOOTLOADER_SIZE)

#define DEVICE_ID (0x42)

#define SYNC_SEQ_0 (0xC4)
#define SYNC_SEQ_1 (0x55)
#define SYNC_SEQ_2 (0x7E)
#define SYNC_SEQ_3 (0x10)

#define DEFAULT_TIMEOUT (500000)

uint32_t SystemCoreClock = 168000000;

typedef enum bootloaderState_t
{
    bootloaderSyncState,
    bootloaderWaitUpdareReqState,
    bootloaderDeviceIDReqState,
    bootloaderDeviceIDResState,
    bootloaderFWLengthReqState,
    bootloaderFWLengthResState,
    bootloaderEraseAppState,
    bootloaderReceiveFWState,
    bootloaderDoneState,
} bootloaderState_t;

static bootloaderState_t state = bootloaderSyncState;
static uint32_t fwLength = 0;
static uint32_t bytesWritten = 0;
static uint8_t syncSequence[4] = {0};

static simpleTimer_t timer;
static commsPacket_t tempPacket;

static void jump_to_application(void);

/*Global variables*/
uint8_t dataBuffer[RING_BUFFER_SIZE] = {0U};
ringBuffer_t ringBuffer = {0U};

static void Bootloader_Fail(void);
static void Check_Timeout(void);
static bool Device_ID_Packet(const commsPacket_t* packet);
static bool FW_Length_Packet(const commsPacket_t* packet);

int main(void)
{
    SysTick_Setup();
    RCC_Setup();
    GPIO_Setup();
    VirtualCOMPort_Setup();
    Comms_Setup();

    Ring_Buffer_Setup(&ringBuffer, dataBuffer, RING_BUFFER_SIZE);

    Simple_Timer_Setup(&timer, DEFAULT_TIMEOUT, false);

    while (state != bootloaderDoneState)
    {
        if (state == bootloaderSyncState)
        {
            if (USART_Data_Available(USART3))
            {
                syncSequence[0] = syncSequence[1];
                syncSequence[1] = syncSequence[2];
                syncSequence[2] = syncSequence[3];
                syncSequence[3] = USART_Read_Byte(USART3);

                bool isMatch = syncSequence[0] == SYNC_SEQ_0;
                isMatch = isMatch && (syncSequence[1] == SYNC_SEQ_1);
                isMatch = isMatch && (syncSequence[2] == SYNC_SEQ_2);
                isMatch = isMatch && (syncSequence[3] == SYNC_SEQ_3);

                if (isMatch)
                {
                    Comms_Create_Single_Byte_Packet(
                        &tempPacket, BOOTLOADER_PACKET_SYNC_OBSERVED_DATA0);
                    Comms_Write(&tempPacket);
                    Simple_Timer_Reset(&timer);
                    state = bootloaderWaitUpdareReqState;
                }
                else
                {
                    Check_Timeout();
                }
            }
            else
            {
                Check_Timeout();
            }
            continue;
        }

        Comms_Update();

        switch (state)
        {
        case bootloaderWaitUpdareReqState:
        {
            if (Comms_Packets_Available())
            {
                Comms_Read(&tempPacket);
                if (Comms_Single_Byte_Packet(
                        &tempPacket, BOOTLOADER_PACKET_FW_UPDATE_REQUEST_DATA0))
                {
                    Simple_Timer_Reset(&timer);
                    Comms_Create_Single_Byte_Packet(
                        &tempPacket, BOOTLOADER_PACKET_FW_UPDATE_RESPONSE_DATA0);
                    Comms_Write(&tempPacket);
                    state = bootloaderDeviceIDReqState;
                }
                else
                {
                    Bootloader_Fail();
                }
            }
            else
            {
                Check_Timeout();
            }
        }
        break;
        case bootloaderDeviceIDReqState:
        {
            delayMS(500); // Host did not receive BOOTLOADER_PACKET_FW_UPDATE_RESPONSE_DATA0 if
                          // this delay is omitted
            Simple_Timer_Reset(&timer);
            Comms_Create_Single_Byte_Packet(
                &tempPacket, BOOTLOADER_PACKET_DEVICE_ID_REQUEST_DATA0);
            Comms_Write(&tempPacket);
            state = bootloaderDeviceIDResState;
        }
        break;
        case bootloaderDeviceIDResState:
        {
            if (Comms_Packets_Available())
            {
                Comms_Read(&tempPacket);
                if (Device_ID_Packet(&tempPacket) && (tempPacket.data[1] == DEVICE_ID))
                {
                    Simple_Timer_Reset(&timer);
                    state = bootloaderFWLengthReqState;
                }
                else
                {
                    Bootloader_Fail();
                }
            }
            else
            {
                Check_Timeout();
            }
        }
        break;
        case bootloaderFWLengthReqState:
        {
            Simple_Timer_Reset(&timer);
            Comms_Create_Single_Byte_Packet(
                &tempPacket, BOOTLOADER_PACKET_FW_LENGTH_REQUEST_DATA0);
            Comms_Write(&tempPacket);
            state = bootloaderFWLengthResState;
        }
        break;
        case bootloaderFWLengthResState:
        {
            if (Comms_Packets_Available())
            {
                Comms_Read(&tempPacket);
                fwLength =
                    ((tempPacket.data[1]) | (tempPacket.data[2] << 8)
                     | (tempPacket.data[3] << 16) | (tempPacket.data[4] << 24));

                if (FW_Length_Packet(&tempPacket) && (fwLength <= MAX_FW_LENGTH))
                {
                    state = bootloaderEraseAppState;
                }
                else
                {
                    Bootloader_Fail();
                }
            }
            else
            {
                Check_Timeout();
            }
        }
        break;
        case bootloaderEraseAppState:
        {
            Bootloader_FLASH_Erase_Main_App();

            Comms_Create_Single_Byte_Packet(
                &tempPacket, BOOTLOADER_PACKET_FW_READY_FOR_DATA_DATA0);
            Comms_Write(&tempPacket);

            Simple_Timer_Reset(&timer);
            state = bootloaderReceiveFWState;
        }
        break;
        case bootloaderReceiveFWState:
        {
            if (Comms_Packets_Available())
            {
                Comms_Read(&tempPacket);

                const uint8_t packetLength = (tempPacket.length & 0xFF) + 1;
                Bootloader_FLASH_Write(
                    MAIN_APP_START + bytesWritten, tempPacket.data, packetLength);
                bytesWritten += packetLength;
                Simple_Timer_Reset(&timer);

                if (bytesWritten >= fwLength)
                {
                    Comms_Create_Single_Byte_Packet(
                        &tempPacket, BOOTLOADER_PACKET_UPDATE_SUCCESSFUL_DATA0);
                    Comms_Write(&tempPacket);
                    state = bootloaderDoneState;
                }
                else
                {
                    Comms_Create_Single_Byte_Packet(
                        &tempPacket, BOOTLOADER_PACKET_FW_READY_FOR_DATA_DATA0);
                    Comms_Write(&tempPacket);
                }
            }
            else
            {
                Check_Timeout();
            }
        }
        break;

        default:
        {
            state = bootloaderSyncState;
        }
        }
    }

    VirtualCOMPort_Teardown();
    GPIO_Teardown();
    RCC_Teardown();
    Systick_Teardown();

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

    /*5. if we had typdef struct vector_table_t which reset or reset handler was part of
     * it, the 3 lines of code above could be written as */
    // vector_table_t* main_vector_table = (vector_table_t*)MAIN_APP_START;
    // main_vector_table->reset();

    /*All above set of code will generate similar assembly code in assembly converted file
    of
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

static void Bootloader_Fail(void)
{
    Comms_Create_Single_Byte_Packet(&tempPacket, BOOTLOADER_PACKET_NOT_ACKNOWLEDGE_DATA0);
    Comms_Write(&tempPacket);
    state = bootloaderDoneState;
}

static void Check_Timeout(void)
{
    if (Simple_Timer_Elapsed(&timer))
    {
        Bootloader_Fail();
    }
}

static bool Device_ID_Packet(const commsPacket_t* packet)
{
    if (packet->length != 2)
    {
        return false;
    }

    if (packet->data[0] != BOOTLOADER_PACKET_DEVICE_ID_RESPONSE_DATA0)
    {
        return false;
    }

    for (uint8_t i = 2; i < PACKET_DATA_LENGTH; i++)
    {
        if (packet->data[i] != 0xFF)
        {
            return false;
        }
    }

    return true;
}

static bool FW_Length_Packet(const commsPacket_t* packet)
{
    if (packet->length != 5)
    {
        return false;
    }

    if (packet->data[0] != BOOTLOADER_PACKET_FW_LENGTH_RESPONSE_DATA0)
    {
        return false;
    }

    for (uint8_t i = 5; i < PACKET_DATA_LENGTH; i++)
    {
        if (packet->data[i] != 0xFF)
        {
            return false;
        }
    }

    return true;
}