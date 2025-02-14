
#include <stdio.h>

#include "Sensors\snow\snow.h"
#include "driver_uart.h"

#include "app_sensor.h"
#include "hj_snow.h"
#include "dev_io.h"





bool snowInit=false;


bool is_snowInit(void)
{
  return snowInit;
}

bool snow_deInit(void)
{
  snowInit = false;
  
  return snowInit;
}



void snow_init(sensor_t *sensor)
{
  dev_io_t dev_io;
  snowInit = true;

  switch (sensor->type)
  {
    case S_T_SNOW_HJ_485:
    {
        rs485_config_t *rs485_config;
    dev_io.io  = eRS485_IO;
    rs485_config = get_sensor_config(sensor);
    dev_io.handle = (void *)rs485_config->port;
    dev_io.config = (void *)&rs485_config;
    hjsnow_init(&dev_io);
    }

    break;
    case S_T_SNOW_HJ_232:
    {
        rs232_config_t *rs232_config;
    dev_io.io  = eRS232_IO;
    rs232_config  = get_sensor_config(sensor);
    dev_io.handle = (void *)rs232_config->port;
    dev_io.config = (void *)rs232_config;
    hjsnow_init(&dev_io);
    }

    break;
  }
}

int32_t read_snow_485(void)
{

}

int32_t read_sensor_snow(sensor_t *sensor,uint8_t *err)
{
  int32_t snwoFall=0;
  dev_io_t dev_io;
  void *cfg = get_sensor_config(sensor);
  
  //if(cfg == NULL)
  {
    *err = 2;
    return 0;
  }



  switch (sensor->type)
  {
    case S_T_SNOW_HJ_485:
    {
      rs485_config_t *rs485_config=(rs485_config_t *)cfg;;
    dev_io.io = eRS485_IO;
    dev_io.handle = (void *)rs485_config->port;
    snwoFall = read_hjSnowFall(&dev_io,err);
    }
    break;
    case S_T_SNOW_HJ_232:
  {
          rs232_config_t *rs232_config=(rs232_config_t *)cfg;;
    dev_io.io = eRS232_IO;
    dev_io.handle = (void *)rs232_config->port;
    snwoFall = read_hjSnowFall(&dev_io,err);
  }
    break;
  default:
    break;
  }

  return snwoFall;
}