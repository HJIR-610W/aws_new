/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "driver_led.h"
#include "driver_digitalOut.h"
#include "driver_adc.h"
#include "driver_fram.h"
#include "utile.h"
    
#include "terminal.h"
#include "vt100_command.h"
#include "io.h"
#include "time_define.h"
    #include "driver_rtc.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */

extern UART_HandleTypeDef huart1;


driver_adc_t g_ads12;



 uint8_t test_buff[5];
void fram_test(void)
{
  driver_t *fram;
 

  fram = driver_fram_open(FRAM_FM25LC);

  for(int i = 0;i< sizeof(test_buff);i++)
  {
    test_buff[i] = i;
  }

  driver_fram_write(fram,0,test_buff,sizeof(test_buff));

  memset(test_buff,0,sizeof(test_buff));
  
  driver_fram_read(fram,0,test_buff,sizeof(test_buff)); 

}

DATE_TIME_BUF date;
void rtc_test(void)
{
  driver_t *rtc;
  uint32_t startTieck;

  rtc = driver_rtc_open(RTC_DS1306);

  startTieck = osKernelSysTick();

  while((osKernelSysTick()-startTieck)<20000)
  {
    driver_rtc_read(rtc,&date);

    debug_printf("%02d:%02d:%02d\r\n",date.Hour,date.Min,date.Sec);
    osDelay(500);
  }
}

void StartDefaultTask(void *argument)
{
  char buff[100];
  
  driver_led_t runLed;
  int32_t adc;
  uint16_t adcList[]={0,1,4,5,8,9,12,13,16,17,20,21,24,25,28,29,2,6};
  const char *nameList[]={"A_SIG_1","A_SIG_2",
                          "A_SIG_3","A_SIG_4",
                          "A_SIG_5","A_SIG_6",
                          "A_SIG_7","A_SIG_8",
                          "A_SIG_9","A_SIG_10",
                          "A_SIG_11","A_SIG_12",
                          "A_SIG_13","A_SIG_14",
                          "A_SIG_15","A_SIG_16",
                          "A_SIG_RTD_0","A_SIG_RTD_1"};


  uint32_t i=0;

  MX_LWIP_Init();
  
  fram_test();
  rtc_test();


    driver_led_init(&runLed,LED_SYS_RUN);

    driver_adc_init(&g_ads12,ADC_ADS1220);


    debug_printf(VT100_CLEAR_SCREEN);
    debug_printf(VT100_CURSOR_OFF);

  for(;;)
  {
    debug_printf(VT100_CURSOR_HOME);

       osDelay(50);
    for(i = 0 ;i <_countof(adcList);i++)
    {
        driver_adc_read(&g_ads12,&adc,adcList[i]);
        snprintf(buff,sizeof(buff),"CH:%02d,%10d,%s\r\n",adcList[i],adc,nameList[i]);
        HAL_UART_Transmit(&huart1, buff, strlen(buff), HAL_MAX_DELAY);
    }
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

