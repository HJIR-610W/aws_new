
#ifndef AWS_KMA3_H
#define AWS_KMA3_H

#include <stdint.h>

#include "app_sensor.h"

void kma_update_sensor_err(eSENSOR_LIST_t sensor_num, uint8_t err);
bool kma_is_sensor_error(eSENSOR_LIST_t sensor_num);

uint32_t
    kma2_cmd_handler(uint8_t *packet, uint16_t len, uint8_t *txBuff, uint16_t tSise,
                     uint8_t source);
#endif