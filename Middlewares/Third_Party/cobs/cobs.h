

#ifndef COBS_H__
#define COBS_H__




#include <stdint.h>

uint16_t cobs_decode(const uint8_t *input, uint16_t length, uint8_t *output);
int16_t cobs_encode(const uint8_t *input, uint16_t length, uint8_t *output);

#endif