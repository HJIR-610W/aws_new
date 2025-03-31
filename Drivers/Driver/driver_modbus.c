

#include "driver_modbus.h"

#include "modbus/modbus_master.h"
#include "modbus/modbus_master_def.h"

driver_t *driver_modbus_master_open(int32_t num, void *opt)
{
  driver_t *driver = 0;
  switch (num)
  {
    case DRIVER_MODBUS_MSTER_RTU_OVER_485:
      driver = modbus_master_open(MODBUS_RTU_OVER_485, opt);
      break;
  }

  return driver;
}

int32_t driver_modbus_m_write_single_reg(driver_t *drv, uint8_t slave_id, uint16_t address,
                                         uint16_t val)
{
  const modbus_master_api_t *api = drv->api;

  return api->write_single_reg(drv, slave_id, address, val);
}

int32_t driver_modbus_m_write_multi_reg(driver_t *drv, uint8_t slave_id, uint16_t address,
                                        uint16_t *regs, uint16_t regCnt)
{
  const modbus_master_api_t *api = drv->api;
  return api->write_multi_reg(drv, slave_id, address, regs, regCnt);
}

int32_t driver_modbus_m_read_multi_reg(driver_t *drv, uint8_t slave_id, uint16_t address,
                                       uint16_t *pOutRegs, uint16_t regCnt)
{
  const modbus_master_api_t *api = drv->api;

  return api->read_multi_reg(drv, slave_id, address, pOutRegs, regCnt);
}