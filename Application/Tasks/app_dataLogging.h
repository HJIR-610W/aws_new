

#ifndef APP_DATALOGGING_H
#define APP_DATALOGGING_H

#include "utile_time.h"
void dataLogging_init(void);

void write_sensorData(DATE_TIME_BUF *pDate, void *pInData,uint32_t dataSize, uint8_t Type,uint32_t periodMin);

#endif