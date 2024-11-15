

#include "driver_digitalIn.h"
#include "cmsis_os.h"
#include "io.h"

osSemaphoreId_t g_rainPulseSem;

const osThreadAttr_t rainTask_attributes = {
  .name = "rainTask",
  .stack_size = 1024,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
  .priority = (osPriority_t) osPriorityNormal,
};



void rainTriggerCallBack(void *arg)
{
  //릴리즈
  osSemaphoreRelease(g_rainPulseSem);
}

void rainTask(void *arg)
{
  driver_t *rain_pulse;
  di_isr_set_cfg_t isr_cfg;


  rain_pulse = driver_di_open(DI_RAIN_REED);

  isr_cfg.call    = rainTriggerCallBack;
  isr_cfg.name    = "rain_pulse";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio    = 5;

  driver_di_set(rain_pulse,DI_SET_INTERRUT,&isr_cfg);


  while(1)
  {
    if(osSemaphoreAcquire(g_rainPulseSem,osWaitForever)==osOK)
    {
      debug_printf("rain_pulse\r\n");
    }
  }
}


void rainTask_init(void)
{
  g_rainPulseSem = osSemaphoreNew(3000, 0, NULL);
  osThreadNew(rainTask, NULL, &rainTask_attributes);
}