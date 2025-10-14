

#include "tasks\task_start.h"

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "app_file.h"
#include "application\io\dev_io.h"
#include "application\app\tasks\task_menu.h"
#include "application\app\tasks\logging\app_dataLogging.h"
#include "application\app\tasks\logging\app_logging.h"
#include "application\file_system\fatfs\app\fatfs.h"
#include "app_drivers\app_flash.h"
#include "communications\CDMA\task_cellular.h"
#include "communications\Direct\task_direct.h"
#include "communications\Ethernet\task_client.h"
#include "communications\Ethernet\task_tcpServer.h"
#include "config\config_app.h"
#include "config\config_manager.h"
#include "config\config_sensor.h"
#include "drivers\driver\driver.h"
#include "drivers\driver\drv_rtc.h"
#include "drivers\driver\drv_led.h"
#include "drivers\bsp\cubemx\sdio.h"
#include "drivers\bsp\cubemx\fsmc.h"
#include "drivers\bsp\bsp_interrupt.h"
#include "drivers\bsp\bsp.h"
#include "drivers\bsp\bsp_delay.h"
#include "tasks\task_aws.h"
#include "tasks\task_console.h"
#include "tasks\task_ethernet.h"
#include "tasks\task_isrEvent.h"
#include "tasks\task_logging.h"
#include "tasks\task_measure.h"
#include "tasks\task_panel.h"
#include "tasks\task_key.h"
#include "tasks\task_system.h"
#include "utils\util_time.h"

#include "lwip.h"
#include "pcb_define.h"
#include "project_def.h"



#include "test\task_test.h"
#include "user_heap.h"
#include "bsp.h"
#include "task_wdt.h"
#include "task_http_server.h"
#include "task_telnet_server.h"


const osThreadAttr_t kStartTask_attributes = {
    .name = "startTask",
    .stack_size = TASK_STACK(TASK_START_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_START_DEF),
};

void log_boot_reason(void)
{
  uint32_t csr = RCC->CSR;
  char buffer[64];

  if (csr & RCC_CSR_LPWRRSTF)
      log_printf(L_INFO,"Boot: LPWR Reset");
  else if (csr & RCC_CSR_WWDGRSTF)
      log_printf(L_INFO,"Boot: WWDG Reset");
  else if (csr & RCC_CSR_IWDGRSTF)
      log_printf(L_INFO,"Boot: IWDG Reset");
  else if (csr & RCC_CSR_SFTRSTF)
      log_printf(L_INFO,"Boot: SW Reset");
  else if (csr & RCC_CSR_PORRSTF)
      log_printf(L_INFO,"Boot: POR/PDR Reset");
  else if (csr & RCC_CSR_PINRSTF)
      log_printf(L_INFO,"Boot: NRST Pin");
  else if (csr & RCC_CSR_BORRSTF)
      log_printf(L_INFO,"Boot: BOR Reset");
  else
      log_printf(L_INFO,"Boot: Unknown");

  if(restore_error(buffer,sizeof(buffer)))
  {
    log_printf(L_INFO, "%s", buffer);
  }

      // 리셋 플래그 초기화
      RCC->CSR |= RCC_CSR_RMVF;
}


/**
 * @brief 한번 수행하고 종료될 Task
 * 초기화 작업 수행
 * 우선순위는 가장높게 설정
 */
void startTask(void *arg)
{

  drv_init(); // 에플리케이션에서 사용하는 드라이버 초기화
  drv_led_on(DRV_LED_RUN);


  drv_rtc_read(&Date_Time);
  keyTask_init();
  if (testTask_init() == true)
  {
    test_menu_info();
    osThreadExit(); // 종료 시킴
  }

  consoleTask_init(0);//디버깅 printf 사용 해야해서 먼저 초기화
  #ifdef USE_DEBUG
  wdtTask_init();
  #else
  iwdtTask_init();
  #endif


  menuTask_init();

  config_manager_init();  // 우선 실행
  filesystem_init();//SD카드 초기화 및 파일시스템 초 기화 
  logging_init();//운영 로그 기록 기능 초기화

  systemTask_init(PARA_RUN_MODE);

  isrEventTask_init();
  dataLogging_init();
  loggingTask_init();

  measureTask_init();
  dualportTask_init();


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
    ethernetTask_init();

    if(get_config_app()->eth_mode==eETH_MODE_CLINET)
    {
      tcpClientTask_init();      
    }
    else
    {
      tcpServerTask_init(0);
    }

  }
  else
  {
    ethernet_powerdown();//383->334mA
  }

  if(get_config_app()->panel_model != ePANEL_NOT_USED)
  {
    panelTask_init();
  }

  //http_server_task_init();
  telnet_server_task_init();
  log_boot_reason();
  DEBUG_PRINTF("start end\r\n");

  osThreadExit();  // 종료 시킴
}

void startTask_init(void)
{
  osThreadNew(startTask, NULL, &kStartTask_attributes);
}