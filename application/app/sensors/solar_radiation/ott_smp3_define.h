#ifndef OTT_SMP3_DEFINE_H

#define OTT_SMP3_DEFINE_H

#include <stdint.h>


#define REG_IO_DEVICE_TYPE 0       
#define REG_IO_DATAMODEL_VERSION 1 
#define REG_IO_OPERATIONAL_MODE 2  
#define REG_IO_STATUS_FLAGS 3      
#define REG_IO_SCALE_FACTOR 4      
#define REG_IO_SENSOR1_DATA 5     
#define REG_IO_RAW_SENSOR1_DATA 6 
#define REG_IO_STDEV_SENSOR1 7     
#define REG_IO_BODY_TEMPERATURE 8 
#define REG_IO_EXT_POWER_SENSOR 9   
#define REG_IO_TILT 15           
#define REG_IO_RH 16                


#define SCALE_FACTOR_DIV100 2
#define SCALE_FACTOR_DIV10 1
#define SCALE_FACTOR_NONE 0
#define SCALE_FACTOR_MUL10 -1



#define REG_U_DEVICE_TYPE 10000      
#define REG_U_OPERATIONAL_MODE 10001 
#define REG_U_ERROR_CODE 10002     
#define REG_U_STATUS_FLAGS 10003     
#define REG_U_BATCH_NR 10004        
#define REG_U_SERIAL_NR 10005      
#define REG_FL_SENSOR1_DATA 10006   
#define REG_FL_STDEV_SENSOR1 10008     
#define REG_FL_BODY_TEMPERATURE 10014  
#define REG_FL_EXT_POWER_SENSOR 10016 
#define REG_FL_TILT 10020           
#define REG_FL_RH 10022          
           
#define REG_IO_BATCH_NUMBER 41     
#define REG_IO_SERIAL_NUMBER 42   
#define REG_IO_SOFTWARE_VERSION 43
#define REG_IO_HARDWARE_VERSION 44 
#define REG_IO_NODE_ID 45

// Modbus Coil Address Definitions
#define COIL_IO_CLEAR_ERROR 10     // 정상 동작 선택 및 에러 클리어 (1=에러 클리어)
#define COIL_IO_RESTART_MODBUS 18  // Modbus 프로토콜로 디바이스 재시작
#define COIL_IO_ROUND 20           // 센서 데이터 반올림 활성화 (1=활성화)
#define COIL_IO_AUTO_RANGE 21      // 오토 레인지 모드 활성화 (1=활성화)
#define COIL_IO_FASTRESPONSE 22    // 빠른 응답 필터 활성화 (1=활성화)
#define COIL_IO_TRACKING_FILTER 23 // 트래킹 필터 활성화 (1=활성화)

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
