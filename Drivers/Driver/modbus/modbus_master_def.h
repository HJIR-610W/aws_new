
#ifndef MODBUS_MASTER_DEF_H
#define MODBUS_MASTER_DEF_H

#include <stdint.h>

#include "driver_interface.h"

typedef struct
{
  int32_t (*write_single_reg)(driver_t *drv, uint8_t slave_id, uint16_t address,
                              uint16_t val);
  int32_t (*write_multi_reg)(driver_t *drv, uint8_t slave_id, uint16_t address,
                             uint16_t *regs, uint16_t regCnt);
  int32_t (*read_hold_reg)(driver_t *drv, uint8_t slave_id, uint16_t address,
                            uint16_t *pOutRegs, uint16_t regCnt);
  int32_t (*read_input_reg)(driver_t *drv, uint8_t slave_id, uint16_t address, uint16_t *pOutRegs,
                           uint16_t regCnt);
} modbus_master_api_t;

#endif