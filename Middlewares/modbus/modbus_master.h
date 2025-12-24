
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <stdbool.h>
#include <stdint.h>

#include "modbus.h"
#include "io_interface.h"

typedef struct modbus_config_s
{
  const char *name;
  int32_t port_num;
  eMODBUS_TYPE_t modebus_type;
  void *sem;
  int id;
  io_if_t *io;
}modbus_h_t;

void modbus_init(void);

eMODBUS_RESULT_t modbus_write_single_coil(modbus_h_t *drv, uint16_t address, bool val);
eMODBUS_RESULT_t modbus_write_holding_reg(modbus_h_t *drv, uint16_t address, uint16_t val);

eMODBUS_RESULT_t modbus_read_single_coil(modbus_h_t *drv, uint16_t address, uint16_t *pOutCoil);
eMODBUS_RESULT_t modbus_read_discrete_inputs(modbus_h_t *drv, uint16_t address, uint16_t *pOutInputs, uint16_t inputCnt);
eMODBUS_RESULT_t modbus_write_multi_reg(modbus_h_t *drv, uint16_t address, uint16_t *regs,
                               uint16_t regCnt);
eMODBUS_RESULT_t modbus_read_hold_reg(modbus_h_t *drv, uint16_t address, uint16_t *pOutRegs,
                             uint16_t regCnt);
eMODBUS_RESULT_t modbus_read_input_reg(modbus_h_t *drv, uint16_t address, uint16_t *pOutRegs,
                              uint16_t regCnt);
const char *get_modbus_err_string(eMODBUS_RESULT_t err);
#endif