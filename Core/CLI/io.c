
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "driver_uart.h"



static driver_t *debug_uart=NULL;;


void set_debug_uart_handle(driver_t *drv)
{
  debug_uart = drv;
}

int32_t debug_printf(const char * pFmt, ...)
{
    uint8_t buff[200];
    va_list ap;  
    int32_t len;
   
    va_start(ap, pFmt);
    len =vsnprintf((char *)buff, sizeof(buff), (char *)pFmt, ap);
    va_end(ap);
    


    if(debug_uart == NULL)
    {
      debug_uart = driver_uart_open(UART_STM32_1);
    }
    else
    {
      driver_uart_send(debug_uart,buff,len);
    }

    return 0;

}

void debug_send(uint8_t *pData,uint16_t dataLen)
{
                 
    driver_uart_send(debug_uart,pData,dataLen);
}
