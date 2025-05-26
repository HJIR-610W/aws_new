#ifndef OTT_SMP3_DEFINE_H

#define OTT_SMP3_DEFINE_H

#include <stdint.h>

// 기본 레지스터 정의 (0x04 Read Input Registers 전용)
#define REG_IO_DEVICE_TYPE 0        // U16: 장치 타입
#define REG_IO_DATAMODEL_VERSION 1  // U16: 데이터 모델 버전
#define REG_IO_OPERATIONAL_MODE 2   // U16: 작동 모드 (1=Normal)
#define REG_IO_STATUS_FLAGS 3       // U16: 상태 플래그
#define REG_IO_SCALE_FACTOR 4       // S16: 스케일 팩터

// 센서 데이터
#define REG_IO_SENSOR1_DATA 5      // S16: 보정된 일사량 (W/m²)
#define REG_IO_RAW_SENSOR1_DATA 6  // S16: 원 데이터
#define REG_IO_STDEV_SENSOR1 7     // S16: 표준편차
#define REG_IO_BODY_TEMPERATURE 8  // S16: 본체 온도 (0.1°C)
#define REG_IO_EXT_POWER_SENSOR 9  // S16: 외부 전원 전압 (0.1V)
#define REG_IO_TILT 15             // U16: 기울기 (0.1°)
#define REG_IO_RH 16               // U16: 내부 상대습도 (0.1%)

// 고정소수점 → 실수 변환용 스케일 팩터
#define SCALE_FACTOR_DIV100 2
#define SCALE_FACTOR_DIV10 1
#define SCALE_FACTOR_NONE 0
#define SCALE_FACTOR_MUL10 -1

// 실수형 데이터 (Floating Point Registers, 32bit, 2레지스터 사용)

#define REG_U_DEVICE_TYPE 10000       // U16: 장치 타입
#define REG_U_OPERATIONAL_MODE 10001  // U16: 작동 모드
#define REG_U_ERROR_CODE 10002        // U16: 최근 에러 코드
#define REG_U_STATUS_FLAGS 10003      // U16: 상태 플래그
#define REG_U_BATCH_NR 10004          // U16: 제조 연도
#define REG_U_SERIAL_NR 10005         // U16: 시리얼 번호
#define REG_FL_SENSOR1_DATA 10006      // F32: 실수형 보정 일사량 (W/m²)
#define REG_FL_STDEV_SENSOR1 10008     // F32: 실수형 표준편차
#define REG_FL_BODY_TEMPERATURE 10014  // F32: 온도 (°K)
#define REG_FL_EXT_POWER_SENSOR 10016  // F32: 외부 전압
#define REG_FL_TILT 10020              // F32: 기울기 (°)
#define REG_FL_RH 10022                // F32: 상대습도 (%)

// 기타 정보 (시리얼 넘버, 펌웨어 등)
#define REG_IO_BATCH_NUMBER 41      // U16: 생산 연도 (YY)
#define REG_IO_SERIAL_NUMBER 42     // U16: 시리얼 넘버
#define REG_IO_SOFTWARE_VERSION 43  // U16
#define REG_IO_HARDWARE_VERSION 44  // U16
#define REG_IO_NODE_ID 45           // U16: Modbus 주소 (Slave ID)

// 비트필드 구조체 정의 (총 16비트)
typedef struct
{
  uint16_t signal_quality : 1;        // Bit 0
  uint16_t overflow_error : 1;        // Bit 1
  uint16_t underflow_error : 1;       // Bit 2
  uint16_t general_error : 1;         // Bit 3
  uint16_t adc_error : 1;             // Bit 4
  uint16_t dac_error : 1;             // Bit 5
  uint16_t calibration_error : 1;     // Bit 6
  uint16_t eeprom_update_error : 1;   // Bit 7
  uint16_t power_failure_error : 1;   // Bit 8
  uint16_t tilt_sensor_error : 1;     // Bit 9
  uint16_t rh_sensor_error : 1;       // Bit 10
  uint16_t rh_threshold_warning : 1;  // Bit 11
  uint16_t body_temp_error : 1;       // Bit 12
  uint16_t reserved : 3;              // Bits 13~15
} status_flags_t;

#endif
