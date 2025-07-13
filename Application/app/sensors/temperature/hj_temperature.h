
#ifndef HJ_TEMPERATURE_H
#define HJ_TEMPERATURE_H

#include "driver_interface.h"
#include "temperature_define.h"
#include "hj_temperature_define.h"
#define HJ_TEMPERATURE 0



driver_t *hjTemperature_open(int32_t num, void *opt);
float hjTemperature_read(driver_t *driver, uint8_t *err);


driver_t *hjtemp_opened(void);
void hjtemperature_ctrl(driver_t *driver, eHJTEMPERATURE_OPT_t ctrl, void *w_opt, void *r_opt,
  uint8_t *err);
driver_t *get_hjtemperature_bus_io(void);

#endif