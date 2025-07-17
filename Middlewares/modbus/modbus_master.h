
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <stdbool.h>
#include <stdint.h>

#include "modbus.h"

typedef struct modbus_config_s
{
  const char *name;
  int32_t port_num;
  eMODBUS_TYPE_t modebus_type;
  void *sem;
  int id;
}modbus_h_t;

void modbus_init(void);

int32_t modbus_write_single_reg(modbus_h_t *drv, uint8_t slave_id, uint16_t address, uint16_t val);
int32_t modbus_write_multi_reg(modbus_h_t *drv, uint8_t slave_id, uint16_t address, uint16_t *regs,
                               uint16_t regCnt);
int32_t modbus_read_hold_reg(modbus_h_t *drv, uint8_t slave_id, uint16_t address, uint16_t *pOutRegs,
                             uint16_t regCnt);
int32_t modbus_read_input_reg(modbus_h_t *drv, uint8_t slave_id, uint16_t address, uint16_t *pOutRegs,
                              uint16_t regCnt);
#endif