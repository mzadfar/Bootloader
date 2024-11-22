/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOOTLOADERFLASH_H
#define __BOOTLOADERFLASH_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

void Bootloader_FLASH_Erase_Main_App(void);
void Bootloader_FLASH_Write(
    const uint32_t address, const uint8_t* data, const uint32_t length);

#endif /* __BOOTLOADERFLASH_H */
