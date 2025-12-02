
#include <stdint.h>
#include <stdio.h>

#include "aws_data.h"
#include "driver_interface.h"
#include "drv_rs232.h"
#include "old_aws_define.h"
#include "util_time.h"

void send_panel_aws_std(int32_t panel_port_num)
{

  uint8_t packet[64];
  uint8_t cnt = 0;

  kma_data_ex_t *p_kma = acquire_kma_data(eAWS_DATA_REAL);
  DATE_TIME_BUF *pDate = &Date_Time;

  packet[cnt++] = 0x02;  // STX

  // A: 날짜 YYMMDD
  sprintf((char *)&packet[cnt], "A%02d%02d%02d", pDate->Year % 100, pDate->Month, pDate->Day);
  cnt += 7;

  // B: 시간 HHMM
  sprintf((char *)&packet[cnt], "B%02d%02d", pDate->Hour, pDate->Min);
  cnt += 5;

  // C: 풍향 (degree)
  sprintf((char *)&packet[cnt], "C%03d", p_kma->wind_direction_avg.data / 10);
  cnt += 4;

  // D: 풍속 (0.1 m/s 단위)
  sprintf((char *)&packet[cnt], "D%03d", p_kma->wind_speed_avg.data);
  cnt += 4;

  // E: 기온 (nttt: 부호 + 3자리)
  if (p_kma->temperature.data >= 1000)
    sprintf((char *)&packet[cnt], "E0%03d", p_kma->temperature.data - 1000);
  else
    sprintf((char *)&packet[cnt], "E1%03d", 1000 - p_kma->temperature.data);
  cnt += 5;

  // F: 오늘 강수량
  sprintf((char *)&packet[cnt], "F%04d", g_rainfall.today );
  cnt += 5;

  // G: 어제 강수량
  sprintf((char *)&packet[cnt], "G%04d", g_rainfall.yesterday );
  cnt += 5;

  // H: 강수유무
  sprintf((char *)&packet[cnt], "H%d", p_kma->precipitation_presence.data ? 1 : 0);
  cnt += 2;

  // I: 기압
  sprintf((char *)&packet[cnt], "I%05d", p_kma->pressure.data);
  cnt += 6;

  // J: 습도
  sprintf((char *)&packet[cnt], "J%03d", p_kma->relative_humidity.data / 10);
  cnt += 4;

  // K: 적설
  sprintf((char *)&packet[cnt], "K%04d", p_kma->snowfall.data);
  cnt += 5;

  // M: 예비 (공백 4자리)
  sprintf((char *)&packet[cnt], "    ");
  cnt += 4;

  packet[cnt++] = 0x03;  // ETX

  // 전송
  drv_uart_send(panel_port_num, packet, cnt);
}
