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
#include "driver_do.h"
#include "driver_di.h"
#include "driver_adc.h"
#include "driver_fram.h"
#include "driver_freqInput.h"
#include "driver_rtc.h"
#include "driver_stm32_uart.h"
#include "driver_flash.h"
#include "driver_485.h"
#include "driver_sdi.h"
#include "driver_uart.h"

#include "fatfs.h"



#include "utile.h"
#include "terminal\terminal.h"
#include "terminal\vt100_command.h"
#include "dev_io.h"
#include "time_define.h"


driver_t *debug_uart=NULL;


osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityNormal,
};

extern void MX_LWIP_Init(void);
extern FATFS SDFatFS;    /* File system object for SD logical drive */







void fram_test(void)
{
  driver_t *fram;
static  uint8_t test_buff[5]; 
  fram = driver_fram_open(FRAM_FM25LC);

  for(int i = 0;i< sizeof(test_buff);i++)
  {
    test_buff[i] = i;
  }

  driver_fram_write(fram,0,test_buff,sizeof(test_buff));

  memset(test_buff,0,sizeof(test_buff));
  
  driver_fram_read(fram,0,test_buff,sizeof(test_buff)); 

}

void rtc_test(void)
{
  static DATE_TIME_BUF date;
  driver_t *rtc;
  uint32_t startTieck;

  rtc = driver_rtc_open(RTC_DS1306,0);

  startTieck = osKernelSysTick();

  while((osKernelSysTick()-startTieck)<20000)
  {
    driver_rtc_read(rtc,&date);

    debug_printf("%02d:%02d:%02d\r\n",date.Hour,date.Min,date.Sec);
    osDelay(500);
  }
}


void fat_test(void)
{
    FIL file;            // 파일 객체
    FRESULT res;         // FATFS 함수 결과 코드
    UINT bytesWritten;   // 쓰여진 바이트 수
    UINT bytesRead;      // 읽은 바이트 수
    const char writeData[10] = "1234567890"; // 쓰기 데이터 (10바이트)
    char readData[10] = {0}; // 읽기 데이터를 저장할 버퍼
    const char* fileName = "test.txt"; // 테스트 파일 이름

    // 1. SD 카드 마운트
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    if (res != FR_OK) {
        printf("Failed to mount SD card. Error: %d\n", res);
        return;
    }

    // 2. 파일 열기 (새 파일 생성 또는 쓰기 모드로 열기)
    res = f_open(&file, fileName, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        printf("Failed to open file for writing. Error: %d\n", res);
        f_mount(NULL, (TCHAR const*)SDPath, 1); // SD 카드 언마운트
        return;
    }

    // 3. 파일에 데이터 쓰기
    res = f_write(&file, writeData, sizeof(writeData), &bytesWritten);
    if (res != FR_OK || bytesWritten != sizeof(writeData)) {
        printf("Failed to write data to file. Error: %d\n", res);
        f_close(&file);
        f_mount(NULL, (TCHAR const*)SDPath, 1); // SD 카드 언마운트
        return;
    }

    // 4. 파일 닫기
    f_close(&file);

    // 5. 파일 다시 열기 (읽기 모드로 열기)
    res = f_open(&file, fileName, FA_READ);
    if (res != FR_OK) {
        printf("Failed to open file for reading. Error: %d\n", res);
        f_mount(NULL, (TCHAR const*)SDPath, 1); // SD 카드 언마운트
        return;
    }

    // 6. 파일에서 데이터 읽기
    res = f_read(&file, readData, sizeof(readData), &bytesRead);
    if (res != FR_OK || bytesRead != sizeof(readData)) {
        printf("Failed to read data from file. Error: %d\n", res);
        f_close(&file);
        f_mount(NULL, (TCHAR const*)SDPath, 1); // SD 카드 언마운트
        return;
    }

    // 7. 파일 닫기
    f_close(&file);

    // 8. 데이터 비교
    if (memcmp(writeData, readData, sizeof(writeData)) == 0) {
        printf("Write and read data match!\n");
    } else {
        printf("Write and read data do not match!\n");
    }

    // 9. SD 카드 언마운트
    f_mount(NULL, (TCHAR const*)SDPath, 1);

}





static uint8_t wBuff[1024];
static uint8_t rBuff[1024];

void flash_test(void)
{
  driver_t *flash;


  flash = driver_flash_open(FALSH_AT45DB);

  for(int i = 0 ; i< sizeof(wBuff);i++)
  {
    wBuff[i] = i;
  }
  for(int i = 0 ; i< sizeof(rBuff);i++)
  {
    rBuff[i] = 0;
  }

  driver_flash_write(flash,0,wBuff,sizeof(wBuff));

  driver_flash_read(flash,0,rBuff,sizeof(rBuff),sizeof(rBuff));
}



