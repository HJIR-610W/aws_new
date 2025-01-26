

#include "Sensors\snow\snow.h"
#include "driver_uart.h"

#include "app_sensor.h"
#include "hj_snow.h"
#include "dev_io.h"


void snow_init(sensor_t *sensor)
{
  dev_io_t dev_io;
  rs232_config_t *rs232_config;
  
  switch (sensor->type)
  {
    case S_T_SNOW_HJ_485:
    dev_io.io  = eRS485_IO;
    rs232_config = get_sensor_config(sensor,S_T_SNOW_HJ_485);
    dev_io.handle = (void *)rs232_config->port;
    dev_io.config = (void *)&rs232_config;
    hjsnow_init(&dev_io);
    break;
    case S_T_SNOW_HJ_232:
    dev_io.io  = eRS232_IO;
    rs232_config  = get_sensor_config(sensor,S_T_SNOW_HJ_232);
    dev_io.handle = (void *)rs232_config->port;
    dev_io.config = (void *)rs232_config;
    hjsnow_init(&dev_io);
    break;
  }
}

int32_t read_snow_485(void)
{

}

int32_t read_sensor_snow(sensor_t *sensor,uint8_t *err)
{
  int32_t snwoFall;
  dev_io_t dev_io;
rs232_config_t *rs232_config;

  switch (sensor->type)
  {
    case S_T_SNOW_HJ_485:
    rs232_config = get_sensor_config(sensor,S_T_SNOW_HJ_485);
    dev_io.io = eRS485_IO;
    dev_io.handle = (void *)rs232_config->port;
    snwoFall = read_hjSnowFall(&dev_io,err);
    break;
    case S_T_SNOW_HJ_232:
    rs232_config = get_sensor_config(sensor,S_T_SNOW_HJ_232);
    dev_io.io = eRS232_IO;
    dev_io.handle = (void *)rs232_config->port;
    snwoFall = read_hjSnowFall(&dev_io,err);
    break;
  default:
    break;
  }

  return snwoFall;
}