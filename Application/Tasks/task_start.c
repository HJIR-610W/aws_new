#include "task_start.h"

#include "app_dataLogging.h"
#include "app_file.h"
#include "app_flash.h"
#include "app_logging.h"
#include "app_rtc.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "config_manager.h"
#include "dev_io.h"
#include "driver_led.h"
#include "driver_rtc.h"
#include "fatfs.h"
#include "fsmc.h"
#include "lwip.h"
#include "main.h"
#include "mcu_interrupt.h"
#include "mcu_utile.h"
#include "project_def.h"
#include "pcb_define.h"
#include "sdio.h"
#include "task_ble.h"
#include "task_cellular.h"
#include "task_console.h"
#include "task_direct.h"
#include "task_ethernet.h"

#include "task_isrEvent.h"
#include "task_logging.h"
#include "task_measure.h"
#include "task_sdi.h"
#include "task_system.h"
#include "task_tcpServer.h"

#include "dualport.h"

#include "usDelay.h"
#include "user_heap.h"
#include "utile_time.h"
#include "task_test.h"

const osThreadAttr_t kStartTask_attributes = {
    .name = "startTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityRealtime7,
};

void log_boot_reason(void)
{
    uint32_t csr = RCC->CSR;

    if (csr & RCC_CSR_LPWRRSTF)
        log_printf(L_INFO,"Boot: LPWR reset");
    else if (csr & RCC_CSR_WWDGRSTF)
        log_printf(L_INFO,"Boot: WWDG reset");
    else if (csr & RCC_CSR_IWDGRSTF)
        log_printf(L_INFO,"Boot: IWDG reset");
    else if (csr & RCC_CSR_SFTRSTF)
        log_printf(L_INFO,"Boot: SW reset");
    else if (csr & RCC_CSR_PORRSTF)
        log_printf(L_INFO,"Boot: POR/PDR reset");
    else if (csr & RCC_CSR_PINRSTF)
        log_printf(L_INFO,"Boot: NRST pin");
    else if (csr & RCC_CSR_BORRSTF)
        log_printf(L_INFO,"Boot: BOR reset");
    else
        log_printf(L_INFO,"Boot: unknown");

    // 리셋 플래그 초기화
    RCC->CSR |= RCC_CSR_RMVF;
}


/**
 * @brief 한번 수행하고 종료될 Task
 * 초기화 작업 수행
 * 우선순위는 가장높게 설정하여 startTask가 종료되기까지 다른 Task가
 * 실행 되지 않도록 함
 */
void startTask(void *arg)
{
  if(testTask_init()==true)
  {
    osThreadExit();  // 종료 시킴
  }

  mcu_interrupt_init();  // 최우선 실행

  osDelay(1000);
  usDelay_init();
  rtc_init();

  config_manager_init();
  flash_init();

  systemTask_init();
  consoleTask_init(0);
  isrEventTask_init();
  dataLogging_init();
  loggingTask_init();

  dualportTask_init();
  measureTask_init();

  if (get_config_app()->cdma_use)
  {
    cellularTask_init();
  }
  if (get_config_app()->direct_use)
  {
    directTask_init();
  }

  if (get_config_app()->eth_use)
  {
    tcpServerTask_init(0);
    ethernetTask_init();
  }


  // sdiTask_init();

  file_init();

  logging_init();
  bleTask_init();


  log_boot_reason();
  osThreadExit();  // 종료 시킴
}

void startTask_init(void)
{
  osThreadNew(startTask, NULL, &kStartTask_attributes);
}