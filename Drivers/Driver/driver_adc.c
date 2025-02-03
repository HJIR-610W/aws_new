




#include "FreeRTOS.h"
#include "pcb_define.h"
#include "cmsis_os.h"


#include "driver_adc.h"
#include "driver_spi.h"
#include "driver_digitalOut.h"
#include "driver_digitalIn.h"
#include "ads1220.h"
#include "driver_mux.h"
#include "mcu_interrupt.h"
#include "utile.h"



typedef struct adc_cfg_S
{
    
    GPIO_PinState pin;
   
}digitalOut_cfg_t;

typedef enum adc_ch_mode_s
{
  eADC_CH_SINGLE,
  eADC_CH_DIFF
}eADC_CH_MODE_t;

typedef struct adc_api_s
{
  int32_t (*read_single)(driver_t *drv,int32_t ch,uint8_t *err);
  int32_t (*read_diff)(driver_t *drv,int32_t ch,uint8_t *err);
}adc_api_t;

typedef struct driver_adc_cfg_S
{
  eADC_CH_MODE_t ch_mode;
  uint8_t average_cnt;
}driver_adc_cfg_t;

adc_api_t g_ads1220;
adc_api_t g_stm32;


osThreadId_t g_ads1220TaskHandle=NULL;
osThreadId_t g_adcstm32adcTaskHandle=NULL;


