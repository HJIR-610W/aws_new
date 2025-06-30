
#ifndef KMA_DEFINE_H
#define KMA_DEFINE_H

#include <stdint.h>
#include <stdbool.h>

#define KMA_HEADER_START 0xFAFB
#define KMA_HEADER_END 0xFFFE


// 규격서 자료형식 번호
#define DATA_TYPE_UNUSED_0 0      // 미사용 (하위호환성)
#define DATA_TYPE_UNUSED_1 1      // 미사용 (하위호환성)
#define DATA_TYPE_UNUSED_2 2      // 미사용 (하위호환성)
#define DATA_TYPE_GENERAL 3       // 일반용
#define DATA_TYPE_AGRICULTURAL 4  // 농관용
#define DATA_TYPE_OBSERVATION 5   // 관측요소에 따라 부여 (5～255 범위)

#pragma pack(push, 1)
// KMA2 구조체 정의 (이전과 동일)
typedef struct
{
  uint16_t header_start;
  uint8_t protocol_yy;  // ... (이하 필드 동일)
  uint8_t protocol_mm;
  uint8_t protocol_dd;
  uint8_t date_yy;
  uint8_t date_mm;
  uint8_t date_dd;
  uint8_t time_hh;
  uint8_t time_mm;
  uint8_t time_ss;
  uint16_t password;
  uint16_t station_id;
  char command_str[10];
  uint8_t checksum_xor;
  uint8_t checksum_sum;
  uint16_t header_end;
} kma2_command_request_t;

typedef struct
{
  uint16_t header_start;
  uint8_t protocol_yy;  // ... (이하 필드 동일)
  uint8_t protocol_mm;
  uint8_t protocol_dd;
  uint8_t date_yy;
  uint8_t date_mm;
  uint8_t date_dd;
  uint8_t time_hh;
  uint8_t time_mm;
  uint8_t time_ss;
  uint16_t password;
  uint16_t station_id;
  char command_str[10];
  uint16_t crc;
  uint16_t header_end;
} kma3_command_request_t;

typedef struct
{
  uint16_t start_mark;
  uint8_t protocol_ver_yy;  // ... (이하 필드 동일)
  uint8_t protocol_ver_mm;
  uint8_t protocol_ver_dd;
  uint8_t date_yy;
  uint8_t date_mm;
  uint8_t date_dd;
  uint8_t time_hh;
  uint8_t time_mm;
  uint8_t data_type_char;
  uint8_t data_format_no;
  uint16_t station_id;
} kma2_observation_packet_header_t;

typedef struct
{
  uint8_t checksum_xor;  // ... (이하 필드 동일)
  uint8_t checksum_sum;
  uint16_t end_mark;
} kma2_observation_packet_footer_t;

typedef struct
{
  float temperature;  // ... (이하 필드 및 valid 플래그 동일)
  float wind_direction_avg;
  float wind_speed_avg;
  float gust_wind_direction;
  float gust_wind_speed;
  float precipitation_0_5mm;
  float pressure;
  uint16_t precipitation_presence;
  float snowfall_accum;
  float relative_humidity;
  float precipitation_0_1mm;
  float solar_radiation_mj;
  uint32_t sunshine_duration_sec;
  float surface_temperature;
  float grass_temperature;
  float soil_temp_5cm;
  float soil_temp_10cm;
  float soil_temp_20cm;
  float soil_temp_30cm;
  float soil_temp_50cm;
  float soil_temp_1m;
  float soil_temp_1_5m;
  float soil_temp_3m;
  float soil_temp_5m;
  uint16_t raw_L[10];
  uint16_t raw_S[10];
  uint8_t status_X;
  uint16_t status_Y;
  uint8_t status_Z;
  bool valid_A, valid_B, valid_C, valid_D, valid_E, valid_F, valid_G, valid_H, valid_I, valid_J,
      valid_K;
  bool valid_a, valid_b, valid_c, valid_d, valid_e_m;
  bool valid_L[10], valid_S[10];
  bool valid_X, valid_Y, valid_Z;
} kma2_observation_fields_t;
#pragma pack(pop)

// voltage status 8bit
//  BIT 0: DC 입력 전압 (0: 정상, 1: 비정상)
#define KMA2_PWRSTAT_DC_INPUT_ERR 0x01  // 0000 0001

// BIT 1: 배터리 전압 (0: 정상, 1: 비정상)
#define KMA2_PWRSTAT_BATTERY_ERR 0x02  // 0000 0010

// BIT 2~3: AC 전압 상태 (00: 110V, 01: 220V, 11: AC OFF)
#define KMA2_PWRSTAT_AC_MASK 0x0C  // 0000 1100
#define KMA2_PWRSTAT_AC_110V 0x00  // 0000 0000
#define KMA2_PWRSTAT_AC_220V 0x04  // 0000 0100
#define KMA2_PWRSTAT_AC_OFF 0x0C   // 0000 1100

// BIT 4: 데이터로거함 잠금 상태 (0: 닫힘, 1: 열림)
#define KMA2_PWRSTAT_DOOR_OPEN 0x10  // 0001 0000

// BIT 5~7: 예비 1~3 (0: 정상, 1: 비정상)
#define KMA2_PWRSTAT_SPARE1_ERR 0x20  // 0010 0000
#define KMA2_PWRSTAT_SPARE2_ERR 0x40  // 0100 0000
#define KMA2_PWRSTAT_SPARE3_ERR 0x80  // 1000 0000

#endif