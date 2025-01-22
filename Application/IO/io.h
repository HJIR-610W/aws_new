
#ifndef IO_HHH
#define IO_HHH

#include <stdint.h>
#include "driver_interface.h"

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

#endif
