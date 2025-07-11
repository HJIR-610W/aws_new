#ifndef OTT_SMP3_DEFINE_H

#define OTT_SMP3_DEFINE_H

#include <stdint.h>

// 湲곕낯 ?덉??ㅽ꽣 ?뺤쓽 (0x04 Read Input Registers ?꾩슜)
#define REG_IO_DEVICE_TYPE 0        // U16: ?μ튂 ???
#define REG_IO_DATAMODEL_VERSION 1  // U16: ?곗씠??紐⑤뜽 踰꾩쟾
#define REG_IO_OPERATIONAL_MODE 2   // U16: ?묐룞 紐⑤뱶 (1=Normal)
#define REG_IO_STATUS_FLAGS 3       // U16: ?곹깭 ?뚮옒洹?
#define REG_IO_SCALE_FACTOR 4       // S16: ?ㅼ????⑺꽣

// ?쇱꽌 ?곗씠??
#define REG_IO_SENSOR1_DATA 5      // S16: 蹂댁젙???쇱궗??(W/m짼)
#define REG_IO_RAW_SENSOR1_DATA 6  // S16: ???곗씠??
#define REG_IO_STDEV_SENSOR1 7     // S16: ?쒖??몄감
#define REG_IO_BODY_TEMPERATURE 8  // S16: 蹂몄껜 ?⑤룄 (0.1째C)
#define REG_IO_EXT_POWER_SENSOR 9  // S16: ?몃? ?꾩썝 ?꾩븬 (0.1V)
#define REG_IO_TILT 15             // U16: 湲곗슱湲?(0.1째)
#define REG_IO_RH 16               // U16: ?대? ?곷??듬룄 (0.1%)

// 怨좎젙?뚯닔?????ㅼ닔 蹂?섏슜 ?ㅼ????⑺꽣
#define SCALE_FACTOR_DIV100 2
#define SCALE_FACTOR_DIV10 1
#define SCALE_FACTOR_NONE 0
#define SCALE_FACTOR_MUL10 -1

// ?ㅼ닔???곗씠??(Floating Point Registers, 32bit, 2?덉??ㅽ꽣 ?ъ슜)

#define REG_U_DEVICE_TYPE 10000       // U16: ?μ튂 ???
#define REG_U_OPERATIONAL_MODE 10001  // U16: ?묐룞 紐⑤뱶
#define REG_U_ERROR_CODE 10002        // U16: 理쒓렐 ?먮윭 肄붾뱶
#define REG_U_STATUS_FLAGS 10003      // U16: ?곹깭 ?뚮옒洹?
#define REG_U_BATCH_NR 10004          // U16: ?쒖“ ?곕룄
#define REG_U_SERIAL_NR 10005         // U16: ?쒕━??踰덊샇
#define REG_FL_SENSOR1_DATA 10006      // F32: ?ㅼ닔??蹂댁젙 ?쇱궗??(W/m짼)
#define REG_FL_STDEV_SENSOR1 10008     // F32: ?ㅼ닔???쒖??몄감
#define REG_FL_BODY_TEMPERATURE 10014  // F32: ?⑤룄 (째K)
#define REG_FL_EXT_POWER_SENSOR 10016  // F32: ?몃? ?꾩븬
#define REG_FL_TILT 10020              // F32: 湲곗슱湲?(째)
#define REG_FL_RH 10022                // F32: ?곷??듬룄 (%)

// 湲고? ?뺣낫 (?쒕━???섎쾭, ?뚯썾????
#define REG_IO_BATCH_NUMBER 41      // U16: ?앹궛 ?곕룄 (YY)
#define REG_IO_SERIAL_NUMBER 42     // U16: ?쒕━???섎쾭
#define REG_IO_SOFTWARE_VERSION 43  // U16
#define REG_IO_HARDWARE_VERSION 44  // U16
#define REG_IO_NODE_ID 45           // U16: Modbus 二쇱냼 (Slave ID)

// 鍮꾪듃?꾨뱶 援ъ“泥??뺤쓽 (珥?16鍮꾪듃)
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
