
#ifndef PCF8575_H
#define PCF8575_H


#include "driver_interface.h"
#include "driver_di_def.h"
#include "driver_do_define.h"

#define DI_PCF8575_0 0
#define DI_PCF8575_1 1
#define DI_PCF8575_2 2
#define DI_PCF8575_3 3
#define DI_PCF8575_4 4
#define DI_PCF8575_5 5
#define DI_PCF8575_6 6
#define DI_PCF8575_7 7

#define DO_PCF8575_0 0
#define DO_PCF8575_1 1
#define DO_PCF8575_2 2
#define DO_PCF8575_3 3
#define DO_PCF8575_4 4
#define DO_PCF8575_5 5
#define DO_PCF8575_6 6
#define DO_PCF8575_7 7



driver_t *pcf8575_di_open(uint32_t num,void *opt);
driver_t *pcf8575_do_open(uint32_t num,void *opt);

#endif
