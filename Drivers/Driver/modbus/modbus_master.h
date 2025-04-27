
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <stdbool.h>
#include <stdint.h>

#include "driver_interface.h"
#include "modbus.h"

#define MODBUS_RTU_OVER_485 0
#define MODBUS_RTU_OVER_232 1

#define MODBUS_RTU_MAX 2

driver_t *modbus_master_open(int32_t num, void *opt);

int32_t modbus_write_single_reg(driver_t *drv, uint8_t slave_id, uint16_t address, uint16_t val);
int32_t modbus_write_multi_reg(driver_t *drv, uint8_t slave_id, uint16_t address, uint16_t *regs,
                               uint16_t regCnt);
int32_t modbus_read_multi_reg(driver_t *drv, uint8_t slave_id, uint16_t address, uint16_t *pOutRegs,
                              uint16_t regCnt);

#endif