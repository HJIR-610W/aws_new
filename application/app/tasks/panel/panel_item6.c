


#include "panel_item6.h"

#include <stdint.h>

#include "cmsis_os.h"
#include "util_time.h"
#include "old_aws_define.h"

#include "aws_data.h"
#include "driver_interface.h"
#include "drv_rs232.h"
#include "config_app.h"


/**
 * 금일 시간 우량
 * 전일 우량
 * 금일 우량
 * 누계 우량
 * 현재 적설
 * 금일 적설
 */
void send_panel_item6(int32_t panel_port_num)
{
    AWS_DATA_STRUCT *pAws;
    kma_data_ex_t *p_kma;

    uint8_t framemk[100];
    uint8_t cnt = 0;
    uint8_t i;
    


    cnt = 0;
    framemk[cnt++] = 0x02; // STX
    framemk[cnt++] = 0x01; // 장비 ID
    framemk[cnt++] = 0x00; // 컬러
    framemk[cnt++] = 0x00; // 데이터의 시작 위치
    cnt++;
    if (config.panel_item6_type == ePANEL_ITEM6_MONTH)
    {
        sprintf((char *)&framemk[cnt], "%02d%02d", Date_Time.Month, 1);
    }
    else
    {
        sprintf((char *)&framemk[cnt], "%02d%02d", 1,1);
    }
    cnt += 4;
    sprintf((char *)&framemk[cnt], "%02d%02d", 0, 0);
    cnt += 4;

    sprintf((char *)&framemk[cnt], "%02d%02d%02d%02d", Date_Time.Month, Date_Time.Day,
            Date_Time.Hour, Date_Time.Min);
    cnt += 8;

    sprintf((char *)&framemk[cnt], "%6.1f", g_rainfall.hourly/10.0f); // 금일 시간 우량
    cnt += 6;
    sprintf((char *)&framemk[cnt], "%6.1f", g_rainfall.yesterday / 10.0f); // 전일 우량
    cnt += 6;

    sprintf((char *)&framemk[cnt], "%6.1f", g_rainfall.today/ 10.0f); // 금일 우량
    cnt += 6;
    if (config.panel_item6_type == ePANEL_ITEM6_TOTAL)
        sprintf((char *)&framemk[cnt], "%6.1f", g_rainfall.yearly / 10.0f); // 누계 우량
    else
        sprintf((char *)&framemk[cnt], "%6.1f", g_rainfall.monthly/ 10.0f);

    cnt += 6;

    framemk[4] = cnt - 5; // 데이터의 사이즈
    framemk[cnt] = make_sum(&framemk[1], cnt - 1);
    cnt++;
    framemk[cnt++] = 0x03;

    drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);
    
    osDelay(100);
    cnt = 0;
    framemk[cnt++] = 0x02; // STX
    framemk[cnt++] = 0x02; // 장비 ID
    framemk[cnt++] = 0x00; // 컬러
    framemk[cnt++] = 0x10; // 데이터의 시작 위치
    cnt++;
    sprintf((char *)&framemk[cnt], "%6.1f", (float)g_snowfall.current / 10.0f); // 현재 적설
    cnt += 6;
    sprintf((char *)&framemk[cnt], "%6.1f", (float)g_snowfall.daily/10.0f); // 금일 적설
    cnt += 6;

    framemk[4] = cnt - 5; // 데이터의 사이즈
    framemk[cnt] = make_sum(&framemk[1], cnt - 1);
    cnt++;
    framemk[cnt++] = 0x03;

    drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);

    osDelay(100);
}
