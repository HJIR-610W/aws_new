

#include <stdint.h>
#include "old_aws_define.h"
#include "config_app.h"
#include "util_time.h"
#include "aws_data.h"
#include "drv_rs232.h"
#include "util_memory.h"
#include "panel_common.h"
#include "cmsis_os2.h"

/*

51 01 05 02 01 0C 15     //풍향
51 02 00 00 14 20 20 30 2E 30 20 32 36 2E 33 20 20 30 2E 30 20 20 30 2E 30 69
//풍속,온도,금일우량,전일우량
AWS(구)
51 02 00 00 14 20 20 30 2E 30 38 39 39 2E 39 20 20 30 2E 30 20 20 30 2E 30 91

*/
// ========================================================================================================== //
//                          한성 전자 Protocol과 공용
// ========================================================================================================== //
void send_panel_hansung(int32_t panel_port_num)
{
    char   framemk[60];
	uint8_t 	cnt = 0;


  kma_data_ex_t *p_kma;


  p_kma =acquire_kma_data(eAWS_DATA_REAL);

	framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
	framemk[cnt++] 		= 0x01;																	// ID
	framemk[cnt++] 		= 0x05;																	// Status
	framemk[cnt++] 		= 0x02;																	// Start Address
	framemk[cnt++] 		= 0x01;																	// Length
	framemk[cnt++]		= radd_dirc((uint32_t)(p_kma->wind_direction_avg.data / 10.0));
	framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);

	drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);

	osDelay(500);																				// 500 ms
	
	cnt					= 0;
	framemk[cnt++] 		= 'Q';																	//Start Code 'Q'
	framemk[cnt++] 		= 0x02;																	// ID
	framemk[cnt++] 		= 0x00;																	// Status
	framemk[cnt++] 		= 0x00;																	// Start Address
	framemk[cnt++] 		= 20;																	// Length
	
	sprintf(&framemk[cnt]," %4.1f", (float)p_kma->wind_speed_avg.data / 10.0); //실측값 전송
	cnt					+= 5;
	sprintf(&framemk[cnt],"%5.1f", ((float)(p_kma->temperature.data - 1000.0))/10.0); //실측값 전송
	cnt					+= 5;
	sprintf(&framemk[cnt],"%5.1f%5.1f", (float)(g_rainfall.today/10.0f), (float)(g_rainfall.yesterday/10.0f));
	cnt					+= 10;
	framemk[cnt++]		= (char)make_sum((uint8_t*)&framemk[1], framemk[4]+4);

	drv_uart_send(panel_port_num, (uint8_t *)framemk, cnt);
}
