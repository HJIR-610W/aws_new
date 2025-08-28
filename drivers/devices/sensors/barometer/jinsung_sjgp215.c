








#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "drv_rs232.h"

#include "app_sensor.h"
#include "config_sensor.h"
#include "os_user_def.h"


typedef struct sjgp215_instance_s
{
    uint8_t rs232_port;
    void *sem;
    bool opened;
} sjgp215_instance_t;

sjgp215_instance_t sjgp215_inst;


#define ADC_BAROMETER_IDX 5
#define RS232_BAROMETER 3
#define PACKET_LEN 12 //<CR><LF>는 제외

int32_t sjgp215_init(void *opt)
{
    jinsung_sjgp215_config_t *cfg = (jinsung_sjgp215_config_t *)opt;
    uart_config_t uart_config;

    if (sjgp215_inst.opened)
    {
        return 1;
    }

    sjgp215_inst.opened = true;
    uart_config.baud = 9600;
    uart_config.dataLen = UART_DATA_LEN_8;
    uart_config.parity_index = PARITY_NONE;
    uart_config.stop_bit = UART_STOP_BIT_1;

    sjgp215_inst.rs232_port = uart_num_to_driver_num(cfg->rs232_port);
    drv_uart_init(sjgp215_inst.rs232_port, &uart_config);

    OS_CREATE_BINARY_SEM(sjgp215_inst.sem);

    return 1;
}

/**
 * @brief 진성 이엔지 SJGP-215 시리얼 기업계
 * 9600bps
 * 출력:1002.29 hPa <CR><LF>
 * 출력: 087.41 hPa <CR><LF>
 *
 * 31 30 30 32 2E 32 39 20 68 50 61 20 0D 0A
 */
float read_sjgp215_baromater(uint8_t *err)
{
    char buff[20];
    int len;
    int failed = 0;
    float barometer = NAN;
    

    len = drv_uart_recv_crlf(sjgp215_inst.rs232_port, buff, sizeof(buff),50);

    if (len == PACKET_LEN)
    {
        // 체크섬이 없으므로
        for (int i = 0; i < 8; i++)
        {
            if (buff[i] < '0' || buff[i] > '9')
            {
                if (buff[i] != ' ' && buff[i] != '.')
                {
                    *err = 1;
                    failed =1;
                     break;
                }
            }
        }

        if(failed==0)
        {
            barometer = atof(buff);
            *err = 0;
        }
    
    }

    return barometer;
}

