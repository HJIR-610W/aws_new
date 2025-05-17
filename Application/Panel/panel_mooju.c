

#include <stdint.h>
#include "old_aws_define.h"
#include "config_app.h"
#include "utile_time.h"
#include "aws_data.h"
#include "driver_uart.h"
#include "utile.h"
#include "panel_common.h"

void	send_panel_muju(driver_t *panel_port)
{
  char   framemk[60];
	uint8_t 	cnt = 0;
	uint32_t 	i;
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
framemk[cnt++]		= radd_dirc( p_kma->wind_direction_avg.data/ 10.0);
framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);
driver_uart_send(panel_port,framemk,cnt);

osDelay(500);																				// 500 ms

cnt					= 0;
framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
framemk[cnt++] 		= 0x02;																	// ID
framemk[cnt++] 		= 0x00;																	// Status
framemk[cnt++] 		= 0x00;																	// Start Address
framemk[cnt++] 		= 17;																	// Length

if(get_config_app()->panel_snow_use)
{
// SNOW FALL 추가(2012. 03. 26)
sprintf(&framemk[cnt],"%6.1f   ", (float)p_kma->snowfall.data / 10.0);
cnt					+= 9;
//SNOW FALL 추가(2012. 03. 26)  --끝--
}

if(get_config_app()->panel_barometer_use)
{

// 기압 FALL 추가(2013. 05. 21)
sprintf(&framemk[cnt],"%6.1f   ", (float)(p_kma->pressure.data) / 10);
cnt					+= 9;
// 기압 FALL 추가(2013. 05. 21)  --끝--

}



sprintf(&framemk[cnt],"%02d%02d%02d%02d", pDate->Month, pDate->Day,
                       pDate->Hour, pDate->Min);						// 월일시분 
cnt					+= 8;
framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);
driver_uart_send(panel_port,framemk,cnt);

osDelay(500);																				// 500 ms
                        
cnt					= 0;
framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
framemk[cnt++] 		= 0x03;																	// ID
framemk[cnt++] 		= 0x00;																	// Status
framemk[cnt++] 		= 0x00;																	// Start Address
framemk[cnt++] 		= 20;																	// Length

sprintf(&framemk[cnt],"%5.1f", ((float)p_kma->temperature.data - 1000.0)/10.0);			// 온도 현재
cnt					+= 5;
sprintf(&framemk[cnt],"%5.1f", ((float)p_kma->temperature.max - 1000.0)/10.0);				// 일 최고온도
cnt					+= 5;
sprintf(&framemk[cnt],"%5.1f", ((float)p_kma->temperature.min - 1000.0)/10.0);				// 일 최저온도
cnt					+= 5;
sprintf(&framemk[cnt],"%5.1f", (float)p_kma->relative_humidity.data/10.0);							// 습도 현재
cnt					+= 5;

framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);
driver_uart_send(panel_port,framemk,cnt);
osDelay(100);																				// 500 ms

cnt					= 0;
framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
framemk[cnt++] 		= 0x04;																	// ID
framemk[cnt++] 		= 0x00;																	// Status
framemk[cnt++] 		= 0x00;																	// Start Address
framemk[cnt++] 		= 17;																	// Length

sprintf(&framemk[cnt],"%5.1f", (float)p_kma->wind_speed_avg.data / 10.0);						// 풍속
cnt					+= 5;
sprintf(&framemk[cnt],"%4d", get_rainfall()->rainfall_yearly / 10);					// 연간 누계 강우량
cnt					+= 4;
sprintf(&framemk[cnt],"%4d", get_rainfall()->rainfall_today / 10);						// 금일  강우량
cnt					+= 4;
sprintf(&framemk[cnt],"%4d", get_rainfall()->rainfall_yesterday / 10);					// 전일  강우량
cnt					+= 4;

framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);
driver_uart_send(panel_port,framemk,cnt);
osDelay(100);																				// 500 ms

}