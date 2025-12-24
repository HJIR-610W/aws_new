








#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "drv_rs232.h"

#include "app_sensor.h"
#include "config_sensor.h"
#include "os_user_def.h"
#include "system_err.h"


typedef struct rmyoung_61402v_rs232_instance_s
{
    uint8_t rs232_port;
    void *sem;
    bool opened;
} rmyoung_61402v_rs232_instance_t;

rmyoung_61402v_rs232_instance_t rmyoung_61402v_rs232_inst;


#define ADC_BAROMETER_IDX 5
#define RS232_BAROMETER 3
#define PACKET_LEN 12 //<CR><LF>는 제외

int32_t rmyoung_61402v_rs232_init(void *opt)
{
    barometer_rmyoung_61402v_rs232_config_t *cfg = (barometer_rmyoung_61402v_rs232_config_t *)opt;
    uart_config_t uart_config;

    if (rmyoung_61402v_rs232_inst.opened)
    {
        return 1;
    }

    rmyoung_61402v_rs232_inst.opened = true;
    uart_config.baud = 9600;
    uart_config.dataLen = UART_DATA_LEN_8;
    uart_config.parity_index = PARITY_NONE;
    uart_config.stop_bit = UART_STOP_BIT_1;

    rmyoung_61402v_rs232_inst.rs232_port = uart_num_to_driver_num(cfg->rs232_port);
    drv_uart_init(rmyoung_61402v_rs232_inst.rs232_port, &uart_config,"Pressure");

    OS_CREATE_BINARY_SEM(rmyoung_61402v_rs232_inst.sem);

    return 1;
}

/**
 * @brief 진성 이엔지 SJGP-215 시리얼 기업계
 * 9600bps
M0!<CR>
>                 3E 20
0999.04<CR>

 */
float read_rmyoung_61402v_rs232_baromater(uint8_t *err)
{
    char buff[20];
    int len;
    float barometer = NAN;
    *err = 1;
    

    drv_uart_send(rmyoung_61402v_rs232_inst.rs232_port, "M0!\r", 4);

    len = drv_uart_recv_crlf(rmyoung_61402v_rs232_inst.rs232_port, buff, sizeof(buff),50);

    if (len > 0)
    {
        if(buff[0] == '>')
        {
        buff[len] = 0;
        barometer = strtof(&buff[2], NULL);
        *err = 0;
        }
    }
    else
    {
      ERROR_PRINTF("read_rmyoung_61402v_rs232_baromater error %d",len);
    }
    
    

    return barometer;
}

