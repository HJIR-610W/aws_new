

#ifndef AWS_PROTOCOL_H
#define AWS_PROTOCOL_H
#include <stdint.h>
int32_t aws_cmd(uint8_t *input,uint32_t len,uint8_t *txBuff,uint16_t txSize,uint8_t source);
#endif