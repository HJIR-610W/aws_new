
#ifndef DRIVER_DIGITALIN_H
#define DRIVER_DIGITALIN_H


#include <stdint.h>

#include "cmsis_os.h"
#include "driver_interface.h"



#define DI_ADC_RDY    0
#define DI_RTC_IRQ    1



driver_t *driver_di_open(uint32_t num);
int32_t driver_di_read(driver_t *drv);


#endif




