
#include <string.h>



#include "driver_stm32_i2c.h"
#include "driver_di_def.h"
#include "driver_gpio_def.h"
#include "driver_do_define.h"
#include "pcf8575.h"
#include "system_err.h"


#define PCF8575_0X20 0


typedef struct pcf8575_cfg_s
{
  void *i2c_io;
  void *irq_io;
  uint16_t address;
  uint16_t port_data;
  uint16_t dir;//읽기 1, 쓰기 0
}pcf8575_cfg_t;


int32_t pcf8575_write8(driver_t *drv,uint8_t port_data);
int32_t pcf8575_read8(driver_t *drv,uint16_t *port_data);
int pcf8575_write_pin(driver_t *drv,uint16_t pin,uint16_t high);
int pcf8575_write(driver_t *drv,uint16_t port_data);


gpio_api_t pcf8575_gpio_api ={.write8 = pcf8575_write8,
                              .read8  = pcf8575_read8,
                              .write_pin = pcf8575_write_pin};

pcf8575_cfg_t g_pcf8575_cfg;
driver_t g_pcf8575;

driver_t *pcf8575_open(uint32_t num,void *opt)
{
  i2c_open_opt_t i2c_open_opt;
  uint16_t dir=0;

  if(g_pcf8575.opened)
  {
    return &g_pcf8575;
  }

  switch (num)
  {
  case PCF8575_0X20:

    i2c_open_opt.freq = 400000;

    g_pcf8575_cfg.i2c_io =  driver_stm32_i2c_open(STM32_I2C_2,&i2c_open_opt);
    g_pcf8575_cfg.address = 0x20;
    g_pcf8575.opened = true;
    g_pcf8575.api  =&pcf8575_gpio_api;
    g_pcf8575.cfg = &g_pcf8575_cfg;

#if 0 
    if(g_pcf8575.sem == NULL)
    {
      g_pcf8575.sem = osSemaphoreNew(1, 1, NULL);

      if(g_pcf8575.sem ==NULL)
      {
        Error_Handler(__FILE__,__LINE__);
      }
    }
#endif
    dir |= DIR_IN(GPIO_PIN0);
    dir |= DIR_IN(GPIO_PIN1);
    dir |= DIR_IN(GPIO_PIN2);
    dir |= DIR_IN(GPIO_PIN3);
    dir |= DIR_IN(GPIO_PIN4);
    dir |= DIR_IN(GPIO_PIN5);
    dir |= DIR_IN(GPIO_PIN6);
    dir |= DIR_IN(GPIO_PIN7);
    

    pcf8575_write(&g_pcf8575,(uint16_t)dir);//방향을 설정한다.
    
    for(int i = 0 ;i < 8; i++)
    {
      pcf8575_write_pin(&g_pcf8575,1<<i,1);//전부 High 출력
    }


    break;
  }

  return &g_pcf8575;
}






int pcf8575_write(driver_t *drv,uint16_t port_data)
{
  uint8_t data[2];
  pcf8575_cfg_t *cfg = (pcf8575_cfg_t *)drv->cfg;

  cfg->port_data = port_data; 

  memcpy(data,&port_data,2);

  stm32_i2c_send_byte(cfg->i2c_io,cfg->address,data,2);

  return 0;
}


int pcf8575_read(driver_t *drv,uint16_t *port_data)
{
  uint8_t data[2];

  pcf8575_cfg_t *cfg = (pcf8575_cfg_t *)drv->cfg;

  stm32_i2c_recv_byte(cfg->i2c_io,cfg->address,data,2);

  *port_data = ((uint16_t)data[0]) | ((uint16_t)data[1]<<8)&0xFF00;

 return 0; 
}





void pcf8575_irq(void)
{
  
}





