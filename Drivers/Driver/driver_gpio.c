




#include "driver_gpio.h"

#include "stm32f4xx_hal.h"
#include "main.h"
#include <stdint.h>
#include "pcf8575.h"
#include "driver_stm32_i2c.h"
#include "driver_interface.h"
#include "driver_gpio_def.h"

typedef struct driver_gpio_api_s
{
  int (*write)(driver_t *,uint16_t);
  int (*read)(driver_t *,uint16_t *);
  uint16_t (*read_pin)(driver_t *drv,uint16_t pin);
  int (*write_pin)(driver_t *drv,uint16_t pin,uint16_t high);
}driver_gpio_api_t;

typedef struct driver_gpio_cfg_s
{
  uint16_t wData;
}driver_gpio_cfg_t;

driver_gpio_api_t driver_gpio_api={.write = pcf8575_write8,
                                   .read = pcf8575_read8,
                                   .read_pin =pcf8575_read_pin,
                                   .write_pin = pcf8575_write_pin};
driver_t g_gpio[2];






driver_t *driver_gpio_open(uint32_t num)
{
  if(g_gpio[num].opened)
  {
    return &g_gpio[num];
  }
  switch(num)
  {
    case DRIVER_PCF8575:
    pcf8575_cfg_t *cfg;
    driver_t *drv;

    
    drv = pcf8575_open(0);
    cfg = (pcf8575_cfg_t *)drv->cfg;

    cfg->i2c_io = driver_stm32_i2c_open(STM32_I2C_2);
    cfg->address = 0x20;

    uint16_t dir = 0;
    
    dir |= DIR_IN(GPIO_PIN0);
    dir |= DIR_IN(GPIO_PIN1);
    dir |= DIR_IN(GPIO_PIN2);
    dir |= DIR_IN(GPIO_PIN3);
    dir |= DIR_IN(GPIO_PIN4);
    dir |= DIR_IN(GPIO_PIN5);
    dir |= DIR_IN(GPIO_PIN6);
    dir |= DIR_IN(GPIO_PIN7);

    pcf8575_set(drv,PCF88575_CMD_DIR_SET,(void *)dir); 
    g_gpio[0].api = &driver_gpio_api;
    g_gpio[0].handle = drv;
    
    break;
  }

  return &g_gpio[0];
}


int driver_gpio_write(driver_t *drv,uint16_t port_data)
{
  uint16_t data16;

  driver_gpio_api_t *api = (driver_gpio_api_t *)drv->api;
  
  api->write(drv->handle,port_data);

}


int driver_gpio_read(driver_t *drv,uint16_t *port_data)
{
  driver_gpio_api_t *api = (driver_gpio_api_t *)drv->api;
  return api->read(drv->handle,port_data);
}


int driver_gpio_write_pin(driver_t *drv,uint16_t pin, uint16_t set)
{
  uint16_t data16;

  driver_gpio_api_t *api = (driver_gpio_api_t *)drv->api;
  
  return api->write_pin(drv->handle,pin,set);

}


uint16_t driver_gpio_read_pin(driver_t *drv,uint16_t pin)
{
  driver_gpio_api_t *api = (driver_gpio_api_t *)drv->api;
  return api->read_pin(drv->handle,pin);
}




void driver_gpio_write_pin_low(driver_t *drv,uint16_t pin)
{
  uint16_t data16;

  driver_gpio_api_t *api = (driver_gpio_api_t *)drv->api;
 
  api->write_pin(drv->handle,pin,0);
}

void driver_gpio_write_pin_high(driver_t *drv,uint16_t pin)
{
  uint16_t data16;

  driver_gpio_api_t *api = (driver_gpio_api_t *)drv->api;
  
   api->write_pin(drv->handle,pin,1);

}
