




#include "drv_di.h"
#include "rain_present.h"
#include "drv_power.h"

typedef struct 
{
  uint8_t channel;
}rain_present_cfg_t;

rain_present_cfg_t g_rain_present_cfg;
driver_t g_rain_present_driver;

driver_t *rainPresent_open(int32_t num, void *opt)
{
 // driver_t *driver=NULL;

  if (g_rain_present_driver.opened)
  {
    return &g_rain_present_driver;
  }

  g_rain_present_driver.cfg = &g_rain_present_cfg;
  switch (num)
  {
  case RAIN_PRESENT_DI:
    drv_power_on(DRV_POWER_RAIN_DECT_DIGITAL);
    g_rain_present_cfg.channel = RAIN_PRESENT_DI;
    break;
  case RAIN_PRESENT_ANALOG:
    drv_power_on(DRV_POWER_RAIN_DECT_ANALOG);
    g_rain_present_cfg.channel = RAIN_PRESENT_ANALOG;
    break;
  
  default:
    break;
  }
g_rain_present_driver.opened = true;

      return &g_rain_present_driver;
}


bool read_sensor_rainPresent(driver_t *driver,uint8_t *err)
{
  rain_present_cfg_t *p_cfg = driver->cfg;

  bool data=true;
   *err = 0;

  switch (p_cfg->channel)
  {
  case RAIN_PRESENT_DI:
    if (drv_di_read(DRV_DI_RAIN_DETECT_DIGITAL) > 0)
    {
      data = false;
    }
    break;
  case RAIN_PRESENT_ANALOG:
    if (drv_di_read(DRV_DI_DI_RAIN_DETECT_ANALOG) > 0)
    {
      data = false;
    }
  default:
    break;
  }


  return data;
}