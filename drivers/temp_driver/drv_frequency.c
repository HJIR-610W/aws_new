#include "drv_frequency.h"
#include "bsp_frequency.h"

void drv_frequency_init(void)
{
  bsp_frequency_init();
}

float drv_frequency_read(int channel, uint8_t *err)
{
  return bsp_frequency_read(channel, err);
}

void drv_frequency_start_measurement(int channel)
{
  bsp_frequency_start_measurement(channel);
}

void drv_frequency_stop_measurement(int channel)
{
  bsp_frequency_stop_measurement(channel);
}

bool drv_frequency_is_measuring(int channel)
{
  return bsp_frequency_is_measuring(channel);
}