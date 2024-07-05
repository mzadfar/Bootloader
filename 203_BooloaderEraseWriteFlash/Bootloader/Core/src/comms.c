#include "comms.h"
#include "crc.h"
#include "usart.h"

typedef enum commsState_t
{
    commsStateLength,
    commsStateData,
    commsStateCRC,
} commsState_t;

static commsState_t state = commsStateLength;
static uint8_t dataByteCount = 0;

static commsPacket_t tempPacket = {.length = 0, .data = {0}, .crc = 0};
static commsPacket_t retransmitPacket = {.length = 0, .data = {0}, .crc = 0};
static commsPacket_t acknowledgePacket = {.length = 0, .data = {0}, .crc = 0};
static commsPacket_t lastTransmittedPacket = {.length = 0, .data = {0}, .crc = 0};

static commsPacket_t packetBuffer[PACKET_BUFFER_LENGTH];
static uint32_t packetReadIndex = 0;
static uint32_t packetWriteIndex = 0;
static uint32_t packetBufferMask = PACKET_BUFFER_LENGTH - 1;

void Comms_Setup(void)
{
    retransmitPacket.length = PACKET_LENGTH_BYTES;
    retransmitPacket.data[0] = PACKET_RETRANSMIT_DATA0;
    for (uint8_t i = PACKET_LENGTH_BYTES; i < PACKET_DATA_LENGTH; i++)
    {
        retransmitPacket.data[i] = 0xFF;
    }
    retransmitPacket.crc = Comms_Compute_CRC(&retransmitPacket);

    acknowledgePacket.length = PACKET_LENGTH_BYTES;
    acknowledgePacket.data[0] = PACKET_ACKNOWLEDGE_DATA0;
    for (uint8_t i = PACKET_LENGTH_BYTES; i < PACKET_DATA_LENGTH; i++)
    {
        acknowledgePacket.data[i] = 0xFF;
    }
    acknowledgePacket.crc = Comms_Compute_CRC(&acknowledgePacket);
}
void Comms_Update(void)
{
    while (USART_Data_Available(USART3))
    {
        switch (state)
        {
        case commsStateLength:
        {
            tempPacket.length = USART_Read_Byte(USART3);
            state = commsStateData;
        }
        break;
        case commsStateData:
        {
            tempPacket.data[dataByteCount++] = USART_Read_Byte(USART3);
            if (dataByteCount >= PACKET_DATA_LENGTH)
            {
                dataByteCount = 0;
                state = commsStateCRC;
            }
        }
        break;
        case commsStateCRC:
        {
            tempPacket.crc = USART_Read_Byte(USART3);

            if (tempPacket.crc != Comms_Compute_CRC(&tempPacket))
            {
                Comms_Write(&retransmitPacket);
                state = commsStateLength;
                break;
            }

            if (Comms_Single_Byte_Packet(&tempPacket, PACKET_RETRANSMIT_DATA0))
            {
                Comms_Write(&lastTransmittedPacket);
                state = commsStateLength;
                break;
            }

            if (Comms_Single_Byte_Packet(&tempPacket, PACKET_ACKNOWLEDGE_DATA0))
            {
                state = commsStateLength;
                break;
            }

            uint32_t nextWriteIndex = (packetWriteIndex + 1) & packetBufferMask;
            if (nextWriteIndex == packetReadIndex)
            {
                __asm__("BKPT #0");
            }
            // assert(nextWriteIndex != packetReadIndex);
            Comms_Packet_Copy(&tempPacket, &packetBuffer[packetWriteIndex]);
            packetWriteIndex = nextWriteIndex;
            Comms_Write(&acknowledgePacket);
            state = commsStateLength;
        }
        break;

        default:
        {
            state = commsStateLength;
        }
        }
    }
}

bool Comms_Packets_Available(void)
{
    return packetReadIndex != packetWriteIndex;
}

void Comms_Write(commsPacket_t* packet)
{
    USART_Write(USART3, (uint8_t*)packet, PACKET_LENGTH);
    Comms_Packet_Copy(packet, &lastTransmittedPacket);
}

void Comms_Read(commsPacket_t* packet)
{
    Comms_Packet_Copy(&packetBuffer[packetReadIndex], packet);
    packetReadIndex = (packetReadIndex + 1) & packetBufferMask;
}

uint8_t Comms_Compute_CRC(commsPacket_t* packet)
{
    return crc8((uint8_t*)packet, PACKET_LENGTH - PACKET_CRC_BYTES);
}

bool Comms_Single_Byte_Packet(const commsPacket_t* packet, uint8_t byte)
{
    if (packet->length != PACKET_LENGTH_BYTES)
    {
        return false;
    }

    if (packet->data[0] != byte)
    {
        return false;
    }

    for (uint8_t i = PACKET_LENGTH_BYTES; i < PACKET_DATA_LENGTH; i++)
    {
        if (packet->data[i] != 0xFF)
        {
            return false;
        }
    }

    return true;
}

void Comms_Packet_Copy(const commsPacket_t* source, commsPacket_t* destination)
{
    destination->length = source->length;
    for (uint8_t i = 0; i < PACKET_DATA_LENGTH; i++)
    {
        destination->data[i] = source->data[i];
    }

    destination->crc = source->crc;
}