

#include "adc.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "fsmc.h"
#include "gpio.h"
#include "i2c.h"
#include "io.h"
#include "lwip.h"
#include "main.h"
#include "mcu_delay.h"
#include "mcu_interrupt.h"
#include "project_def.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "tlsf.h"
#include "usart.h"
#include "task_start.h"
#include "task_test.h"
#include  "task_ethernet.h"
#include "driver_led.h"


#define POOL_SIZE (1024 * 4)  

static char memory_pool[POOL_SIZE];
tlsf_t tlsf_handle=NULL;

const osThreadAttr_t startTask_attributes = {
  .name = "startTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityNormal,
};

void asw_tlsf_init(size_t size)
{
  
   tlsf_handle = tlsf_create_with_pool(memory_pool, size);
   if (tlsf_handle == NULL)
   {
        // 초기화 실패 처리
        while (1);
   }
}


void *aws_malloc(size_t size)
{
  return (void *)tlsf_malloc(tlsf_handle,size);
}


void aws_free(void *ptr)
{
    tlsf_free(tlsf_handle,ptr);
}



/**
 * @brief 드라이버 초기화
 */
void startTask(void *arg)
{
  driver_t *runLed;    
  led_freq_cfg_t cfg={.freq=5,.highDuty=10};
  
  
  asw_tlsf_init(POOL_SIZE);

  debug_uart_init(115200);



  mcu_interrupt_init();
  
  MX_FSMC_Init();

  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  DWT_Delay_Init();
  
  
  runLed = driver_led_open(LED_SYS_RUN);
  
  driver_led_set(runLed,LED_CMD_SET_TOGGLE_FREQ,&cfg);
  driver_led_set(runLed,LED_CMD_START,NULL);
  
//  ethernetTask_init();
  testTask_init();
  
  osThreadExit();//종료 시킴
  
}

void startTask_init(void)
{


  osThreadNew(startTask, NULL, &startTask_attributes);
}