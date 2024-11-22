/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "main.h"

/*For max 10ms latency for 115200 bits per second (11520 bytes (including
 * start/stop bits) per second): 115200 * 10ms = 115.2 ~ 128*/
#define RING_BUFFER_SIZE (128)

/* Struct ------------------------------------------------------------------*/
typedef struct ringBuffer_t {
  uint8_t *buffer;
  uint32_t mask;
  uint32_t readIndex;
  uint32_t writeIndex;
} ringBuffer_t;

/* Prototypes
 * ------------------------------------------------------------------*/
void Ring_Buffer_Setup(ringBuffer_t *ringBuffer, uint8_t *buffer,
                       uint32_t size);
bool Ring_Buffer_Empty(ringBuffer_t *ringBuffer);
bool Ring_Buffer_Write(ringBuffer_t *ringBuffer, uint8_t byte);
bool Ring_Buffer_Read(ringBuffer_t *ringBuffer, uint8_t *byte);

#endif /* __RING_BUFFER_H */
