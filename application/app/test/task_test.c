
#include "app_adc.h"
#include "app_file.h"

#include "bsp.h"
#include "bsp.h"
#include "bsp_delay.h"
#include "bsp_interrupt.h"
#include "cmsis_os2.h"
#include "config_manager.h"
#include "drv_di.h"
#include "os_user_def.h"
#include "system_err.h"
#include "task_console.h"
#include "task_system.h"
#include "task_wdt.h"

const osThreadAttr_t kTestTask_attributes = {
    .name = "test_task",
    .stack_size = TASK_STACK(TASK_TEST_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_TEST_DEF),
};

void testTask(void *arg)
{
  bsp_interrupt_init();  // 최우선 실행
  consoleTask_init((void *)1);
  osDelay(1000);

  systemTask_init(PARA_TEST_MODE);
  config_manager_init();
  
  filesystem_init();

  osThreadExit();  // 종료 시킴
}



void testTask_init(void)
{

  osThreadNew(testTask, NULL, &kTestTask_attributes);
#if IWDG_USE
        // iwdt우선순위는 가장 낮게 하여 가장 마지막에 실행 되도록 한다.
        // 이유는 초기화 과정중 이더넷이  3초이상 소요되기 때문
        iwdtTask_init();
#endif



}
