

#include "ls1024.h"

#include "task_modbus.h"

int32_t read_ls1024(power_t *power)
{
  uint16_t reg[15];  // F/WVersion_H ~Alarm Bit
  eRET_t ret;

  ret = modbus_read_hold_reg(0x01, 0x3100, reg, 15);

  if (ret == RET_OK)
  {
    power->solarVoltage = reg[0];
    power->solarCurrent = reg[1];
    power->batteryVoltage1 = reg[4];
    power->batteryVoltage2 = 0;
    power->load1Current = reg[13];
    power->load2Current = 0;
    power->load3Current = 0;
    power->loadCurrentN = reg[13];
  }

  return 0;
}