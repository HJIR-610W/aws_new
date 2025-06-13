

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "temperature_define.h"

// 사용가능한 온도센서 목록
#ifndef GENERAL_ADC
#define GENERAL_ADC 0
#endif

#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif

#define TEMP_PT100_A 100
#define TEMP_PT100_B 101
#define TEMP_HJ_TEMPERATURE 102  // RS485포트 A고정 사용
// 센서 목록 끝 총 5개

// 에러 값
#define TEMP_ERR_VAL 1000
driver_t *temperature_open(uint32_t num, void *opt);
float temperature_read(driver_t *driver, uint8_t *err);


#endif