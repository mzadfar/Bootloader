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
void Comms_Packet_Copy(const commsPacket_t* source, commsPacket_t* destination);

#endif /* __COMMS_H */
