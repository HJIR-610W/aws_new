

#ifndef APP_DATALOGGING_H
#define APP_DATALOGGING_H

#include "utile_time.h"

#define LOGGING_AWS 0
void dataLogging_init(void);


int32_t write_data(DATE_TIME_BUF *p_date, void *p_data,uint16_t dataLen, uint8_t type,uint8_t period_min);
int32_t read_data(DATE_TIME_BUF *p_date, void *p_buff,uint16_t readLen, uint8_t type,uint8_t period_min);

#endif