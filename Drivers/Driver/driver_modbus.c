


#include "driver_modbus.h"



driver_t *driver_modbus_open(int32_t num,void *opt)
{
  driver_t *driver;
  switch (num)
  {
    case DRIVER_MODBUS_RTU_OVER_485:

    break;
  
    case DRIVER_MODBUS_ASCII_OVER_485:

    break;

    case DRIVER_MODBUS_TCP:

    break;
  }  

  return driver;
  
}