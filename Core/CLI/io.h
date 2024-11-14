
#ifndef IO_HHH
#define IO_HHH

#include <stdint.h>

int32_t debug_printf(const char * pFmt, ...);
void debug_send(uint8_t *pData,uint16_t dataLen);
void set_debug_uart_handle(driver_t *drv);
#endif
