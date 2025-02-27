#include "cmsis_os2.h"

#include "app_rtc.h"
#include "app_bsp.h"
#include "driver_do.h"

driver_t *g_test_do;

const osThreadAttr_t kSystemTask_attributes = {
  .name = "systemTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityLow,
};


void systemTask(void *arg)
{
  battery_init();
  while(1)
  {
    rtc_update();
    osDelay(500);
  }
}

void test_do(int out)
{
  if(out)
  {
    driver_do_high(g_test_do);
  }
  else{
    driver_do_low(g_test_do);
  }
}

void test_do_toggle(void)
{
  static int i=0;

  i^=1;
  if(i)
  {
    driver_do_high(g_test_do);
  }
  else{
    driver_do_low(g_test_do);
  }
}

void systemTask_init(void)
{
  g_test_do = driver_do_open(DO_EXT_0,0);

  osThreadNew(systemTask, NULL, &kSystemTask_attributes);
}