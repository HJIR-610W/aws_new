
#ifndef DRIVER_UART_H

#define DRIVER_UART_H

#define CONSOLE_UART 1

typedef struct driver_uart_s
{
    const char *name;
    uint8_t err;
    osSemaphoreId mutex;
    uint32_t num;

}driver_uart_t;


#endif