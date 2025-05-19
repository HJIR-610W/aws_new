#ifndef KMA3_PROTOCOL_H
#define KMA3_PROTOCOL_H

#include <stdint.h>
#include "aws_data.h"
#include "app_sensor.h"

// 규격서 프로토콜 버전
#define KMA3_PROTOCOL_YEAR 2018
#define KMA3_PROTOCOL_MONTH 2
#define KMA3_PROTOCOL_DAY 1

// 횡성 기상청 관련 업체 테스트시 캡쳐한 프레임
// FA FB 11 0A 18 18 0C 06 10 07 00 01 4F 01 4F 41 42 3F 00 00 00 00 00 00 00 18 BD FF FE
// 년 17
// 월 10
// 일 24

uint32_t make_kma3_data_unusedSesor(uint8_t *lpSend, uint16_t lpSendSize, kma_data_ex_t *aws);
uint16_t make_kma3_resp(uint8_t *out, char dataType,  uint8_t dataNum, uint16_t id,
  uint8_t *data, uint16_t dataLen);

uint16_t make_kma3_resp_RODTWC(uint8_t *out, uint16_t outSize, uint16_t id, uint8_t cmd,
    const char *result);


void kma_update_sensor_err(eSENSOR_LIST_t sensor_num, uint8_t err);
void kma_update_sensor_err(eSENSOR_LIST_t sensor_num, uint8_t err);
bool kma_is_sensor_error(eSENSOR_LIST_t sensor_num);
#endif