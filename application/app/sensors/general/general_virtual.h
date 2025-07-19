

#ifndef GENERAL_VIRTUAL_H
#define GENERAL_VIRTUAL_H


#include "driver_interface.h"

#define GENERAL_V   2


driver_t *general_v_open(int32_t n,void *opt);
float general_v_read(driver_t *drv,uint8_t *err);
void general_v_set(driver_t *drv,float data);

#endif