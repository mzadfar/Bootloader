/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMS_H
#define __COMMS_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define PACKET_DATA_LENGTH  (16)
#define PACKET_LENGTH_BYTES (1)
#define PACKET_CRC_BYTES    (1)
#define PACKET_LENGTH       (PACKET_DATA_LENGTH + PACKET_LENGTH_BYTES + PACKET_CRC_BYTES)

#define PACKET_RETRANSMIT_DATA0  (0x19) /*An arbitrary set value*/
#define PACKET_ACKNOWLEDGE_DATA0 (0x15) /*An arbitrary set value*/

#define PACKET_BUFFER_LENGTH (8)

#define BOOTLOADER_PACKET_SYNC_OBSERVED_DATA0      (0x20)
#define BOOTLOADER_PACKET_FW_UPDATE_REQUEST_DATA0  (0x31)
#define BOOTLOADER_PACKET_FW_UPDATE_RESPONSE_DATA0 (0x37)
#define BOOTLOADER_PACKET_DEVICE_ID_REQUEST_DATA0  (0x3C)
#define BOOTLOADER_PACKET_DEVICE_ID_RESPONSE_DATA0 (0x3F)
#define BOOTLOADER_PACKET_FW_LENGTH_REQUEST_DATA0  (0x42)
#define BOOTLOADER_PACKET_FW_LENGTH_RESPONSE_DATA0 (0x45)
#define BOOTLOADER_PACKET_FW_READY_FOR_DATA_DATA0  (0x48)
#define BOOTLOADER_PACKET_UPDATE_SUCCESSFUL_DATA0  (0x54)
#define BOOTLOADER_PACKET_NOT_ACKNOWLEDGE_DATA0    (0x59)

typedef struct commsPacket_t
{
    uint8_t length;
    uint8_t data[PACKET_DATA_LENGTH]; /*A packet of max 16 bytes of data plus 1 byte crc and 1
                                         byte length*/
    uint8_t crc;
} commsPacket_t;

void Comms_Setup(void);
void Comms_Update(void);
bool Comms_Packets_Available(void);
void Comms_Write(commsPacket_t* packet);
void Comms_Read(commsPacket_t* packet);
uint8_t Comms_Compute_CRC(commsPacket_t* packet);
bool Comms_Single_Byte_Packet(const commsPacket_t* packet, uint8_t byte);
// void Comms_Packet_Copy(const commsPacket_t* source, commsPacket_t* destination);
void Comms_Create_Single_Byte_Packet(commsPacket_t* packet, uint8_t byte);

#endif /* __COMMS_H */
