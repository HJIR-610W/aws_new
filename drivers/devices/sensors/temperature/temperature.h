

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "temperature_define.h"

// ?ъ슜媛?ν븳 ?⑤룄?쇱꽌 紐⑸줉
#ifndef GENERAL_ADC
#define GENERAL_ADC 0
#endif

#ifndef GENERAL_RS485
#define GENERAL_RS485 1
#endif

#define TEMP_PT100_A 100
#define TEMP_PT100_B 101
#define TEMP_HJ_TEMPERATURE 102  // RS485?ы듃 A怨좎젙 ?ъ슜
// ?쇱꽌 紐⑸줉 ??珥?5媛?

// ?먮윭 媛?
#define TEMP_ERR_VAL 1000
driver_t *temperature_open(uint32_t num, void *opt);
float temperature_read(driver_t *driver, uint8_t *err);


#endif