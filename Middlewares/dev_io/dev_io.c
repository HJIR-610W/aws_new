

#include "dev_io.h"
#include "drv_rs232.h"
#include "drv_rs485.h"
void dev_io_get(dev_io_t *dev, uint8_t cmd, void *opt)
{
  switch (cmd)
  {
    case DEV_IO_GET_CMD_CFG:
      switch (dev->io)
      {
        case eRS485_IO:

          break;
        case eRS232_IO:

          break;
      }
      break;
  }
}

void dev_io_write(dev_io_t *dev, uint8_t *data, uint32_t dataLen, uint32_t opt)
{
  switch (dev->io)
  {
    case eRS485_IO:
      drv_rs485_send(dev->num, data, dataLen);
      break;
    case eRS232_IO:
      drv_uart_send(dev->num, data, dataLen);
      break;
  }
}

void dev_io_flush(dev_io_t *dev)
{
  switch (dev->io)
  {
    case eRS485_IO:
      drv_rs485_flush_rx(dev->num);
      break;
    case eRS232_IO:
      drv_uart_flush_rx(dev->num);
      break;
  }
}

uint16_t dev_io_read(dev_io_t *dev, uint8_t *out, uint32_t dataLen, uint8_t cmd, void *opt)
{
  devIoTimeOutopt_t *pdevopt = opt;
  uint32_t data_timeout;
  data_timeout = pdevopt->waitTimeOutMs / 2;

  switch (dev->io)
  {
    case eRS485_IO:
       return drv_rs485_recv_opt(dev->num, out, dataLen, pdevopt->waitTimeOutMs,data_timeout);
      break;
    case eRS232_IO:

      return drv_uart_recv_opt(dev->num, out, dataLen, pdevopt->waitTimeOutMs, data_timeout);
      break;
  }
  return 0;
}