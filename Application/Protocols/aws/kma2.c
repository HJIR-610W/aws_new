
#include <stdint.h>
#include <string.h>

#include "aws_data.h"
#include "config_app.h"
#include "crc16_ccitt.h"
#include "kma2.h"
#include "kma_define.h"
#include "utile.h"
#include "utile_time.h"


#define KMA2_DATA_CONTENT_ESSENTIAL_SELECTIVE_LEN 91  // 필수 및 선택 모두 관측 시
#define KMA2_DATA_CONTENT_ESSENTIAL_LEN 45            // 필수 관측 시
#define KMA2_DATA_CONTENT_PRECIPITATION_LEN 16        // 강수량 관측 시

uint16_t make_kma2_essential(uint8_t *buffer, kma2_response_t *p_aws)
{
  uint16_t offset=0;

  SetWord(&buffer[offset], p_aws->temperature);
  offset += 2;
  SetWord(&buffer[offset], p_aws->wind_direction_avg);
  offset += 2;
  SetWord(&buffer[offset], p_aws->wind_speed_avg);
  offset += 2;
  SetWord(&buffer[offset], p_aws->wind_direction_instant);
  offset += 2;
  SetWord(&buffer[offset], p_aws->wind_speed_instant);
  offset += 2;
  SetWord(&buffer[offset], p_aws->precipitation);
  offset += 2;
  SetWord(&buffer[offset], p_aws->pressure);
  offset += 2;
  SetWord(&buffer[offset], p_aws->precipitation_presence);
  offset += 2;
  SetWord(&buffer[offset], p_aws->snowfall);
  offset += 2;
  SetWord(&buffer[offset], p_aws->relative_humidity);
  offset += 2;
  SetWord(&buffer[offset], p_aws->precipitation_fine);
  offset += 2;

  SetWord(&buffer[offset], 0);  // 1
  offset += 2;
  SetWord(&buffer[offset], 0);  // 2
  offset += 2;
  SetWord(&buffer[offset], 0);  // 3
  offset += 2;
  SetWord(&buffer[offset], 0);  // 4
  offset += 2;
  SetWord(&buffer[offset], 0);  // 5
  offset += 2;
  SetWord(&buffer[offset], 0);  // 6
  offset += 2;
  SetWord(&buffer[offset], 0);  // 7
  offset += 2;
  SetWord(&buffer[offset], 0);  // 8
  offset += 2;
  SetWord(&buffer[offset], 0);  // 9
  offset += 2;
  SetWord(&buffer[offset], 0);  // 10
  offset += 2;

  buffer[offset++] = p_aws->X_voltage_status;
  SetWord(&buffer[offset], p_aws->Y_logger_status); 
  offset += 2;

  return offset;
}

uint16_t make_kma2_essential_selective(uint8_t *buffer, kma2_response_t *p_aws)
{
  uint16_t offset = 0;

  SetWord(&buffer[offset], p_aws->temperature);
  offset += 2;
  SetWord(&buffer[offset], p_aws->wind_direction_avg);
  offset += 2;
  SetWord(&buffer[offset], p_aws->wind_speed_avg);
  offset += 2;
  SetWord(&buffer[offset], p_aws->wind_direction_instant);
  offset += 2;
  SetWord(&buffer[offset], p_aws->wind_speed_instant);
  offset += 2;
  SetWord(&buffer[offset], p_aws->precipitation);
  offset += 2;
  SetWord(&buffer[offset], p_aws->pressure);
  offset += 2;
  SetWord(&buffer[offset], p_aws->precipitation_presence);
  offset += 2;
  SetWord(&buffer[offset], p_aws->snowfall);
  offset += 2;
  SetWord(&buffer[offset], p_aws->relative_humidity);
  offset += 2;
  SetWord(&buffer[offset], p_aws->precipitation_fine);
  offset += 2;

  SetWord(&buffer[offset], p_aws->L1);  // 1
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 2
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 3
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 4
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 5
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 6
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 7
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 8
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 9
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);  // 10
  offset += 2;

  SetWord(&buffer[offset], p_aws->solar_radiation);
  offset += 2;
  SetWord(&buffer[offset], p_aws->sunshine_duration);
  offset += 2;
  SetWord(&buffer[offset], p_aws->surface_temperature);
  offset += 2;
  SetWord(&buffer[offset], p_aws->grass_temperature);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_5cm);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_10cm);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_20cm);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_30cm);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_50cm);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_1m);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_1_5m);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_3m);
  offset += 2;
  SetWord(&buffer[offset], p_aws->soil_temperature_5m);
  offset += 2;

  SetWord(&buffer[offset], p_aws->S1);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S2);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S3);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S4);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S5);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S6);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S7);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S8);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S9);  // 10
  offset += 2;
  SetWord(&buffer[offset], p_aws->S10);  // 10
  offset += 2;


  buffer[offset++] = p_aws->X_voltage_status;
  SetWord(&buffer[offset], p_aws->Y_logger_status);
  offset += 2;

  return offset;
}
uint16_t make_kma2_precipitation(uint8_t *buffer, kma2_response_t *p_aws)
{
  uint16_t offset = 0;

  SetWord(&buffer[offset], p_aws->precipitation);  
  offset += 2;
  SetWord(&buffer[offset], p_aws->precipitation_fine);
  offset += 2;
  SetWord(&buffer[offset], p_aws->L1);
  offset += 2;
  SetWord(&buffer[offset], p_aws->L2);
  offset += 2;
  SetWord(&buffer[offset], p_aws->L3);
  offset += 2;
  SetWord(&buffer[offset], p_aws->L4);
  offset += 2;
  SetWord(&buffer[offset], p_aws->L5);
  offset += 2;

  buffer[offset++] = p_aws->X_voltage_status;
  buffer[offset++] =  p_aws->Z_logger_status;

  return offset;

}

uint16_t make_kma2_response(uint8_t *rx_frame, uint8_t *data, uint16_t data_len,
                            uint8_t data_format_no, uint8_t *tx_frame)
{
  uint16_t offset = 0;
  uint16_t station_id;
  uint16_t check_len;


  SetWord(&tx_frame[offset], KMA_HEADER_START);
  offset += 2;

  tx_frame[offset++] = rx_frame[2];  // 프로토콜 버전 년
  tx_frame[offset++] = rx_frame[3];  // 프로토콜 버전 월
  tx_frame[offset++] = rx_frame[4];  // 프로토콜 버전 일

  tx_frame[offset++] = Date_Time.Year%100;
  tx_frame[offset++] = Date_Time.Month;
  tx_frame[offset++] = Date_Time.Day;
  tx_frame[offset++] = Date_Time.Hour;
  tx_frame[offset++] = Date_Time.Min;
  tx_frame[offset++] = rx_frame[16];  //명령어[1]

  tx_frame[offset++] = data_format_no;

  station_id = GetWord((uint8_t *)&rx_frame[13]);

  SetWord(&tx_frame[offset], station_id);
  offset +=2;

  memcpy(&tx_frame[offset],data,data_len);
  offset += data_len;

  check_len =  offset;

  tx_frame[offset++] = calculate_xor_checksum(&tx_frame[2], check_len - 2);
  tx_frame[offset++] = make_sum(&tx_frame[2], check_len - 2);

  SetWord(&tx_frame[offset], KMA_HEADER_END);
  offset += 2;

  return offset;
}