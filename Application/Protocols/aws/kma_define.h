
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