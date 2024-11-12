
#ifndef PCF8575_H
#define PCF8575_H


#include "driver_interface.h"

#define PCF88575_CMD_DIR_SET 0
typedef struct pcf8575_cfg_s
{
  void *i2c_io;
  void *irq_io;
  uint16_t address;
  uint32_t port_data;
  uint16_t dir;//읽기 1, 쓰기 0
}pcf8575_cfg_t;


driver_t *pcf8575_open(uint32_t num);
int pcf8575_write(driver_t *drv,uint16_t port_data);
int pcf8575_read(driver_t *drv,uint16_t *port_data);
int pcf8575_set(driver_t *drv,uint8_t cmd,void *option);
//하드코딩 함,0..7 입력, 8..15출력 추후 수정
int pcf8575_write8(driver_t *drv,uint16_t port_data);
int pcf8575_read8(driver_t *drv,uint16_t *port_data);

uint16_t pcf8575_read_pin(driver_t *drv,uint16_t pin);
int pcf8575_write_pin(driver_t *drv,uint16_t pin,uint16_t high);

#endif
