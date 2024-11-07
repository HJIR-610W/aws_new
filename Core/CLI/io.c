
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"


extern UART_HandleTypeDef huart1;

int32_t debug_printf(const char * pFmt, ...)
{
    uint8_t buff[200];
    va_list ap;  
    int32_t len;
   


    va_start(ap, pFmt);
    len =vsnprintf((char *)buff, sizeof(buff), (char *)pFmt, ap);
    va_end(ap);
    


   HAL_UART_Transmit(&huart1, buff, strlen(buff), HAL_MAX_DELAY);
            
            


    return 0;
}
