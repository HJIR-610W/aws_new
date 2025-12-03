

#include "hj_wind_speed_modbus.h"

#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "cmsis_os.h"
#include "app_sensor.h"
#include "config_sensor.h"
#include "modbus_master.h"
#include "dev_io.h"
#include "drv_rs485.h"





typedef struct hj_wind_speed_instance_s
{
  bool opened;
  modbus_h_t modbus;
} hj_wind_speed_instance_t;


hj_wind_speed_instance_t g_hj_wind_speed_inst;;


int32_t hj_wind_speed_init(void *opt)
{
  uart_config_t uart_config;
  wind_speed_hj_config_t *ott = (wind_speed_hj_config_t *)opt;

  if(  g_hj_wind_speed_inst.opened )
  {
    return 1;
  }
  uart_config.baud = 19200;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;

  g_hj_wind_speed_inst.modbus.modebus_type = eMODBUS_RS485;
  g_hj_wind_speed_inst.modbus.port_num = rs485_num_to_driver_num(ott->rs485_port);
  g_hj_wind_speed_inst.modbus.id = ott->modbus_id;
  modbus_init();
  
  drv_rs485_init(g_hj_wind_speed_inst.modbus.port_num,&uart_config,"Wind Speed");


  g_hj_wind_speed_inst.opened = true;
  


  return 1;
}


float hj_wind_speed_read(uint8_t *err)
{
  uint16_t reg[4];
  int16_t s_reg;
  int32_t ret;
  uint16_t status;
  uint16_t type;
  float wind_speed=0;

  osDelay(30);
  ret = modbus_read_hold_reg(&g_hj_wind_speed_inst.modbus,  REG_R_WIND_DATA, reg, _countof(reg));

  if(ret)
  {
    *err = DRV_ERR_TIMEOUT;
    wind_speed = NAN;
  }
  else
  {
    *err = DRV_ERR_NONE;
    s_reg = (int16_t)reg[REG_R_WIND_DATA] ;
    type = reg[REG_R_TYPE] ;
    
    if(type == 1)//풍속인경우
    {
      wind_speed = (float)s_reg / 10.0f;
    }
    else if(type !=2)// 풍향도 아니면 에러 
    {
      wind_speed = -1;
    }
    }


  return wind_speed;
}


void hj_wind_direction_write_holding_reg(uint16_t address,uint16_t val)
{


  if(!g_hj_wind_speed_inst.opened)
  {
    return;
  }

   modbus_write_holding_reg(&g_hj_wind_speed_inst.modbus,address,val);

}


void hj_wind_direction_read_holding_reg(uint16_t address,uint16_t count,uint16_t *p_out)
{


  if(!g_hj_wind_speed_inst.opened)
  {
    return;
  }

   modbus_read_hold_reg(&g_hj_wind_speed_inst.modbus,address,p_out,count);

}


modbus_h_t* hj_wind_speed_get_bus_io(void)
{
  
    return &g_hj_wind_speed_inst.modbus;
}