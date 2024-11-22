#include "fw_info.h"
#include "main.h"

__attribute__((section(".fw_info")))

fwInfo_t fwInfo = {
    .sentinel = FW_INFO_SENTINEL,
    .deviceID = DEVICE_ID,
    .version = 0xFFFFFFFF,
    .length = 0xFFFFFFFF,
    /*    .reserved0 = 0xFFFFFFFF,
        .reserved1 = 0xFFFFFFFF,
        .reserved2 = 0xFFFFFFFF,
        .reserved3 = 0xFFFFFFFF,
        .reserved4 = 0xFFFFFFFF,
        .crc32 = 0xFFFFFFFF,
        */
};

__attribute__((section(".fw_signature")))

uint8_t fw_signature[16] = {0}; // AES_BLOCK_SIZE =16 from aes.h