

#include "app_rtc.h"
#include "driver_led.h"
#include "driver_rtc.h"

#include "adc.h"
#include "app_flash.h"
#include "cmsis_os.h"
#include "config.h"
#include "fatfs.h"
#include "fsmc.h"
#include "dev_io.h"
#include "lwip.h"
#include "main.h"
#include "mcu_delay.h"
#include "mcu_interrupt.h"
#include "project_def.h"
#include "sdio.h"
#include "user_heap.h"
#include "task_start.h"
#include "task_test.h"
#include  "task_ethernet.h"
#include "task_logging.h"
#include "task_isrEvent.h"
#include "task_console.h"
#include "task_measure.h"
#include "utile_time.h"
#include "task_tcpServer.h"
#include "mcu_utile.h"



const osThreadAttr_t startTask_attributes = {
  .name = "startTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityRealtime,
};





void runLed_init(void)
{
  driver_t *runLed;    
  led_freq_cfg_t cfg={.freq=5,.highDuty=10};

  runLed = driver_led_open(LED_SYS_RUN);
  
  driver_led_set(runLed,LED_CMD_SET_TOGGLE_FREQ,&cfg);
  driver_led_set(runLed,LED_CMD_START,NULL);

}

void startTask(void *arg)
{
  DWT_Delay_Init();
  mcu_interrupt_init();//최우선 실행
  rtc_init();

  config_init();
  flash_init();
  runLed_init();

  isrEventTask_init();
  loggingTask_init();
  measureTask_init();
  consoleTask_init();

  if(config.eth_use)
  {
    tcpServerTask_init(0);
    ethernetTask_init();

  }

  MX_ADC1_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  DWT_Delay_Init();
 
  osThreadExit();//종료 시킴
  
}

void startTask_init(void)
{
  osThreadNew(startTask, NULL, &startTask_attributes);
}