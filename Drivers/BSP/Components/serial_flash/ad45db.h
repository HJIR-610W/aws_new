
#ifndef DS1306_H
#define DS1306_H

#include <stdint.h>

#include "cmsis_os.h"

#include "driver_interface.h"

typedef struct ds1306_cfg_s
{
  void *spi_io;
  void *cs_io;
  void *irq_io;
  
}fm25lc_cfg_t;

driver_t *ds1306_open(void);
void ds1306_init(driver_t *ds1306);


#endif