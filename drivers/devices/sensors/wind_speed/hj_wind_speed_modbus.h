
#ifndef HJ_WIND_SPEED_MODBUS_HHH
#define HJ_WIND_SPEED_MODBUS_HHH

#include <stdint.h>
#include "modbus_master.h"


#define REG_R_WIND_DATA 0 //풍속,풍향 데이터
#define REG_R_VERSION   1 //버전 정보 읽기 전용
#define REG_R_STATUS    2 //센서 상태
#define REG_R_TYPE      3 // 1: 풍속, 2: 풍향

#define REG_RW_ID       10 //설정 값
#define REG_RW_FULLSET  11 //설정 값
#define REG_RW_OFFSET   12 //설정 값




int32_t hj_wind_speed_init(void *opt);
float hj_wind_speed_read(uint8_t *err);

modbus_h_t* hj_wind_speed_get_bus_io(void);


#endif
