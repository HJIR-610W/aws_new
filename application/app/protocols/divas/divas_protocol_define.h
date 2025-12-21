
#ifndef DIVAS_PARSE_DIVAS_H_
#define DIVAS_PARSE_DIVAS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
typedef struct
{
  uint8_t STX;
  uint16_t LEN;
  uint8_t SEQ;
  uint16_t Year;
  uint8_t Month;
  uint8_t Day;
  uint8_t Hour;
  uint8_t Min;
  uint8_t Sec;
  uint8_t CMD;
  uint8_t DATA[1];  // 가변 길이 데이터
  // 이후 ETX, SUM
} __attribute__((packed)) divas_frame_t;


uint16_t make_divas_frame(uint8_t cmd, uint8_t *rx_frame, const uint8_t *p_in_data, size_t data_length,
                         uint8_t *p_out_data, size_t out_size);

                         
bool is_divas_frame(uint8_t *p_in_data, size_t data_length);
#endif
