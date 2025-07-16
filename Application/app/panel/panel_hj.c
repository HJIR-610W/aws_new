
#include <stdio.h>
#include "util_time.h"
#include "old_aws_define.h"

#include "aws_data.h"
#include "driver_interface.h"
#include "drv_rs232.h"

/*
02 41 32 34 31 32 31 31 42 30 30 31 35 43 32 37 39 44 39 39 39 45 30 32 36 37 46 30 30 30 30 47 30
30 30 30 48 30 49 30 35 35 30 39 4A 30 34 33 4B 39 39 39 39 4C 31 31 30 30 4D 30 32 36 37 20 20 20
20 03
<STX>A241211B0015C279D999E0267F0000G0000H0I05509J043K9999L1100M0267    <ETX>
*/
uint16_t send_panel_hj(int32_t panel_port_num)
{
    char framemk[50];
    uint8_t 	cnt = 0;

    DATE_TIME_BUF			*pDate;
    kma_data_ex_t *p_kma;

    p_kma = get_kma_data(eAWS_DATA_AVG);

	pDate			= &Date_Time;

	framemk[cnt++] 		= 0x02;                                                                  // STX
    sprintf(&framemk[cnt],"A%02d%02d%02d", pDate->Year % 100, pDate->Month, pDate->Day);        // A YYMMDD
    cnt     += 7;
    sprintf(&framemk[cnt],"B%02d%02d", pDate->Hour, pDate->Min);                                // B HHMM
    cnt     += 5;
    sprintf(&framemk[cnt],"C%03d", p_kma->wind_direction_avg.data / 10);                          // C 풍향 000 - 360
    cnt     += 4;
    sprintf(&framemk[cnt],"D%03d", p_kma->wind_speed_avg.data);                                   // D 풍속 000 -750   (관측값 * 10)
    cnt     += 4;

    if(p_kma->temperature.data >= 1000)
        sprintf(&framemk[cnt],"E0%03d", p_kma->temperature.data - 1000);                       // E 온도 -500 - 500 (관측값 * 10)
    else
        sprintf(&framemk[cnt],"E1%03d", 1000 - p_kma->temperature.data);                       // E 온도 -500 - 500 (관측값 * 10)

    cnt     += 5;

    sprintf(&framemk[cnt],"F%04d", (uint16_t)(get_rainfall()->rainfall_today*10));                                    // F 오늘 강수량 0000 - 9999 mm (관측값 * 10)
    cnt     += 5;
    sprintf(&framemk[cnt],"G%04d", (uint16_t)(get_rainfall()->rainfall_yesterday*10));                                 // G 어제 강수량 0000 - 9999 mm (관측값 * 10)
    cnt     += 5;

    sprintf(&framemk[cnt],"H%01d", p_kma->precipitation_presence.data); //AWS(구)동일처리 유:10 무:0                    // H 강수 유무 1: 유 0: 무
    cnt     += 2;
    sprintf(&framemk[cnt],"I%05d", p_kma->pressure.data);                                    // I 기압 05000 - 11000 hPa(관측값 * 10)
    cnt     += 6;
    sprintf(&framemk[cnt],"J%03d", p_kma->relative_humidity.data / 10);                                 // J 습도 000 - 100 %
    cnt     += 4;
    sprintf(&framemk[cnt],"K%04d", p_kma->snowfall.data);                                      // K 적설 0000 - 9999 cm (관측값 * 10)
    cnt     += 5;

    if(p_kma->temperature.min >= 1000)
        sprintf(&framemk[cnt],"L0%03d", p_kma->temperature.min - 1000);                       // E 온도 -500 - 500 (관측값 * 10)
    else
        sprintf(&framemk[cnt],"L1%03d", 1000 - p_kma->temperature.min);                       // E 온도 -500 - 500 (관측값 * 10)
    cnt     += 5;

    if(p_kma->temperature.max >= 1000)
        sprintf(&framemk[cnt],"M0%03d", p_kma->temperature.max  - 1000);                       // E 온도 -500 - 500 (관측값 * 10)
    else
        sprintf(&framemk[cnt],"M1%03d", 1000 - p_kma->temperature.max );                       // E 온도 -500 - 500 (관측값 * 10)
    cnt     += 5;

    sprintf(&framemk[cnt],"    ");                                                              // 예비 
    cnt     += 4;

    framemk[cnt++]  = 0x03;                                                                     // ETX

    drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);
    return cnt;
		
}