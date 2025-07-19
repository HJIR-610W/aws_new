

#ifndef APP_DATALOGGING_H
#define APP_DATALOGGING_H

#include "util_time.h"

#define LOGGING_AWS  0
#define LOGGING_RAIN_1MIN 1
#define LOGGING_SUNSHINE_1MIN 2
void dataLogging_init(void);

int32_t write_data_year(DATE_TIME_BUF *pDate, void *pInData, uint32_t dataSize, uint8_t Type,
                      uint32_t periodMin);
int32_t  write_data_month(DATE_TIME_BUF *p_date, void *p_data, uint16_t dataLen, uint8_t type,
                          uint8_t period_min);
int32_t read_data_month(DATE_TIME_BUF *p_date, void *p_buff,uint16_t readLen, uint8_t type,uint8_t period_min);
int32_t read_data_month_bulk(DATE_TIME_BUF *p_date, void *p_buff, uint16_t readLen, uint8_t type,
  uint8_t period_min,uint16_t read_cnt);

uint32_t timeToOffsetDay(time_t current_tick, uint8_t min, uint16_t byte);
void make_rain_1min_path(uint16_t year, char *buffer, int32_t buffer_size);
void make_sunshine_1min_path(uint16_t year, char *buffer, int32_t buffer_size);

uint8_t read_sensorDataMulti(DATE_TIME_BUF *pDate, uint32_t dataSize, int32_t ReadCnt,
  uint8_t cSystem, uint32_t periodMin, uint8_t *pOutBuff,
  uint32_t buffSize);

void get_filePath(uint8_t type, uint16_t year, uint8_t month, char *pOutBuff, uint32_t buffSize);
#endif