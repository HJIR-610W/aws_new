
#ifndef IO_HHH
#define IO_HHH

#include <stdint.h>
#include <stdarg.h>

#include "driver_interface.h"

#define IO_COLOR_RED     31
#define IO_COLOR_GREEN   32
#define IO_COLOR_YELLOW  33
#define IO_COLOR_BLUE    34
#define IO_COLOR_MAGENTA 35
#define IO_COLOR_CYAN    36
#define IO_COLOR_WHITE   37

#define ASCII_CODE_ESC 0x1B
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
  int32_t num;
  void *config;
}dev_io_t;

void dev_io_write(dev_io_t *dev, uint8_t *data, uint32_t dataLen, uint32_t opt);
uint16_t dev_io_read(dev_io_t *dev, uint8_t *out, uint32_t dataLen, uint8_t cmd, void *opt);
void dev_io_flush(dev_io_t *dev);

int32_t io_vprintf(const char *pFmt, va_list ap);
int32_t io_printf(const char * pFmt, ...);
void io_printf_color(int color, const char *pFmt, ...);
int32_t io_recv(char *out, uint16_t outSize, uint32_t timeout);
void io_put_ch(char ch);
void io_send(uint8_t *pData,uint16_t dataLen);
void io_puts(const char *str);

int32_t io_inject(uint8_t *p_data,uint32_t data_len);
void debug_uart_init(uint32_t baud_rate);



void debug_puts_nonos(char *str);
void set_debug_uart_handle(int32_t drv);

int32_t get_debug_uart_handle(void);
void LOG_MEM(uint8_t* src, uint32_t size, uint32_t startAddr,uint32_t col);


void set_task_id(void *task_id);
void task_printf( const char *pFmt, ...);
void task_hex_dump(const char *title, const uint8_t *data, uint32_t length);
void set_forced_print(bool set);
int32_t io_vprintf(const char *pFmt, va_list ap);
int io_scanf_s(const char *fmt, ...);


#endif