//하드코딩 함,0..7 입력, 8..15출력 추후 수정
int32_t pcf8575_write8(driver_t *drv,uint8_t port_data)
{
  uint8_t data[2];
  uint8_t data16;
  pcf8575_cfg_t *cfg = (pcf8575_cfg_t *)drv->cfg;

  data16 = port_data<<8|0xFF;

  cfg->port_data = port_data; 

  memcpy(data,&data16,2);

  stm32_i2c_send_byte(cfg->i2c_io,cfg->address,data,2);

  return 0;
}


int pcf8575_read8(driver_t *drv,uint16_t *port_data)
{
  uint8_t data[2];

  uint16_t data16;
  pcf8575_cfg_t *cfg = (pcf8575_cfg_t *)drv->cfg;

  stm32_i2c_recv_byte(cfg->i2c_io,cfg->address,data,2);

 

   data16 = ((uint16_t)data[0]) | ((uint16_t)data[1]<<8)&0xFF00;

   *port_data = data16&0x00FF;

 return 0; 
}





uint16_t pcf8575_read_pin(driver_t *drv,uint16_t pin)
{
  uint8_t data[2];

  uint16_t data16;
  pcf8575_cfg_t *cfg = (pcf8575_cfg_t *)drv->cfg;

  stm32_i2c_recv_byte(cfg->i2c_io,cfg->address,data,2);

  data16 = ((uint16_t)data[0]) | ((uint16_t)data[1]<<8)&0xFF00;

  if(data16 &pin)
  {
    return 1;
  }

  return 0;
}

//상위 8bit가 출력
int pcf8575_write_pin(driver_t *drv,uint16_t pin,uint16_t high)
{
  uint8_t data[2];
  uint16_t data16;
  pcf8575_cfg_t *cfg = (pcf8575_cfg_t *)drv->cfg;


  stm32_i2c_recv_byte(cfg->i2c_io,cfg->address,data,2);

  data16 = ((uint16_t)data[0]) | ((uint16_t)data[1]<<8)&0xFF00;


  if(high)
  {
    data16 = data16|(pin<<8);
  }
  else
  {
    data16 = data16&(~(pin<<8));
  }


  cfg->port_data |= data16;
  data16 |=0x00FF;


  memcpy(data,&data16,2);

  stm32_i2c_send_byte(cfg->i2c_io,cfg->address,data,2);

  return 0;
}



void pcf8575_di_close(driver_t *drv);
int32_t pcf8575_di_read(driver_t *drv);
void pcf8575_di_set(driver_t *drv,di_set_option_t cmd,void *option);

const di_api_t pcf8575_di_api={.close = pcf8575_di_close,
                               .read  = pcf8575_di_read,
                               .set   = pcf8575_di_set};

typedef struct pcf8575_di_cfg_s
{
  driver_t *pcf8575_io;
  uint16_t channel;
}pcf8575_di_cfg_t,pcf8575_do_cfg_t;

driver_t g_pcf8575_di_list[8];
pcf8575_di_cfg_t pcf8575_di_cfg[8];



driver_t *pcf8575_di_open(uint32_t num,void *opt)
{
  uint16_t dir=0;
  i2c_open_opt_t i2c_open_opt;


  if(g_pcf8575_di_list[num].opened)
  {
    return &g_pcf8575_di_list[num];
  }

  
  g_pcf8575_di_list[num].cfg = &pcf8575_di_cfg[num];
  g_pcf8575_di_list[num].api = &pcf8575_di_api;

  switch(num)
  {
    case DI_PCF8575_0:
    pcf8575_di_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_di_cfg[num].channel = num;
    break;
    case DI_PCF8575_1:
    pcf8575_di_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_di_cfg[num].channel = num;
    break;
    case DI_PCF8575_2:
    pcf8575_di_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_di_cfg[num].channel = num;
    break;
    case DI_PCF8575_3:
    pcf8575_di_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_di_cfg[num].channel = num;
    break;
    case DI_PCF8575_4:
    pcf8575_di_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_di_cfg[num].channel = num;
    break;
    case DI_PCF8575_5:
    pcf8575_di_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_di_cfg[num].channel = num;
    break;
    case DI_PCF8575_6:
    pcf8575_di_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_di_cfg[num].channel = num;
    break;
    case DI_PCF8575_7:
    pcf8575_di_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_di_cfg[num].channel = num;
    break;

  }

  return &g_pcf8575_di_list[num];
}