static const osThreadAttr_t ads1220Task_attributes = {
  .name = "ads1220",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

static const osThreadAttr_t adcstm32Task_attributes = {
  .name = "stm32adc",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


int32_t g_adcChannel[32];





void stm32AdcTask(void *argument)
{



  for(;;)
  {
        osDelay(100);

  }

}




void driver_adc_start(driver_t *adc)
{
  
}
void driver_adc_stop(driver_t *adc)
{
  
}







void ads1220_read(int32_t *val,uint32_t ch)
{
  

  *val = g_adcChannel[ch];
}


void stm32_init(void)
{
  
}



void set_adc_mux(uint16_t ch)
{
  
}
driver_t g_ads1220_h={.opened = false};

int32_t g_sample[20];
void adc1220Task(void *argument)
{
  uint8_t err;
  int32_t adc;
  uint16_t adcChannelList[]={0,1,4,5,8,9,12,13,16,17,20,21,24,25,28,29,2,6};
  uint16_t channel=0;
  uint32_t sum=0;
int n=0;
int sample_cnt=0;
int32_t first=0;

  for(;;)
  {
    for(int i = 0 ; i< _countof(adcChannelList);i++)
    {
      channel = adcChannelList[i];
      adc_single_mux_set(channel);
      osDelay(50);
      
      sum = 0;
      sample_cnt=0;
      first =1;
      
      for(int n = 0 ; n< 10; n++)
      {
        g_sample[n]=0;
        //adc = ads1220_read_single_ch(g_ads1220_h.handle,channel%4,&err);
        if(err == 0)
        {
          g_sample[n]=adc;
          if(first)
          {
            first =0;
            continue;
          }
          sample_cnt++;
          sum +=adc;
        }
    

      }

        if(sample_cnt)
        g_adcChannel[channel] = sum/sample_cnt;
        else
        g_adcChannel[channel] = -1;


    }
  }

}


void ads1220_common_open(driver_t *adc)
{
  static bool initialized=false;
  static driver_t *p_ads1220=NULL;
      
  if(adc->opened == false)
  {
    if(initialized == false)//한번만 초기화
    {

      ads1220_cfg_t *p_ads1220_cfg;
 

      p_ads1220 = ads1220_open();
      p_ads1220_cfg = (ads1220_cfg_t *)(p_ads1220->cfg);

      p_ads1220_cfg->spi_io = driver_spi_open(STM_SPI_2);
      p_ads1220_cfg->cs_io  = driver_do_open(DO_ADC_NCS);
      p_ads1220_cfg->irq_io = driver_di_open(DI_ADC_RDY);

      ads1210_init(p_ads1220);
      adc_mux_init();

      initialized = true;
    }

    adc->handle = p_ads1220;
  }
}



typedef struct adc_cfg_s
{
  int samplingHz;
}adc_cfg_t;



adc_api_t adc_api_ads1220 ={.read_single = ads1220_read_single_ch,
                            .read_diff   = ads1220_read_diff_ch};


driver_t g_ads1220_single;
driver_t g_ads1220_diff;
driver_adc_cfg_t g_ads1220_cfg_single;
driver_adc_cfg_t g_ads1220_cfg_diff;

driver_t * driver_adc_open(uint32_t num)
{
  
    switch(num)
    {
      case ADC_ADS1220_SINGLE_CH_0:
      case ADC_ADS1220_SINGLE_CH_1:
      case ADC_ADS1220_SINGLE_CH_2:
      case ADC_ADS1220_SINGLE_CH_3://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_4://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_5:
      case ADC_ADS1220_SINGLE_CH_6:
      case ADC_ADS1220_SINGLE_CH_7:
      case ADC_ADS1220_SINGLE_CH_8:
      case ADC_ADS1220_SINGLE_CH_9:
      case ADC_ADS1220_SINGLE_CH_10://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_11://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_12:
      case ADC_ADS1220_SINGLE_CH_13:
      case ADC_ADS1220_SINGLE_CH_14://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_15://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_16:
      case ADC_ADS1220_SINGLE_CH_17:
      case ADC_ADS1220_SINGLE_CH_18://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_19://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_20:
      case ADC_ADS1220_SINGLE_CH_21:
      case ADC_ADS1220_SINGLE_CH_22://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_23://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_24:
      case ADC_ADS1220_SINGLE_CH_25:
      case ADC_ADS1220_SINGLE_CH_26://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_27://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_28:
      case ADC_ADS1220_SINGLE_CH_29:
      case ADC_ADS1220_SINGLE_CH_30://하드웨어 미지원
      case ADC_ADS1220_SINGLE_CH_31://하드웨어 미지원
        ads1220_common_open(&g_ads1220_single);
        g_ads1220_cfg_single.ch_mode = eADC_CH_SINGLE;
        g_ads1220_single.cfg = &g_ads1220_cfg_single;
        g_ads1220_single.api  = &adc_api_ads1220;
        return &g_ads1220_single;
      case ADC_ADS1220_DIFF_CH_0:
      case ADC_ADS1220_DIFF_CH_1:
      case ADC_ADS1220_DIFF_CH_2:
      case ADC_ADS1220_DIFF_CH_3:
      case ADC_ADS1220_DIFF_CH_4:
      case ADC_ADS1220_DIFF_CH_5:
      case ADC_ADS1220_DIFF_CH_6:
      case ADC_ADS1220_DIFF_CH_7:
        ads1220_common_open(&g_ads1220_diff);
        g_ads1220_cfg_diff.ch_mode = eADC_CH_DIFF;
        g_ads1220_diff.cfg = &g_ads1220_cfg_diff;
        g_ads1220_diff.api  = &adc_api_ads1220;
        return &g_ads1220_diff;
      break;
    }
    
    return 0;
}

void stm32_read(int32_t *val,uint32_t ch)
{
  
}



/**
 * @brief 
 */
int32_t driver_adc_read(driver_t *drv,uint32_t ch,uint8_t *err)
{
  adc_api_t *adc_api = (adc_api_t *)drv->api;
  driver_adc_cfg_t *cfg = drv->cfg;
  uint32_t diff_ch;
  int32_t adc;
  
  if(cfg->ch_mode == eADC_CH_DIFF)
  {
    diff_ch = (ch - ADC_ADS1220_DIFF_CH_0);//총 8개 채널이 실제 물리 0채널임
    //차동 채널 0,1,2,3,4,5,6,7 은 ADS1220에서는 0채널로만 측정하며  MUX가 채널이 됨
    adc_diff_mux_set(diff_ch);
    
    adc = adc_api->read_diff(drv->handle,diff_ch/8,err);
  }
  else
  {
    adc_single_mux_set(ch);
    adc = adc_api->read_single(drv->handle,ch%4,err);
  }

  return adc;

}


/**
 * @brief 
 */
int32_t driver_adc_read_average(driver_t *drv,uint32_t ch,uint8_t *err,uint8_t average_cnt)
{
  adc_api_t *adc_api = (adc_api_t *)drv->api;
  driver_adc_cfg_t *cfg = drv->cfg;
  uint32_t diff_ch;
  int32_t adc;
  int32_t sum=0;
  uint8_t valid_cnt=0;

  if(cfg->ch_mode == eADC_CH_DIFF)
  {
    
    diff_ch = (ch - ADC_ADS1220_DIFF_CH_0);//총 8개 채널이 실제 물리 0채널임
    //차동 채널 0,1,2,3,4,5,6,7 은 ADS1220에서는 0채널로만 측정하며  MUX가 채널이 됨
    adc_diff_mux_set(diff_ch);
    
    for(int i = 0 ; i< average_cnt;i++)
    {
      adc = adc_api->read_diff(drv->handle,diff_ch/8,err);
      if(*err ==0)
      {
        sum += adc;
        valid_cnt++;
      }
    }
  }
  else
  {
     adc_single_mux_set(ch);
     for(int i = 0 ; i< average_cnt; i++)
     {
       adc = adc_api->read_single(drv->handle,ch%4,err);
     
      if(*err ==0)
      {
        sum += adc;
        valid_cnt++;
      }
      

     }
  }

  adc = sum/valid_cnt;

  return adc;

}



void driver_adc_set(driver_t *drv,uint8_t cmd,void *option)
{
  driver_adc_cfg_t *cfg = drv->cfg;
  switch(cmd)
  {
    case ADC_CMD_AVERAGE_SET:
    {
      uint8_t average_cnt = (uint8_t)(int)option;


    }
    break;
  }
}