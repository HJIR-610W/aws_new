

#ifndef DRIVER_FLASH_H
#define DRIVER_FLASH_H


#include "cmsis_os.h"

#include "driver_interface.h"
#include "driver_flash_define.h"
#define FALSH_AT45DB 0

driver_t * driver_flash_open(int num);
void driver_flash_read(driver_t *drv, uint32_t offset, uint8_t* pBuff,uint32_t buffSize, uint32_t readLen);
int32_t driver_flash_write(driver_t *drv, uint32_t offset, uint8_t* pData, uint32_t dataLen);

void driver_flash_ctrl(driver_t *driver, eFLASH_CTRL_t ctrl, void *w_opt, void *r_opt,
                       uint8_t *err);
#endif