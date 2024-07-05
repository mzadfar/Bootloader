/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FW_INFO_H
#define __FW_INFO_H

#include "main.h"

#define ALIGNED(address, alignment)                                            \
  (((address) - 1U + (alignment)) & -(alignment))
#define FW_INFO_SENTINEL (0xDEADC0DE)
#define BOOTLOADER_SIZE (0x10000U)
// #define FLASH_BASE (0x08000000U) /*Defined in stm32f4x.h */
#define MAIN_APP_START (FLASH_BASE + BOOTLOADER_SIZE)
#define MAX_FW_LENGTH ((1024U * 2048U) - BOOTLOADER_SIZE)
#define DEVICE_ID (0x42)
#define VECTOR_TABLE_SIZE (0x1B0) // sizeof(g_pfnVectors[])
#define FW_INFO_ADDRESS (ALIGNED((MAIN_APP_START + VECTOR_TABLE_SIZE), 16))
/* #define FW_INFO_VALIDATE_ADDRESS (FW_INFO_ADDRESS + sizeof(fwInfo_t))
#define FW_INFO_VALIDATE_LENGTH(fwLength) \
  (fwLength - (VECTOR_TABLE_SIZE + sizeof(fwInfo_t)))*/
#define SIGNATURE_ADDRESS (FW_INFO_ADDRESS + sizeof(fwInfo_t))

/* Struct
 * ------------------------------------------------------------------*/
typedef struct fwInfo_t {
  uint32_t sentinel;
  uint32_t deviceID;
  uint32_t version;
  uint32_t length;
  // uint32_t reserved[5];
  // uint32_t crc32;
} fwInfo_t; // Needed to be multiple of 16 bytes to ease implementation

#endif /* __FW_INFO_H */