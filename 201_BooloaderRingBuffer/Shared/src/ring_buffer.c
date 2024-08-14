#include "ring_buffer.h"

void Ring_Buffer_Setup(ringBuffer_t *ringBuffer, uint8_t *buffer,
                       uint32_t size) {
  ringBuffer->buffer = buffer;
  ringBuffer->readIndex = 0;
  ringBuffer->writeIndex = 0;
  ringBuffer->mask = size - 1; // size is a power of 2
}

bool Ring_Buffer_Empty(ringBuffer_t *ringBuffer) {
  return ringBuffer->readIndex == ringBuffer->writeIndex;
}

bool Ring_Buffer_Write(ringBuffer_t *ringBuffer, uint8_t byte) {
  uint32_t localReadIndex = ringBuffer->readIndex;
  uint32_t localWriteIndex = ringBuffer->writeIndex;

  uint32_t nextWriteIndex = (localWriteIndex + 1) & ringBuffer->mask;

  if (nextWriteIndex == localReadIndex) {
    return false; // we write on not-used/read index
  }

  ringBuffer->buffer[localWriteIndex] = byte;
  ringBuffer->writeIndex = nextWriteIndex;

  return true;
}

bool Ring_Buffer_Read(ringBuffer_t *ringBuffer, uint8_t *byte) {
  uint32_t localReadIndex = ringBuffer->readIndex;
  uint32_t localWriteIndex = ringBuffer->writeIndex;

  if (localWriteIndex == localReadIndex) {
    return false;
  }

  *byte = ringBuffer->buffer[localReadIndex];
  localReadIndex = (localReadIndex + 1) & ringBuffer->mask;
  ringBuffer->readIndex = localReadIndex;

  return true;
}