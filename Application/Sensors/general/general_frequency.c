
#include "driver_freqInput.h"

#include "app_sensor.h"
#include "config_sensor.h"

#include "general_frequency.h"

#include <math.h>
typedef struct general_freq_s
{
  driver_t *freq_io;
  float scale_factor;
  int channel;
} general_freq_cfg_t;

general_freq_cfg_t freq_cfg[2];
driver_t g_general_freq[2];

#define FREQ_A 0
#define FREQ_B 1

driver_t *general_freq_open(int channel,void *opt)
{
  frequency_config_t *config = (frequency_config_t *)opt;

  switch (config->channel)
  {
    case FREQ_A:
      if (g_general_freq[FREQ_A].opened)
      {
        return &g_general_freq[FREQ_A];
      }
      g_general_freq[FREQ_A].name = "GENERAL_FREQ_A";
      g_general_freq[FREQ_A].driver_type = eDRIVER_GENERAL_FREQ;
      g_general_freq[FREQ_A].opened = true;
      freq_cfg[FREQ_A].scale_factor = config->scale_factor;
      freq_cfg[FREQ_A].freq_io = driver_freq_open(FREQ_MEAURE_B);
      g_general_freq[FREQ_A].cfg = &freq_cfg[FREQ_A];

      break;
    case FREQ_B:
      if (g_general_freq[FREQ_B].opened)
      {
        return &g_general_freq[FREQ_B];
      }
      g_general_freq[FREQ_B].name = "GENERAL_FREQ_B";
      g_general_freq[FREQ_B].driver_type = eDRIVER_GENERAL_FREQ;
      g_general_freq[FREQ_B].opened = true;
      freq_cfg[FREQ_B].scale_factor = config->scale_factor;
      freq_cfg[FREQ_B].freq_io = driver_freq_open(FREQ_MEAURE_C);
      g_general_freq[FREQ_B].cfg = &freq_cfg[FREQ_B];

      break;
  }

  return &g_general_freq[config->channel];
}


float general_freq_read(driver_t *drv,uint8_t *err)
{
  general_freq_cfg_t *cfg = (general_freq_cfg_t *)drv->cfg;
  float freq;
  float data;


  freq = driver_freq_read(cfg->freq_io);

  if (isfinite(freq))
  {
    data = freq*cfg->scale_factor;

    *err = DRV_ERR_NONE;
  }
  else
  {
    *err = DRV_ERR_DATA_NAN;
  }

  return data;

}