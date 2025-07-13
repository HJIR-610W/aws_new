
#ifndef PCF8575_H
#define PCF8575_H



#include "driver_interface.h"


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

typedef enum {
    PCF8575_OK = 0,
    PCF8575_ERROR_I2C = -1,
    PCF8575_ERROR_INVALID_PIN = -2,
    PCF8575_ERROR_WRONG_DIRECTION = -3,
    PCF8575_ERROR_NOT_INITIALIZED = -4,
    PCF8575_ERROR_INVALID_INSTANCE = -5,
    PCF8575_ERROR_SEMAPHORE = -6
} pcf8575_result_t;

pcf8575_result_t pcf8575_init(void);
int pcf8575_read_pin(int number);
pcf8575_result_t pcf8575_write_pin(int number, int high);




#endif
