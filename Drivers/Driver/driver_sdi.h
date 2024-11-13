

#ifndef DRIVER_SDI_H
#define DRIVER_SDI_H

#include "driver_interface.h"


#define SDI_1 0


driver_t *driver_sdi_open(uint32_t num);
void driver_sdi_sends(driver_t *drv,uint8_t *pData,uint16_t dataLen);
uint16_t driver_sdi_recv(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms);
#endif