
#ifndef DRIVER_FRAM_DEFINE_H
#define DRIVER_FRAM_DEFINE_H

#include "driver_interface.h"
typedef struct fram_api_s
{
  void (*read)(driver_t *driver, uint32_t offset, unsigned char *pBuff, uint16_t rLen);
  void (*write)(driver_t *driver, uint32_t offset, unsigned char *pBuff, uint16_t wLen);
} fram_api_t;


#endif