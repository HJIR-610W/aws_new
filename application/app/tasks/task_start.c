

#include "tasks\task_start.h"

#include "app_drivers\app_flash.h"
#include "app_file.h"
#include "application\app\tasks\logging\app_dataLogging.h"
#include "application\app\tasks\logging\app_logging.h"
#include "application\app\tasks\task_menu.h"
#include "application\file_system\fatfs\app\fatfs.h"
#include "application\io\dev_io.h"
#include "bsp.h"
#include "cmsis_os2.h"
#include "communications\CDMA\task_cellular.h"
#include "communications\Direct\task_direct.h"
#include "communications\Ethernet\task_client.h"
#include "communications\Ethernet\task_tcpServer.h"
#include "config\config_app.h"
#include "config\config_manager.h"
#include "config\config_sensor.h"
#include "drivers\bsp\bsp.h"
#include "drivers\bsp\bsp_delay.h"
#include "drivers\bsp\bsp_interrupt.h"
#include "drivers\bsp\cubemx\fsmc.h"
#include "drivers\bsp\cubemx\sdio.h"
#include "drivers\driver\driver.h"
#include "drivers\driver\drv_led.h"
#include "drivers\driver\drv_rtc.h"
#include "drivers\driver\drv_di.h"
#include "FreeRTOS.h"
#include "lwip.h"
#include "pcb_define.h"

#include "task_http_server.h"
#include "task_telnet_server.h"
#include "task_wdt.h"
#include "tasks\task_aws.h"
#include "tasks\task_console.h"
#include "tasks\task_ethernet.h"
#include "tasks\task_event.h"
#include "tasks\task_key.h"
#include "tasks\task_logging.h"
#include "tasks\task_measure.h"
#include "tasks\task_panel.h"
#include "tasks\task_system.h"
#include "test\task_test.h"
#include "user_heap.h"
#include "utils\util_time.h"

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


uint8_t check_test_mode(void)
{
  uint8_t count=0;

  if (drv_di_read(DRV_DI_USER_BTN) == 0)
  {
    osDelay(10);
    if (drv_di_read(DRV_DI_USER_BTN) == 0)
    {
      return 1;
    }
  }
  return 0;
}

/**
 * @brief 한번 수행하고 종료될 Task
 * 초기화 작업 수행
 * 우선순위는 가장높게 설정
 */
void startTask(void *arg)
{
  uint8_t test_mode = 0;

  bsp_init();

  test_mode = check_test_mode();



  #if IWDG_USE
  bsp_iwdg_init(16000);//iwdg task가 실행 전까지는 16초로 타임아웃
#endif
  
  drv_init(); // 애플리케이션에서 사용하는 드라이버 초기화

  keyTask_init();//키 task는 테스트 모드, 실행모드 공통 사용

  if(test_mode)
  {
    testTask_init();
    test_menu_info();
    osThreadExit(); // 종료 시킴
  }
  wdtTask_init();
  menuTask_init();//최소 test_mode 다음에 선언하여 이때 부팅화면 출력 
  consoleTask_init((void *)test_mode);//디버깅 printf 사용 해야해서 먼저 초기화
  config_manager_init();  // 우선 실행
  filesystem_init();//SD카드 초기화 및 파일시스템 초 기화 
  logging_init();//운영 로그 기록 기능 초기화

  systemTask_init(PARA_RUN_MODE);

  eventTask_init();
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

#if IWDG_USE
//iwdt우선순위는 가장 낮게 하여 가장 마지막에 실행 되도록 한다.
//이유는 초기화 과정중 이더넷이  3초이상 소요되기 때문
iwdtTask_init();
#endif
  log_boot_reason();
  DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"start end\r\n");

  osThreadExit();  // 종료 시킴
}

void startTask_init(void)
{
  osThreadNew(startTask, NULL, &kStartTask_attributes);
}