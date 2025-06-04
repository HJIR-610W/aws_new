
#include "MCU\mcu_utile.h"
#include "app_adc.h"
#include "app_bsp.h"
#include "app_file.h"
#include "app_flash.h"
#include "bsp.h"
#include "cmsis_os2.h"
#include "config_manager.h"
#include "driver_di.h"
#include "mcu_interrupt.h"
#include "os_user_def.h"
#include "task_console.h"
#include "task_system.h"
#include "usDelay.h"

const osThreadAttr_t kTestTask_attributes = {
    .name = "test_task",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityRealtime7,
};

void testTask(void *arg)
{
  mcu_interrupt_init();  // 최우선 실행
  consoleTask_init((void *)1);
  osDelay(1000);
  usDelay_init();
  bsp_init();
  app_bsp_init();
  adc_init();
  status_led_set(LED_BLINK);
  systemTask_init(PARA_TEST_MODE);
  config_manager_init();
  flash_init();
  
  file_init();

  osThreadExit();  // 종료 시킴

}


bool testTask_init(void)
{ 
  driver_t *user_btn;
  uint32_t startTime;

  user_btn = driver_di_open(DI_USER_BTN,0);

  //사용자가 5초이상 버튼을 누르면 testTask 실행행
  if(driver_di_is_low(user_btn,1000,10))
  {
    osThreadNew(testTask, NULL, &kTestTask_attributes);
    return true;
  }

  return false;
}