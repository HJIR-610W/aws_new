

#ifndef DRIVER_RTC_DEFINE_H
#define DRIVER_RTC_DEFINE_H

#include "driver_interface.h"
#include "time_define.h"
#include "driver_di_def.h"

typedef struct rtc_set_irq_cfg_s
{
  di_isr_set_cfg_t cfg;
}rtc_set_irq_cfg_t;


typedef enum
{
    eRTC_SET_TIME,
    eRTC_SET_IRQ   
} rtc_set_option_t;


typedef struct
{
  void (*close)(driver_t *handle);
  void (*read)(driver_t *handle,DATE_TIME_BUF *ct);
  void (*set)(driver_t *handle, rtc_set_option_t option, void *value);
}rtc_api_t;

#endif