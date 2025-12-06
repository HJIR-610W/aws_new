
#ifndef DEV_IO_H
#define DEV_IO_H

#include <stdint.h>
#include <stdarg.h>

#include "driver_interface.h"
#include "io_interface.h"


#define DEV_IO_CMD_RECV_TIMEOUT  1 
#define DEV_IO_CMD_DATA_TIMEOUT  2

#define DEV_IO_GET_CMD_CFG 0x01
    typedef struct
{
  uint32_t waitTimeOutMs;
  uint32_t dataTimeOutMs;
}devIoTimeOutopt_t;


typedef enum dev_io_e
{
  eRS485_IO,
  eRS232_IO,
  eETHERNET_IO,
  eFILE_FLASH,
  eFILE_SD,
  eSMS_IO
}eDEV_IO_t;

typedef struct dev_io_s
{
  eDEV_IO_t io;
  int32_t num;
  void *config;
}dev_io_t;

void dev_io_write(dev_io_t *dev, uint8_t *data, uint32_t dataLen, uint32_t opt);
uint16_t dev_io_read(dev_io_t *dev, uint8_t *out, uint32_t dataLen, uint8_t cmd, void *opt);
void dev_io_flush(dev_io_t *dev);
#endif