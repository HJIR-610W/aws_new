
#ifndef IO_HHH
#define IO_HHH

#include <stdint.h>
#include "driver_interface.h"

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
  void *handle;
  void *config;
}dev_io_t;


void debug_uart_init(uint32_t baud_rate);
int32_t debug_printf(const char * pFmt, ...);
void debug_send(uint8_t *pData,uint16_t dataLen);
void debug_puts(char *str);
void debug_puts_nonos(char *str);
void set_debug_uart_handle(driver_t *drv);
uint16_t debug_recv(char *out,uint16_t outSize,uint32_t timeout);
void debug_putch(char ch);
driver_t * get_debug_uart_handle(void);
void LOG_MEM(uint8_t* src, uint32_t size, uint32_t startAddr,uint32_t col);



void dev_io_write(dev_io_t  *dev,uint8_t *data,uint32_t dataLen,uint32_t opt);
uint16_t dev_io_read(dev_io_t  *dev,uint8_t *out,uint32_t dataLen,uint8_t cmd,void *opt);
#endif
