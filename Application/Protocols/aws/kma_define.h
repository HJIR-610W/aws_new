
#ifndef KMA_DEFINE_H
#define KMA_DEFINE_H

#include <stdint.h>

#define KMA_HEADER_START 0xFAFB
#define KMA_HEADER_END 0xFFFE


// 규격서 자료형식 번호
#define DATA_TYPE_UNUSED_0 0      // 미사용 (하위호환성)
#define DATA_TYPE_UNUSED_1 1      // 미사용 (하위호환성)
#define DATA_TYPE_UNUSED_2 2      // 미사용 (하위호환성)
#define DATA_TYPE_GENERAL 3       // 일반용
#define DATA_TYPE_AGRICULTURAL 4  // 농관용
#define DATA_TYPE_OBSERVATION 5   // 관측요소에 따라 부여 (5～255 범위)


typedef struct
{
  uint16_t header_start;    // FAFB
  uint8_t protocol_ver_yy;  // 프로토콜버전 년
  uint8_t protocol_ver_mm;  // 프로토콜버전 월
  uint8_t protocol_ver_dd;  // 프로토콜버전 일
  uint8_t date_yy;          // C.1) 년
  uint8_t date_mm;          // C.2) 월
  uint8_t date_dd;          // C.3) 일
  uint8_t time_hh;          // C.4) 시
  uint8_t time_mm;          // C.5) 분
  uint8_t time_ss;          // C.6) 초
  uint16_t password;        // D. 비밀번호
  uint16_t station_id;      // E. 지점번호
  char command_str[10];     // F. 명령어
  uint8_t checksum_xor;     // G. CHECK XOR
  uint8_t checksum_sum;     // G. CHECK SUM
  uint16_t header_end;      // H. FFFE
} kma2_command_request_t;

typedef struct
{
  uint16_t header;
  uint8_t protocol_year;
  uint8_t protocol_month;
  uint8_t protocol_day;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint16_t password;
  uint16_t station_id;
  char command_str[10];
  uint16_t crc;
  uint16_t end;
} kma3_command_request_t;

#endif