
#include "cmsis_os.h"
#include "task_isrEvent.h"
#include "pcb_define.h"
#include "driver_digitalIn.h"

static uint16_t g_rainPulse;

void rainReedCallBack(void *arg)
{
  os_send_isrEvent(eRAIN_REED_INT,0);
}

void rainHallCallBack(void *arg)
{
  os_send_isrEvent(eRAIN_REED_INT,0);
}

void rain_init(void *arg)
{
  driver_t *rain_pulse;
  di_isr_set_cfg_t isr_cfg;

  rain_pulse = driver_di_open(DI_RAIN_REED);

  isr_cfg.call    = rainReedCallBack;
  isr_cfg.name    = "rain_pulse";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio    = 5;

  driver_di_set(rain_pulse,DI_SET_INTERRUT,&isr_cfg);


}
