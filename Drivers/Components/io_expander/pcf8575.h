
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

#define DO_PCF8575_0 8
#define DO_PCF8575_1 9
#define DO_PCF8575_2 10
#define DO_PCF8575_3 11
#define DO_PCF8575_4 12
#define DO_PCF8575_5 13
#define DO_PCF8575_6 14
#define DO_PCF8575_7 15



void pcf8575_init(void);
int32_t pcf8575_read_pin(int number);
int32_t pcf8575_write_pin(int number, int high);

#endif
