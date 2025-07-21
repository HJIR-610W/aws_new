
#include "ls1024.h"

#include <string.h>
#include <math.h>

#include "cmsis_os2.h"
#include "util_memory.h"
#include "util_time.h"
#include "os_user_def.h"

#include "modbus_master.h"
#include "drv_rs485.h"
typedef struct ls1024_cfg_s
{
  modbus_h_t modbus;
  void *sem;
  bool opened;

} ls1024_instance_t;

ls1024_instance_t ls1024_inst;

int32_t ls1024_init(void)
{
  uart_config_t uart_config;

  if (ls1024_inst.opened)
  {
    return 1;
  }

  uart_config.baud = 115200;
  uart_config.dataLen = UART_DATA_LEN_8;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;

  ls1024_inst.modbus.name="ls1024";
  ls1024_inst.modbus.modebus_type = eMODBUS_RS485;
  ls1024_inst.modbus.port_num = RS485_B  ;
  ls1024_inst.modbus.id = 1;
      drv_rs485_init(ls1024_inst.modbus.port_num, &uart_config);

  OS_CREATE_BINARY_SEM(ls1024_inst.sem);

  ls1024_inst.opened = true;

  return 1;
}


// 요청 01 04 31 00 00 0F BE F2  
// 응답    01 04 1E 00 67 00 00 00 00 00 00 04 FE 00 00 00 00 00 00 04 FE 00 00 00 00 00 00 04 FE 00 00 00 00 AF BA 

#define READ_REG_CNT 15
void ls1024_read( charger_data_t *charger_data, uint8_t *err)
{
  uint16_t reg[READ_REG_CNT];

  int32_t ret;

  ret = modbus_read_input_reg(&ls1024_inst.modbus,  0x3100, reg, READ_REG_CNT);

  if (ret)
  {
    *err = DRV_ERR_TIMEOUT;
  }
  else
  {
    *err = DRV_ERR_NONE;
    // Solar (PV1)
    charger_data->solar1Volt = (float)reg[0] / 100.0f;    // 0x3100
    charger_data->solar1Current = (float)reg[1] / 100.0f; // 0x3101

    // Battery1
    charger_data->battery1 = (float)reg[4] / 100.0f; // 0x3104

    // Load1
    charger_data->load1Current = (float)reg[13] / 100.0f; // 0x310D

    // Unused (추가 솔라 및 배터리는 현재 미사용)
    charger_data->solar2Volt = 0.0f;
    charger_data->solar2Current = 0.0f;
    charger_data->battery2 = 0.0f;
    charger_data->load2Current = 0.0f;
    charger_data->load3Current = 0.0f;
  }
}