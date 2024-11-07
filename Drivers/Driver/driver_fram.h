
#ifndef DRIVER_FRAM_H
#define DRIVER_FRAM_H

#include <stdint.h>

#include "cmsis_os.h"

#include "driver_interface.h"

#define FRAM_FM25LC       0



driver_t * driver_fram_open(int num);
void driver_fram_read(driver_t* driver, uint32_t offset, unsigned char* pBuff, uint16_t rLen);
void driver_fram_write(driver_t* driver, uint32_t offset, unsigned char* pBuff, uint16_t rLen);

#endif