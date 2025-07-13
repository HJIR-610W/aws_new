

#include "Tasks\task_start.h"

#include "App_drivers\app_file.h"

#include "App_drivers\app_flash.h"

#include "Communications\CDMA\task_cellular.h"
#include "Communications\Direct\task_direct.h"
#include "Communications\Ethernet\task_client.h"
#include "Communications\Ethernet\task_tcpServer.h"
#include "Config\config_app.h"
#include "Config\config_manager.h"
#include "Config\config_sensor.h"
#include "Drivers\Driver\driver_led.h"
#include "Drivers\Driver\driver_rtc.h"
#include "IO\dev_io.h"
#include "bsp_interrupt.h"
#include "bsp.h"
#include "bsp_delay.h"
#include "Tasks\task_aws.h"
#include "Tasks\task_console.h"
#include "Tasks\task_ethernet.h"
#include "Tasks\task_isrEvent.h"
#include "Tasks\task_logging.h"
#include "Tasks\task_measure.h"
#include "Tasks\task_panel.h"
#include "Tasks\task_system.h"
#include "Utils\util_time.h"
#include "Tasks\task_menu.h"
#include "app_dataLogging.h"
#include "app_logging.h"
#include "fatfs.h"
#include "fsmc.h"
#include "lwip.h"
#include "pcb_define.h"
#include "project_def.h"
#include "sdio.h"
#include "test\task_test.h"
#include "user_heap.h"
#include "bsp.h"
#include "task_wdt.h"
#include "task_http_server.h"
#include "task_telnet_server.h"


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

  consoleTask_init(0);//디버깅 printf 사용 해야해서 먼저 초기화 
  wdtTask_init();
  mcu_interrupt_init();  // 최우선 실행

  bsp_init();
  usDelay_init();

  config_manager_init();// 우선 실행 
  
  menuTask_init();
  //flash_init();
  file_init();
  logging_init();

  systemTask_init(PARA_RUN_MODE);

  isrEventTask_init();
  dataLogging_init();
  loggingTask_init();

//  measureTask_init();
//  dualportTask_init();


  if (get_config_app()->cdma_active)
  {
    cellularTask_init();
  }
  
  if (get_config_app()->direct_active)
  {
    directTask_init();
  }

  if (get_config_app()->eth_active)
  {
    if(get_config_app()->eth_mode==eETH_MODE_CLINET)
    {
      tcpClientTask_init();      
    }
    else
    {
      tcpServerTask_init(0);
    }

    ethernetTask_init();
  }

  panelTask_init();

  //http_server_task_init();
  telnet_server_task_init();
  log_boot_reason();
  osThreadExit();  // 종료 시킴
}

void startTask_init(void)
{
  osThreadNew(startTask, NULL, &kStartTask_attributes);
}