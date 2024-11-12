

#ifndef DRIVER_FLASH_H
#define DRIVER_FLASH_H


#include "cmsis_os.h"

#include "driver_interface.h"

#define FALSH_AT45DB 0


driver_t * driver_flash_open(int num);
void driver_flash_read(driver_t *drv, uint32_t offset, uint8_t* pBuff,uint32_t buffSize, uint32_t readLen);


int32_t driver_flash_write(driver_t *drv, uint32_t offset, uint8_t* pData, uint32_t dataLen);
#endif