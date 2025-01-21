
#ifndef AT45DB_H
#define AT45DB_H

#include <stdint.h>

#include "cmsis_os.h"

#include "driver_interface.h"



typedef struct ad45db_cfg_s
{
  void *spi_io;
  void *cs_io;
 
}at45db_cfg_t;

driver_t *at45db_open(void);
void at45db_init(driver_t *drv);
void at45db_write_page(driver_t *drv,uint32_t WriteAddr, uint8_t *writebuff);
void at45db_read_page(driver_t *drv,uint32_t ReadAddr,uint8_t *readbuff);


#endif