#include "cmsis_os2.h"

#include "app_file.h"
#include "app_rtc.h"
#include "app_logging.h"
#include "app_dataLogging.h"
#include "app_flash.h"

#include "driver_led.h"
#include "driver_rtc.h"

#include "config.h"
#include "fatfs.h"
#include "fsmc.h"
#include "dev_io.h"
#include "lwip.h"
#include "main.h"
#include "usDelay.h"
#include "mcu_interrupt.h"
#include "project_def.h"
#include "sdio.h"
#include "user_heap.h"
#include "task_cellular.h"
#include "task_start.h"
#include "task_ethernet.h"
#include "task_logging.h"
#include "task_isrEvent.h"
#include "task_system.h"
#include "task_console.h"
#include "task_hart.h"
#include "task_measure.h"
#include "task_direct.h"
#include "task_modbus.h"
#include "utile_time.h"
#include "task_sdi.h"
#include "task_tcpServer.h"
#include "task_usb.h"
#include "mcu_utile.h"


const osThreadAttr_t kStartTask_attributes = {
  .name = "startTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityRealtime7,
};


void runLed_init(void)
{
  driver_t *run_led;    
  led_freq_cfg_t cfg={.freq=5,.highDuty=10};

  run_led = driver_led_open(LED_SYS_RUN);
  
  driver_led_set(run_led,LED_CMD_SET_TOGGLE_FREQ,&cfg);
  driver_led_set(run_led,LED_CMD_START,NULL);

}

/**
 * @brief 한번 수행하고 종료될 Task
 * 초기화 작업 수행
 * 우선순위는 가장높게 설정하여 startTask가 종료되기까지 다른 Task가 
 * 실행 되지 않도록 함
 */
void startTask(void *arg)
{
  mcu_interrupt_init();//최우선 실행
  
  usbTask_init();
  osDelay(1000);
  usDelay_init();
  rtc_init();



  config_init();
  flash_init();


  modbusTask_init();
  systemTask_init();
  consoleTask_init();
  

  isrEventTask_init();
  dataLogging_init();
  loggingTask_init();

  
  measureTask_init();

  if(config.cdma_use)
  {
    cellularTask_init();
  }
  if(config.direct_use)
  {
    directTask_init();
  }

  if(config.eth_use)
  {
    tcpServerTask_init(0);
    ethernetTask_init();
  }
  
 // hartTask_init();
  sdiTask_init();
  
  file_init();

  logging_init();

  runLed_init();
  os_logging_printf("Starting task");
  osThreadExit();//종료 시킴

}


void startTask_init(void)
{
  osThreadNew(startTask, NULL, &kStartTask_attributes);
}