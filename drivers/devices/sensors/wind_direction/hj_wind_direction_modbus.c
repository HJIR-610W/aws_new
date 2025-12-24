

#include "hj_wind_direction_modbus.h"

#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "driver_interface.h"
#include "cmsis_os.h"
#include "app_sensor.h"
#include "config_sensor.h"
#include "modbus_master.h"
#include "debug_io.h"
#include "drv_rs485.h"
#include "system_err.h"

#define REG_R_WIND_DATA 0 //풍속,풍향 데이터
#define REG_R_VERSION   1 //버전 정보 읽기 전용
#define REG_R_STATUS    2 //센서 상태
#define REG_R_TYPE      3 // 1: 풍속, 2: 풍향

#define REG_RW_ID       10 //설정 값
#define REG_RW_FULLSET  11 //설정 값
#define REG_RW_OFFSET   12 //설정 값


typedef struct hj_wind_dir_instance_s
{
  bool opened;
  modbus_h_t modbus;
} hj_wind_dir_instance_t;


hj_wind_dir_instance_t g_hj_wind_dir_inst;;


int32_t hj_wind_direction_init(void *opt)
{
  uart_config_t uart_config;
  wind_direction_hj_config_t *ott = (wind_direction_hj_config_t *)opt;

  if(g_hj_wind_dir_inst.opened)
  {
    return 1;
  }
  uart_config.baud = 19200;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;

  g_hj_wind_dir_inst.modbus.modebus_type = eMODBUS_RS485;
  g_hj_wind_dir_inst.modbus.port_num = rs485_num_to_driver_num(ott->rs485_port);
  g_hj_wind_dir_inst.modbus.id = ott->modbus_id;

  modbus_init();
  drv_rs485_init(g_hj_wind_dir_inst.modbus.port_num,&uart_config,"Wind Direction");


  g_hj_wind_dir_inst.opened = true;
  


  return 1;
}


float hj_wind_direction_read(uint8_t *err)
{
  uint16_t reg[4];
  int16_t s_reg;
  eMODBUS_RESULT_t mb_ret;
  uint16_t status;
  uint16_t type;
  float wind_dir=0;

  osDelay(30);
  mb_ret = modbus_read_hold_reg(&g_hj_wind_dir_inst.modbus,  REG_R_WIND_DATA, reg, _countof(reg));

  if(mb_ret != eMODBUS_OK)
  {
            ERROR_PRINTF("hj_wind_direction_read error %s",get_modbus_err_string(mb_ret));
    *err = DRV_ERR_TIMEOUT;
    wind_dir = NAN;
  }
  else
  {
    *err = DRV_ERR_NONE;
    s_reg = (int16_t)reg[REG_R_WIND_DATA] ;
    type = reg[REG_R_TYPE] ;
    
    if(type == 2)//풍향인경우
    {
      wind_dir = (float)s_reg / 10.0f;
    }
    else if(type !=1)// 풍속도 아니면 에러 
    {
      wind_dir = -1;
    }
    }


  return wind_dir;
}


