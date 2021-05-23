#ifndef __Circular_buffer_H
#define __Circular_buffer_H
#include <stdint.h>
#include <stdbool.h>
uint16_t getCurentLenght(void);
bool sendByte(uint8_t *date);
uint16_t getFreeSize(void);
uint8_t readByte(void);
#endif//_Circular_buffer_H
