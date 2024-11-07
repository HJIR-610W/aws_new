
#ifndef FM25CL_H
#define FM25CL_H

#include <stdint.h>

#include "cmsis_os.h"

#include "driver_interface.h"

typedef struct fm25lc_cfg_s
{
  void *spi_io;
  void *cs_io;
  void *sem;
}fm25lc_cfg_t;

driver_t *fm25lc_open(void);
void fm25cl_init(driver_t *fm25cl);
void fm25cl_write(driver_t *fm25cl,uint32_t offset,uint8_t *pData,uint16_t wLen);
void fm25cl_read(driver_t *fm25cl,uint32_t offset,uint8_t *pBuff,uint16_t rLen);

#endif