void adc_test(void)
{
  uint8_t err;
  driver_t *ads1220;
  int i;
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


  ads1220 = driver_adc_open(ADC_ADS1220,0);
    debug_printf(VT100_CLEAR_SCREEN);
    debug_printf(VT100_CURSOR_OFF);

  for(;;)
  {

    debug_printf(VT100_CURSOR_HOME);

    osDelay(50);
    for(i = 0 ;i <_countof(adcList);i++)
    {
       //adc =  driver_adc_read(ads1220,adcList[i],&err);

        debug_printf("CH:%02d,%10d,%s\r\n",adcList[i],adc,nameList[i]);
    }
  }
}

void gpio_test(void)
{
  driver_t *gpio;
  uint16_t data;
  uint16_t out_data=0xffff;
  uint16_t out=0x00ff;
  char buff[20];

  //gpio = driver_gpio_open(DRIVER_PCF8575,NULL);

#if 0 
  while(1)
  {
    driver_gpio_read(gpio,&data);

    out^=0xFF;

    driver_gpio_write(gpio,out_data);
    hex_to_binary_string(data,buff,16);

    printf("b%s\r\n",buff);
    osDelay(1000);
    out^=0xFF;
    
  }
#endif
    uint8_t out_pin=0xff;
  while(1)
  {
    uint8_t pin;

    //pin = driver_gpio_read_pin(gpio,GPIO_PIN3);
   // printf("핀 2:%d\r\n",pin);
    
   // driver_gpio_write_pin(gpio,GPIO_PIN5,out_pin);
    out_pin^=0xff;
    osDelay(100);
  }
}


void freq_test(void)
{
  driver_t *countA;
  driver_t *countB;
  driver_t *countC;
  driver_t *gpio;
  uint16_t out_pin = 1;
  float freq[3];

  countA = driver_freq_open(FREQ_MEAURE_A);
  countB = driver_freq_open(FREQ_MEAURE_A);
  countC = driver_freq_open(FREQ_MEAURE_A);

  //gpio = driver_gpio_open(DRIVER_PCF8575,NULL);

  while(1)
  {
    //driver_gpio_write_pin(gpio,GPIO_PIN5,out_pin);
    osDelay(100);
    out_pin ^= 1;
    driver_freq_read(countA,&freq[0]);
    driver_freq_read(countB,&freq[1]);
    driver_freq_read(countC,&freq[2]);

    debug_printf("%f,%f,%f\r\n",freq[0],freq[1],freq[2]);

  }
}



void rs485_test(void)
{
  driver_t *rs485_a;
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  uint8_t ch='a';
  uint8_t cmd;
  uart_config.baud = 19200;
  uart_config.parityIdx = 0;
  uart_config.stop_bit = 0;

  rs485_a = driver_rs485_open(RS485_A,&uart_config);

  while(1)
  {
    driver_rs485_send(rs485_a,&ch,1);
    driver_rs485_recv(rs485_a,&ch,1,5000);
    osDelay(100);
  
  }
}

void sram_test(void)
{

  static uint16_t rData[10];
  
  volatile uint16_t *ptr = (volatile uint16_t *)0x64000000;

    for(int i = 0 ; i< 10;i++)
    {
      ptr[i]=i;
    }
    
    for(int i = 0 ; i< 10;i++)
    {
      rData[i] = ptr[i];
    }
}




void cdmaPower_test(void)
{
  driver_t *cdma_pwr= driver_do_open(DO_PWR_CDMA,0);

  while(1)
  {
    driver_do_low(cdma_pwr);
    osDelay(500);
    driver_do_high(cdma_pwr);
        osDelay(500);
  }
}


void rain_test(void)
{
  driver_t *rain_reed;
  driver_t *rain_hall;
  driver_t *rain_err;
  int32_t rain_reed_data;
  int32_t rain_hall_data;
  int32_t rain_err_data;

  rain_hall= driver_di_open(DI_RAIN_HALL,0);
  rain_reed= driver_di_open(DI_RAIN_REED,0);
  rain_err= driver_di_open(DI_RAIN_HALL_ERR,0);




  while(1)
  {
      rain_reed_data = driver_di_read(rain_reed);
  rain_hall_data = driver_di_read(rain_hall);
  rain_err_data = driver_di_read(rain_err);
    debug_printf("reed:%d,hall:%d,err:%d\r\n",rain_reed_data,rain_hall_data,rain_err_data);
    osDelay(100);
  }
}
void sss(void *argument)
{
  char buff[100];
  uint32_t i=0;

  debug_uart = stm32_uart_open(STM32_UART_0_DEBUG,0);




 

  //sram_test();
  //fat_test();
  //MX_LWIP_Init();
  //fram_test();
  //rtc_test();
  //flash_test();
  //cmdTask_init();

  //hostTask_init();
  //adc_test();
  //gpio_test();

  //freq_test();
  //rs485_test();
  //cdmaPower_test();
 // rain_test();


  while(1)
  {
    osDelay(1000);
  }


}


