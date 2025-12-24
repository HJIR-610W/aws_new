
#include <math.h>
#include <string.h>


#include "config_app.h"
#include "app_adc.h"
#include "Sensors\barometer\barometer.h"
#include "Sensors\general\general_adc.h"
#include "sensors\barometer\jinsung_sjgp215.h"
#include "sensors\barometer\barometer_rmyoung_61402v.h"
#include "sensors\barometer\barometer_rmyoung_61402v_rs232.h"
driver_t *barometer_open(int32_t num,void *opt)
{
  driver_t *driver=NULL;

  switch (num)
  {
  case GENERAL_ADC:
    driver = general_adc_open(GENERAL_ADC,opt,"Pressure");
    break;
  case BARO_JINSUNG_SJGP215:
    sjgp215_init(opt);
    driver = (driver_t *)sjgp215_init;
    break;
  case BAROMETER_RMYOUNG_61402V:
    rmyoung_61402v_init(opt);
    driver = (driver_t *)rmyoung_61402v_init;
    break;
    case BAROMETER_RMYOUNG_61402V_RS232:
    rmyoung_61402v_rs232_init(opt);
    driver = (driver_t *)rmyoung_61402v_rs232_init;
    break;
  }
  
  return driver;
}

float read_sensor_barometer(driver_t *driver,uint8_t *err)
{
  if(driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return NAN;
  }

  
  if (driver == (driver_t *)sjgp215_init)
  {
    return read_sjgp215_baromater(err);
  }
  else if (driver == (driver_t *)rmyoung_61402v_init)
  {
    return read_baromater_rmyoung_61402v(err);
  }
  else if (driver == (driver_t *)rmyoung_61402v_rs232_init)
  {
    return read_rmyoung_61402v_rs232_baromater(err);
  }
  

  if (strncmp(driver->name, "GENERAL_ADC", 11) == 0)
  {
    return general_adc_read(driver, err);
  }

  *err = DRV_ERR_HANDLE;

  return NAN;
}
