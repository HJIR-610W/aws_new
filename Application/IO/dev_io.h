
#ifndef IO_HHH
#define IO_HHH

#include <stdint.h>
#include "driver_interface.h"

#define ASCII_CODE_ESC    0x1B
#define ASCII_CODE_CTRL_Q 0x11
#define ASCII_CODE_CTRL_C 0x03
#define ASCII_CODE_CR     0x0D

#define ASCII_SPEICIAL    0x5B //   '['   

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
  void *driver;
  void *config;
}dev_io_t;

void debug_uart_init(uint32_t baud_rate);
int32_t io_printf(const char * pFmt, ...);
int32_t io_recv(char *out, uint16_t outSize, uint32_t timeout);
void io_put_ch(char ch);
void io_send(uint8_t *pData,uint16_t dataLen);
void io_puts(const char *str);

void debug_puts_nonos(char *str);
void set_debug_uart_handle(driver_t *drv);

driver_t * get_debug_uart_handle(void);
void LOG_MEM(uint8_t* src, uint32_t size, uint32_t startAddr,uint32_t col);


void dev_io_write(dev_io_t  *dev,uint8_t *data,uint32_t dataLen,uint32_t opt);
uint16_t dev_io_read(dev_io_t  *dev,uint8_t *out,uint32_t dataLen,uint8_t cmd,void *opt);

void set_task_id(void *task_id);
void task_printf( const char *pFmt, ...);
void task_hex_dump(const char *title, const uint8_t *data, uint32_t length);
void set_forced_print(bool set);
#endif
