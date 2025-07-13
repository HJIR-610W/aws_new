

#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os.h"
#include "dev_io.h"
#include "Sensors\rain\rain.h"
#include "task_isrEvent.h"
#include "pcb_define.h"
#include "driver_di.h"
#include "rain_present.h"


driver_t *rainPresent_open(int32_t num,void *opt)
{
  driver_t *driver=NULL;

  switch(num)
  {
    case RAIN_PRESENT_DI:
      driver = driver_di_open(DI_EXT_5, 0);
      break;
  }

  return driver;

}


bool read_sensor_rainPresent(driver_t *driver,uint8_t *err)
{
  bool data=true;

  if (driver == NULL)
  {
    *err = DRV_ERR_HANDLE;
    return false;
  }


  if(driver_di_read(driver))
  {
    data = false;
  }

  return data;
}