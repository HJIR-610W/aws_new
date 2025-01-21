
#include <string.h>
#include "pcf8575.h"
#include "driver_stm32_i2c.h"

pcf8575_cfg_t g_pcf8575_cfg;
driver_t g_pcf8575;




driver_t *pcf8575_open(uint32_t num)
{
  if(g_pcf8575.opened == false)
  {
    g_pcf8575.opened = true;
    g_pcf8575.cfg = &g_pcf8575_cfg;
 
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


int pcf8575_set(driver_t *drv,uint8_t cmd,void *option)
{
  switch(cmd)
  {
    case PCF88575_CMD_DIR_SET:
      {
      uint32_t  dir = (uint32_t)option;
      pcf8575_write(drv,(uint16_t)dir);//방향을 설정한다.
      }
    break;
  }

  return 0;
}



void pcf8575_irq(void)
{
  
}


//하드코딩 함,0..7 입력, 8..15출력 추후 수정
int pcf8575_write8(driver_t *drv,uint16_t port_data)
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
  
  data16 |=0x00FF;

  memcpy(data,&data16,2);

  stm32_i2c_send_byte(cfg->i2c_io,cfg->address,data,2);

  return 0;
}
