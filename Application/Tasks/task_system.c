#include "task_system.h"

#include "bsp.h"

#include "app_charger.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "driver_di.h"
#include "driver_do.h"
#include "driver_uart.h"
#include "task_isrEvent.h"


const osThreadAttr_t kSystemTask_attributes = {
    .name = "systemTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

void userBtnCallBack(void *arg)
{ 
  os_send_isrEvent(eUSER_BTN_INT, 0); 
}

void userBtn_init(void)
{
  driver_t *user_btn;
  di_isr_set_cfg_t isr_cfg;

  user_btn = driver_di_open(DI_USER_BTN, 0);

  isr_cfg.call = userBtnCallBack;
  isr_cfg.name = "user_btn";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio = 5;

  driver_di_set(user_btn, DI_SET_INTERRUPT, &isr_cfg);
}

void systemTask(void *arg)
{
  uint8_t err=0;
  uint32_t start_time = osKernelGetTickCount();
  
  while (1)
  {
    bsp_rtc_update();

    if ((osKernelGetTickCount() - start_time)>1000)
    {
      start_time = osKernelGetTickCount();
      System.door_opened = bsp_door_opened();
      update_charger();
      System.battery_error = read_batteryVoltage1(&err) < 10.0f?1:0;
      System.ac_status = 1;//220v
      System.dc_error = bsp_read_battery()<11.0f?1:0;
    }

    osDelay(500);
  }
}




void systemTask_init(uint32_t para)
{
  if(para==PARA_RUN_MODE)
  {
    userBtn_init();
    
    charger_init(get_config_app()->charger_model);

  }

  osThreadNew(systemTask, NULL, &kSystemTask_attributes);
}