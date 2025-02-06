


#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H


#include "driver_interface.h"


#define DRIVER_STM32_I2C_1 0
#define DRIVER_STM32_I2C_2 1

driver_t *driver_i2c_open(uint32_t num,void *opt);




#endif