

#ifndef drv_rtc_define_H
#define drv_rtc_define_H

#include "driver_interface.h"
#include "drv_di_def.h"
#include "util_time.h"


typedef enum
{
    eRTC_SET_TIME,
    eRTC_SET_IRQ   
} rtc_set_option_t;


typedef struct
{
  void (*close)(driver_t *handle);
  int32_t (*read)(driver_t *handle,DATE_TIME_BUF *ct);
  void (*set)(driver_t *handle, rtc_set_option_t option, void *value);
}rtc_api_t;

#endif