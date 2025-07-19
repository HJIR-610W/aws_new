
#ifndef DRIVER_STM32_I2C_H
#define DRIVER_STM32_I2C_H

#include <stdint.h>

#define STM32_I2C_1 0
#define STM32_I2C_2 1
#define STM32_I2C_MAX 2

int32_t bsp_i2c_init(uint32_t num);
int32_t bsp_i2c_send(int num, uint32_t address,uint8_t reg,const uint8_t *pData,uint16_t dataLen);
int32_t bsp_i2c_read(int num, uint32_t address, uint8_t reg, uint8_t *pData, uint16_t readCnt);
int32_t bsp_i2c_recv_byte(int num, uint8_t address, uint8_t *pBuff, uint32_t readCnt);
int32_t bsp_i2c_send_byte(int num, uint8_t address, uint8_t *pData, uint32_t dataLen);

#endif
