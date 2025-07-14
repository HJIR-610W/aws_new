
#include "bsp.h"

#include "drv_di.h"
#include "bsp_di.h"


void drv_di_init(void)
{
  bsp_di_init();
}


int32_t drv_di_read(int di_number)
{
  return bsp_di_read(di_number);

}

void  drv_di_set_interrupt(int di_number, di_isr_set_cfg_t *isr_cfg)
{
  bsp_di_set_interrupt(di_number, isr_cfg);
}