
#ifndef DS1306_H
#define DS1360_H


#include <stdint.h>

#include "cmsis_os.h"

#include "driver_interface.h"
#include "time_define.h"


typedef struct ds1306_cfg_s
{
  void *spi_io;
  void *cs_io;
  void *irq_io;
  void *sem;
}ds1306_cfg_t;

driver_t *ds1306_open(void);
void ds1306_init(driver_t *ds1306);
void ds1306_read(driver_t *ds1306,DATE_TIME_BUF *t);

#endif