void pcf8575_di_close(driver_t *drv)
{

}


void pcf8575_di_set(driver_t *drv,di_set_option_t cmd,void *option)
{


}


int32_t pcf8575_di_read(driver_t *drv)
{
  pcf8575_di_cfg_t *cfg = drv->cfg;
  driver_t *pcf;

  pcf = cfg->pcf8575_io;

  gpio_api_t *api = (gpio_api_t*)pcf->api;

  int32_t ret;
  uint16_t data;

  ret = api->read8(pcf,&data);

  if(data&(1<<cfg->channel))
  {
    return 1;
  }

  return 0;
  

}




driver_t g_pcf8575_do_list[8];
pcf8575_do_cfg_t pcf8575_do_cfg[8];

void pcf8575_low(driver_t *handle);
void pcf8575_high(driver_t *handle);
void pcf8575_do_close(driver_t *handle);
void pcf8575_do_set(driver_t *handle, do_set_option_t option, void *value);


const do_api_t pcf8575_do_api={.close = pcf8575_do_close,
                               .set = pcf8575_do_set,
                               .low  = pcf8575_low,
                               .high   = pcf8575_high};


driver_t *pcf8575_do_open(uint32_t num,void *opt)
{
  uint16_t dir=0;

  if(g_pcf8575_do_list[num].opened)
  {
    return &g_pcf8575_do_list[num];
  }

  
  g_pcf8575_do_list[num].cfg = &pcf8575_do_cfg[num];
  g_pcf8575_do_list[num].api = &pcf8575_do_api;

  switch(num)
  {
    case DI_PCF8575_0:
    pcf8575_do_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_do_cfg[num].channel = num;
    break;
    case DI_PCF8575_1:
    pcf8575_do_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_do_cfg[num].channel = num;
    break;
    case DI_PCF8575_2:
    pcf8575_do_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_do_cfg[num].channel = num;
    break;
    case DI_PCF8575_3:
    pcf8575_do_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_do_cfg[num].channel = num;
    break;
    case DI_PCF8575_4:
    pcf8575_do_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_do_cfg[num].channel = num;
    break;
    case DI_PCF8575_5:
    pcf8575_do_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_do_cfg[num].channel = num;
    break;
    case DI_PCF8575_6:
    pcf8575_do_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_do_cfg[num].channel = num;
    break;
    case DI_PCF8575_7:
    pcf8575_do_cfg[num].pcf8575_io = pcf8575_open(PCF8575_0X20,NULL);
    pcf8575_do_cfg[num].channel = num;
    break;

  }

  return &g_pcf8575_do_list[num];
}


void pcf8575_low(driver_t *drv)
{
  pcf8575_do_cfg_t *cfg = drv->cfg;
  driver_t *pcf;

  pcf = cfg->pcf8575_io;

  const gpio_api_t *api = pcf->api;

  int32_t ret;
  uint16_t data;

  api->write_pin(pcf,1<<cfg->channel,0);
}

void pcf8575_high(driver_t *drv)
{
  pcf8575_do_cfg_t *cfg = drv->cfg;
  driver_t *pcf;

  pcf = cfg->pcf8575_io;

  const gpio_api_t *api = pcf->api;

  int32_t ret;
  uint16_t data;

  api->write_pin(pcf,1<<cfg->channel,1);
}


void pcf8575_do_close(driver_t *handle)
{

}

void pcf8575_do_set(driver_t *handle, do_set_option_t option, void *value)
{

}