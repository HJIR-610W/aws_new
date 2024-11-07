




#include "FreeRTOS.h"
#include "main.h"
#include "cmsis_os.h"


#include "driver_adc.h"
#include "driver_spi.h"
#include "driver_digitalOut.h"
#include "driver_digitalIn.h"
#include "ads1220.h"
#include "driver_mux.h"
#include "utile.h"
typedef struct adc_cfg_S
{
    ;
    GPIO_PinState pin;
   
}digitalOut_cfg_t;


typedef struct adc_api_s
{
  void (*init)(void);
  void (*start)(uint32_t ch);
  void (*stop)(uint32_t ch);
  void (*read)(int32_t *val,uint32_t ch);
}adc_api_t;




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




void driver_adc_start(driver_adc_t *adc)
{
  
}
void driver_adc_stop(driver_adc_t *adc)
{
  
}

void driver_adc_read(driver_adc_t *adc,uint32_t *val,uint32_t ch)
{
    adc_api_t *adc_api = (adc_api_t *)adc->api;

    adc_api->read(val,ch);
}



  static ads1210_t ads1210;

void ads1220_init(void)
{

  static driver_spi_t ads1220_spi;
  static driver_digitalOut_t ads1220_cs;
  static driver_digitalIn_t ads1220_drdy;
digitalIn_set_interupt_t cfgInt;


  driver_spi_init(&ads1220_spi,STM_SPI_2);
  driver_digitalOut_init(&ads1220_cs,DO_ADC_NCS);
  
  driver_digitalIn_init(&ads1220_drdy,DI_ADC_RDY);




  ads1210.spi_io = &ads1220_spi;
  ads1210.cs_io  = &ads1220_cs;
  ads1210.drdy_i = &ads1220_drdy;

  cfgInt.call = irq_dataReady;
  cfgInt.intType = INT_FALLING;
  cfgInt.priority = 5;
  cfgInt.handle = &ads1210;

  //driver_digitalIn_set(&ads1220_drdy,eDIG_IN_SET_INTERRUPT,&cfgInt);

  ads1210_init(&ads1210);




}


void ads1220_read(int32_t *val,uint32_t ch)
{
  *val = g_adcChannel[ch];
}


void stm32_init(void)
{
  
}

void stm32_read(int32_t *val,uint32_t ch)
{
  
}

void set_adc_mux(uint16_t ch)
{
  
}


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
      adc_mux_set(channel);
      osDelay(50);
      
      sum = 0;
      sample_cnt=0;
      first =1;
      
      for(int n = 0 ; n< 10; n++)
      {
        g_sample[n]=0;
        adc = AD1220_read_data(&ads1210,channel%4,&err);
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


void driver_adc_init(driver_adc_t *adc,uint32_t num)
{
  
    switch(num)
    {
        case 0:
        if(g_ads1220TaskHandle==NULL)
        {
            g_ads1220TaskHandle = osThreadNew(adc1220Task, NULL, &ads1220Task_attributes);  
        }
        
        adc->api = (void *)&g_ads1220;

        ads1220_init();
        adc_mux_init();

        break;
    case 1:
              if(g_adcstm32adcTaskHandle==NULL)
        {
            g_adcstm32adcTaskHandle = osThreadNew(stm32AdcTask, NULL, &adcstm32Task_attributes);  
        }
        adc->api = (void *)&g_stm32;
      break;
    }
}

adc_api_t g_ads1220={.read = ads1220_read,.init = ads1220_init};
adc_api_t g_stm32={.read = stm32_read,.init= stm32_init};

