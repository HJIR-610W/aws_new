

#include <stdint.h>
#include "cmsis_os2.h"
#include "old_aws_define.h"
#include "config_app.h"
#include "util_time.h"
#include "aws_data.h"
#include "drv_rs232.h"
#include "util_memory.h"
#include "panel_common.h"

/*
(1) 풍향
(2) 적설,기압,월,일,시,분
(3) 온도,일 최고 온도,일 최저 온도, 습도
(4) 풍속,연간우량,금일 우량,전일 우량

AWS(신)
(1) 51 01 05 02 01 0C 15
(2) 51 02 00 00 11 20 20 20 30 2E 30 20 20 20 20 35 35 30 2E 38 20 20 20 30 35 33 30 30 33 30 30 C1
(3) 51 03 00 00 14 20 32 36 2E 33 20 32 36 2E 33 2D 31 30 30 2E 20 34 34 2E 33 BE
(4) 51 04 00 00 11 39 39 39 2E 38 20 20 20 30 20 20 20 30 20 20 20 30 D6


AWS(구)
(1) 51 01 05 02 01 00 09
(2) 51 02 00 00 11 20 20 20 30 2E 30 20 20 20 31 39 39 39 2E 39 20 20 20 30 31 32 39 30 30 34 39 E4
(3) 51 03 00 00 14 38 39 39 2E 39 38 39 39 2E 39 2D 31 30 30 2E 39 39 39 2E 39 37
(4) 51 04 00 00 11 20 20 30 2E 30 2020 20 30 20 20 20 30 20 20 20 30 93
*/

void send_panel_muju(int32_t panel_port_num)
{
  char   framemk[60];
	uint8_t 	cnt = 0;

	DATE_TIME_BUF			*pDate;
  kma_data_ex_t *p_kma;

  pDate			= &Date_Time;

  p_kma =get_kma_data(eAWS_DATA_AVG);


// 무주 기상 상황판용 
framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
framemk[cnt++] 		= 0x01;																	// ID
framemk[cnt++] 		= 0x05;																	// Status
framemk[cnt++] 		= 0x02;																	// Start Address
framemk[cnt++] 		= 0x01;																	// Length
framemk[cnt++]		= radd_dirc( (uint32_t)(p_kma->wind_direction_avg.data/ 10.0));
framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);
drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);
// 51 01 05 02 01 01 0A 


osDelay(500);																				// 500 ms

cnt					= 0;
framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
framemk[cnt++] 		= 0x02;																	// ID
framemk[cnt++] 		= 0x00;																	// Status
framemk[cnt++] 		= 0x00;																	// Start Address
framemk[cnt++] 		= 17;																	// Length

if(get_config_app()->panel_snow_active)
{
// SNOW FALL 추가(2012. 03. 26)
sprintf(&framemk[cnt],"%6.1f   ", (float)p_kma->snowfall.data / 10.0);
cnt					+= 9;
//SNOW FALL 추가(2012. 03. 26)  --끝--
}

if(get_config_app()->panel_barometer_active)
{

// 기압 FALL 추가(2013. 05. 21)
sprintf(&framemk[cnt],"%6.1f   ", (float)p_kma->pressure.data / 10);
cnt					+= 9;
// 기압 FALL 추가(2013. 05. 21)  --끝--

}

sprintf(&framemk[cnt],"%02d%02d%02d%02d", pDate->Month, pDate->Day,
                       pDate->Hour, pDate->Min);						// 월일시분 
cnt					+= 8;
framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);
drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);

osDelay(500);																				// 500 ms
                        
cnt					= 0;
framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
framemk[cnt++] 		= 0x03;																	// ID
framemk[cnt++] 		= 0x00;																	// Status
framemk[cnt++] 		= 0x00;																	// Start Address
framemk[cnt++] 		= 20;																	// Length

sprintf(&framemk[cnt],"%5.1f", ((float)(p_kma->temperature.data - 1000.0))/10.0);			// 온도 현재
cnt					+= 5;
sprintf(&framemk[cnt],"%5.1f", ((float)(p_kma->temperature.max - 1000.0))/10.0);				// 일 최고온도
cnt					+= 5;
sprintf(&framemk[cnt],"%5.1f", ((float)(p_kma->temperature.min - 1000.0))/10.0);				// 일 최저온도
cnt					+= 5;
sprintf(&framemk[cnt],"%5.1f", (float)p_kma->relative_humidity.data/10.0);							// 습도 현재
cnt					+= 5;

framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);
drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);
osDelay(500);																				// 500 ms

cnt					= 0;
framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
framemk[cnt++] 		= 0x04;																	// ID
framemk[cnt++] 		= 0x00;																	// Status
framemk[cnt++] 		= 0x00;																	// Start Address
framemk[cnt++] 		= 17;																	// Length

sprintf(&framemk[cnt],"%5.1f", (float)p_kma->wind_speed_avg.data / 10.0);						// 풍속
cnt					+= 5;
sprintf(&framemk[cnt],"%4d", (uint16_t)(get_rainfall()->rainfall_yearly*10) );					// 연간 누계 강우량
cnt					+= 4;
sprintf(&framemk[cnt],"%4d", (uint16_t)(get_rainfall()->rainfall_today*10) );						// 금일  강우량
cnt					+= 4;
sprintf(&framemk[cnt],"%4d", (uint16_t)(get_rainfall()->rainfall_yesterday*10) );					// 전일  강우량
cnt					+= 4;

framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);
drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);
osDelay(500);																				// 500 ms

}