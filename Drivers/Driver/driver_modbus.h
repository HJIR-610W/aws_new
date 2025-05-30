
#ifndef DRIVER_MODBUS_MASTER_H
#define DRIVER_MODBUS_MASTER_H

#include "driver_485.h"
#include "driver_485_def.h"
#include "driver_interface.h"

// 사용가능한 모드버스 종류
#define DRIVER_MODBUS_MSTER_RTU_OVER_485 100
#define DRIVER_MODBUS_MSTER_RTU_OVER_232 101
/*
모드버스 초기화를 위해 제공하는 정보, 각각의 드라이버에서 확인하여 수동으로 기입
RS485는 2개 사용 가능능
*/
#define MODBUS_MSTER_RTU_OVER_485_PORTA RS485_A  // 사용 가능한 모드버스 포트
#define MODBUS_MSTER_RTU_OVER_485_PORTB RS485_B  // 사용 가능한 모드버스 포트

#ifndef MODBUS_INIT_T
#define MODBUS_INIT_T
#include "driver_485_def.h"
typedef rs485_init_t modbus_init_t;
#endif

driver_t *driver_modbus_master_open(int32_t num, void *opt);

int32_t driver_modbus_m_write_single_reg(driver_t *drv, uint8_t slave_id, uint16_t address,
                                         uint16_t val);

int32_t driver_modbus_m_write_multi_reg(driver_t *drv, uint8_t slave_id, uint16_t address,
                                        uint16_t *regs, uint16_t regCnt);

int32_t driver_modbus_m_read_hold_reg(driver_t *drv, uint8_t slave_id, uint16_t address,
                                       uint16_t *pOutRegs, uint16_t regCnt);

int32_t driver_modbus_m_read_input_reg(driver_t *drv, uint8_t slave_id, uint16_t address,
                                       uint16_t *pOutRegs, uint16_t regCnt);
#endif