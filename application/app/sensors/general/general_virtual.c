


#include "general_virtual.h"
#include "app_sensor.h"

typedef struct general_v_cfg_s
{
  float data;//-16,777,216 ~ 16,777,216  24bit,0.001 ~ 9999.999
}general_v_cfg_t;

driver_t g_general_virtual[SENSOR_LIST_MAX];
general_v_cfg_t general_v_cfg[SENSOR_LIST_MAX];

driver_t *general_v_open(int32_t n,void *opt)
{
  
  eSENSOR_TYPE_t type =(eSENSOR_TYPE_t)(int)opt;

  if(g_general_virtual[type].opened)
  {
    return &g_general_virtual[type];
  }

  g_general_virtual[type].name = "GENERAL_V";
  g_general_virtual[type].opened = true;
  g_general_virtual[type].cfg = &general_v_cfg[type];


  return &g_general_virtual[type];
}



float general_v_read(driver_t *drv,uint8_t *err)
{
   general_v_cfg_t *cfg = drv->cfg;

    *err = 0;
 
  return cfg->data;
}

void general_v_set(driver_t *drv,float data)
{
  general_v_cfg_t *cfg = drv->cfg;

  cfg->data = data;
}