
#ifndef AWS_KMA3_H
#define AWS_KMA3_H

#include <stdint.h>

#include "app_sensor.h"

#define KMA_TX_BUFFER_SIZE 400

typedef enum req_source_e
{
  eREQ_SOURCE_ETH,
  eREQ_SOURCE_DIRECT,
  eREQ_SOURCE_CDMA,
}eREQ_SOURCE_t;

int32_t kma_cmd_handler(uint8_t *rx_frame, size_t rx_len, uint8_t *tx_frame, eREQ_SOURCE_t source);

#endif