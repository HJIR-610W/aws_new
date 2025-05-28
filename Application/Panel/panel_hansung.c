

#include <stdint.h>
#include "old_aws_define.h"
#include "config_app.h"
#include "utile_time.h"
#include "aws_data.h"
#include "driver_uart.h"
#include "utile.h"
#include "panel_common.h"

/*

51 01 05 02 01 0C 15     //풍향
51 02 00 00 14 20 39 39 39 2E 20 32 36 2E 37 20 20 30 2E 30 20 20 30 2E 30 //풍속,온도,금일우량,전일우량

*/
// ========================================================================================================== //
//                          한성 전자 Protocol과 공용
// ========================================================================================================== //
void	send_panel_hansung(driver_t *panel_port)
{
    char   framemk[60];
	uint8_t 	cnt = 0;
	uint32_t 	i;
	DATE_TIME_BUF			*pDate;
  kma_data_ex_t *p_kma;

  pDate			= &Date_Time;

  p_kma =get_kma_data(eAWS_DATA_AVG);

	framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
	framemk[cnt++] 		= 0x01;																	// ID
	framemk[cnt++] 		= 0x05;																	// Status
	framemk[cnt++] 		= 0x02;																	// Start Address
	framemk[cnt++] 		= 0x01;																	// Length
	framemk[cnt++]		= radd_dirc( p_kma->wind_direction_avg.data / 10.0);
	framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);

  driver_uart_send(panel_port,framemk,cnt);

	osDelay(500);																				// 500 ms
	
	cnt					= 0;
	framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
	framemk[cnt++] 		= 0x02;																	// ID
	framemk[cnt++] 		= 0x00;																	// Status
	framemk[cnt++] 		= 0x00;																	// Start Address
	framemk[cnt++] 		= 20;																	// Length
	
	sprintf(&framemk[cnt]," %4.1f", (float)p_kma->wind_speed_avg.data / 10.0); //실측값 전송
	cnt					+= 5;
	sprintf(&framemk[cnt],"%5.1f", ((float)p_kma->temperature.data - 1000.0)/10.0); //실측값 전송
	cnt					+= 5;
	sprintf(&framemk[cnt],"%5.1f%5.1f", (float)get_rainfall()->rainfall_today, (float)get_rainfall()->rainfall_yesterday);
	cnt					+= 10;
	framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);

	driver_uart_send(panel_port,framemk,cnt);

}
