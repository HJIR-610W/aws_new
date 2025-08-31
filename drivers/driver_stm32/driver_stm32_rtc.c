
#include "pcb_define.h"
#include "driver_stm32_rtc.h"
#include "system_err.h"
#include "os_user_def.h"
#include "util_time.h"

typedef struct  stm32_do_cfg_s
{
  RTC_HandleTypeDef *handle;

}stm32_rtc_cfg_t;

driver_t g_stm32_rtc;

RTC_HandleTypeDef RtcHandle;
stm32_rtc_cfg_t stm32_rtc_cfg={.handle= &RtcHandle};





void stm32_rtc_init(void)
{
  RtcHandle.Instance = RTC;

  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follow:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */ 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = 127;
  RtcHandle.Init.SynchPrediv = 255;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);
  if(HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    /* Initialization Error */
    ERROR_PRINTF("rtc");
  }
}


void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{

  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
    
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    ERROR_PRINTF("rtc");
  }

  
  /*##-2- Enable RTC peripheral Clocks #######################################*/ 
  /* Enable RTC Clock */ 
  __HAL_RCC_RTC_ENABLE(); 
  

}

/**
  * @brief RTC MSP De-Initialization 
  *        This function freeze the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
   __HAL_RCC_RTC_DISABLE();     
}
void stm32_rtc_close(driver_t *handle);
int32_t stm32_rtc_read(driver_t *handle,DATE_TIME_BUF *ct);
void stm32_rtc_set(driver_t *handle, rtc_set_option_t option, void *value);



rtc_api_t stm32_rtc_api={.read = stm32_rtc_read,.close = stm32_rtc_close,.set = stm32_rtc_set};


driver_t *driver_stm32_rtc_open(uint32_t num,void *opt)
{
  if(g_stm32_rtc.opened)
  {
    return &g_stm32_rtc;
  }

  g_stm32_rtc.api = &stm32_rtc_api;
  g_stm32_rtc.cfg = &stm32_rtc_cfg;
  

  stm32_rtc_init();

  return &g_stm32_rtc;


}



void stm32_rtc_set_time(driver_t *drv,uint32_t hour,uint32_t min,uint32_t sec)
{
  stm32_rtc_cfg_t *cfg = (drv->cfg);
  RTC_TimeTypeDef stimestructureget;

  stimestructureget.Hours   = hour;
  stimestructureget.Minutes = min;
  stimestructureget.Seconds = sec;
  stimestructureget.TimeFormat = RTC_HOURFORMAT12_PM;
  stimestructureget.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructureget.StoreOperation = RTC_STOREOPERATION_RESET;
  HAL_RTC_SetTime(cfg->handle,&stimestructureget,RTC_FORMAT_BIN);
}

void stm32_rtc_rtc_set_date(driver_t *drv,uint32_t year,uint32_t month,uint32_t day)
{
  stm32_rtc_cfg_t *cfg = (drv->cfg);
    RTC_DateTypeDef sdatestructureget;

    sdatestructureget.Year    = year-2000;
    sdatestructureget.Month   = month;
    sdatestructureget.Date    = day;
    sdatestructureget.WeekDay = RTC_WEEKDAY_MONDAY;

    HAL_RTC_SetDate(cfg->handle,&sdatestructureget,RTC_FORMAT_BIN);
}




void stm32_rtc_close(driver_t *handle)
{

}

int32_t stm32_rtc_read(driver_t *drv,DATE_TIME_BUF *ct)
{
uint8_t sub_sec = 0;
RTC_DateTypeDef sdatestructureget;
RTC_TimeTypeDef stimestructureget;
#if defined(STM32L100xBA) || defined (STM32L151xBA) || defined (STM32L152xBA) || defined(STM32L100xC) || defined (STM32L151xC) || defined (STM32L152xC) || defined (STM32L162xC) || defined(STM32L151xCA) || defined (STM32L151xD) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L162xCA) || defined (STM32L162xD) || defined(STM32L151xE) || defined(STM32L151xDX) || defined (STM32L152xE) || defined (STM32L152xDX) || defined (STM32L162xE) || defined (STM32L162xDX)
uint32_t ans_uint32;
int32_t ans_int32;
uint32_t RtcSynchPrediv = hrtc.Init.SynchPrediv;
#endif
stm32_rtc_cfg_t *cfg = (drv->cfg);

HAL_RTC_GetTime(cfg->handle, &stimestructureget, RTC_FORMAT_BIN);
HAL_RTC_GetDate(cfg->handle, &sdatestructureget, RTC_FORMAT_BIN);


#if defined(STM32L100xBA) || defined (STM32L151xBA) || defined (STM32L152xBA) || defined(STM32L100xC) || defined (STM32L151xC) || defined (STM32L152xC) || defined (STM32L162xC) || defined(STM32L151xCA) || defined (STM32L151xD) || defined (STM32L152xCA) || defined (STM32L152xD) || defined (STM32L162xCA) || defined (STM32L162xD) || defined(STM32L151xE) || defined(STM32L151xDX) || defined (STM32L152xE) || defined (STM32L152xDX) || defined (STM32L162xE) || defined (STM32L162xDX)
/* To be MISRA C-2012 compliant the original calculation:
   sub_sec = ((((((int)RtcSynchPrediv) - ((int)stimestructure.SubSeconds)) * 100) / (RtcSynchPrediv + 1)) & 0xFF);
   has been split to separate expressions */
ans_int32 = (RtcSynchPrediv - (int32_t)stimestructureget.SubSeconds) * 100;
ans_int32 /= RtcSynchPrediv + 1;
ans_uint32 = (uint32_t)ans_int32 & 0xFFU;
sub_sec = (uint8_t)ans_uint32;
#endif
  time_t time_tick;
  DATE_TIME_BUF temp_time;

  temp_time.Year = sdatestructureget.Year + 2000;
  temp_time.Month = sdatestructureget.Month;
  temp_time.Day = sdatestructureget.Date;
  temp_time.Hour = stimestructureget.Hours;
  temp_time.Min = stimestructureget.Minutes;
  temp_time.Sec = stimestructureget.Seconds;
  temp_time.SubSec = sub_sec;

  time_tick = time_cvt_timestamp(&temp_time);

  time_cvt_secTotime(time_tick, ct);
  return 0;
}


void stm32_set_time(driver_t *driver,DATE_TIME_BUF *ct)
{
  stm32_rtc_set_time(driver,ct->Hour,ct->Min,ct->Sec);
  stm32_rtc_rtc_set_date(driver,ct->Year,ct->Month,ct->Day);
}

void stm32_rtc_set(driver_t *driver, rtc_set_option_t option, void *value)
{
  DATE_TIME_BUF *ct;

  OS_PEND_SEM(driver->sem,osWaitForever);

  switch (option)
  {
  case eRTC_SET_TIME:
    ct = value;
    stm32_set_time(driver,ct);
    break;
  case eRTC_SET_IRQ:

    break;
  }
  
  OS_POST_SEM(driver->sem);

}
