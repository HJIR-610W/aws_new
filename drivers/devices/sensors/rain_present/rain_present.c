




#include "drv_di.h"
#include "rain_present.h"
#include "drv_power.h"
driver_t *rainPresent_open(int32_t num,void *opt)
{
  driver_t *driver=NULL;

  drv_power_on(DRV_POWER_RAIN_DECT_DIGITAL);

      return driver;
}


bool read_sensor_rainPresent(driver_t *driver,uint8_t *err)
{
  bool data=true;
   *err = 0;

  if (drv_di_read(DRV_DI_RAIN_DETECT_DIGITAL) > 0)
  {
    data = false;
  }

  return data;
}