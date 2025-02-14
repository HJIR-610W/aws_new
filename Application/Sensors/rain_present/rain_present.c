

#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os.h"
#include "dev_io.h"
#include "Sensors\rain\rain.h"
#include "task_isrEvent.h"
#include "pcb_define.h"
#include "driver_di.h"


driver_t *rainPresent;

bool rainPresentInit=false;


bool is_rainPresentInit(void)
{
  return rainPresentInit;
}

bool rainPresent_deInit(void)
{
  rainPresentInit = false;
}

void rainPresent_init(void)
{
  rainPresentInit = true;
  rainPresent = driver_di_open(DI_EXT_0,0);
}

bool read_sensor_rainPresent(sensor_t *sensor,uint8_t *err)
{
  bool data=false;

  if(driver_di_read(rainPresent))
  {
    data = true;
  }

  return data;
}