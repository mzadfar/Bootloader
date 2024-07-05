#include "bootloaderflash.h"

void Bootloader_FLASH_Erase_Main_App(void)
{
    FLASH_Unlock();

    while (FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3) != FLASH_COMPLETE)
        ;
    while (FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3) != FLASH_COMPLETE)
        ;
    while (FLASH_EraseSector(FLASH_Sector_6, VoltageRange_3) != FLASH_COMPLETE)
        ;
    while (FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3) != FLASH_COMPLETE)
        ;
    while (FLASH_EraseSector(FLASH_Sector_8, VoltageRange_3) != FLASH_COMPLETE)
        ;
    while (FLASH_EraseSector(FLASH_Sector_9, VoltageRange_3) != FLASH_COMPLETE)
        ;

    FLASH_Lock();
}
void Bootloader_FLASH_Write(const uint32_t address, const uint8_t* data, const uint32_t length)
{
    FLASH_Unlock();
    for (uint32_t i = 0; i < length; i++)
    {
        while (FLASH_ProgramByte(address + i, data[i]) != FLASH_COMPLETE)
            ;
    }
    FLASH_Lock();
}