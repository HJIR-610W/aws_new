#include "cmsis_os2.h"

#include "app_do.h"
#include "app_rtc.h"
#include "app_bsp.h"
#include "app_di.h"
#include "driver_do.h"
#include "driver_di.h"
#include "task_isrEvent.h"
#include "app_charger.h"

driver_t *g_test_do;

const osThreadAttr_t kSystemTask_attributes = {
  .name = "systemTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

void userBtnCallBack(void *arg)
{
  os_send_isrEvent(eUSER_BTN_INT,0);
}

void userBtn_init(void)
{
  driver_t *user_btn;
  di_isr_set_cfg_t isr_cfg;

  user_btn = driver_di_open(DI_USER_BTN,0);

  isr_cfg.call    = userBtnCallBack;
  isr_cfg.name    = "user_btn";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio    = 5;

  driver_di_set(user_btn,DI_SET_INTERRUPT,&isr_cfg);
}


void systemTask(void *arg)
{

  while(1)
  {
    rtc_update();
    update_charger();

    for(int i = 0 ; i< 6; i++)
    {
      if(IS_DI_PRESSED(i))
      {
        write_do(i,0);
      }
      else
      {
        write_do(i,1);
      }
    }
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

  battery_init();

  userBtn_init();

  charger_init(APP_CHARGER_HJ);

  di_init();

  do_init();

  osThreadNew(systemTask, NULL, &kSystemTask_attributes);
}