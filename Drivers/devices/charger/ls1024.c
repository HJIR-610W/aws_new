
#include "ls1024.h"

#include <string.h>
#include <math.h>

#include "cmsis_os2.h"
#include "util_memory.h"
#include "util_time.h"
#include "os_user_def.h"
#include "driver_modbus.h"

typedef struct ls1024_cfg_s
{
  modbus_init_t modbus;
  void *sem;
  bool opened;
  driver_t *bus_io;
} ls1024_instance_t;

ls1024_instance_t ls1024_inst;

int32_t ls1024_init(void)
{
  if (ls1024_inst.opened)
  {
    return 1;
  }

  ls1024_inst.modbus.baud = 115200;
  ls1024_inst.modbus.parityIdx = 0;
  ls1024_inst.modbus.stop = 1;
  ls1024_inst.modbus.port_num = RS485_B;
  ls1024_inst.bus_io = driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_485, &ls1024_inst.modbus);

  OS_CREATE_BINARY_SEM(ls1024_inst.sem);

  ls1024_inst.opened = true;

  return 1;
}

void ls1024_read( charger_data_t *charger_data, uint8_t *err)
{
  uint16_t reg[15];

  int32_t ret;

  ret = driver_modbus_m_read_input_reg(ls1024_inst.bus_io, 1, 0x3100, reg, 15);

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