


#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H


#include "driver_interface.h"


#define DRIVER_STM32_I2C_1 0
#define DRIVER_STM32_I2C_2 1

driver_t *driver_i2c_open(uint32_t num);

int32_t driver_i2c_send(driver_t *drv, uint32_t address,uint8_t reg,const uint8_t *pData,uint16_t dataLen);
int32_t driver_i2c_read(driver_t *drv,uint32_t address,uint8_t reg,uint8_t *pData,uint16_t readCnt);
int32_t driver_i2c_recv_byte(driver_t *drv,uint8_t address,uint8_t *pBuff,uint32_t readCnt);
int32_t driver_i2c_send_byte(driver_t *drv,uint8_t address,uint8_t *pData,uint32_t dataLen);


#endif