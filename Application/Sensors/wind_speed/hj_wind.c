
#include <string.h>
#include <math.h>
#include "cmsis_os2.h"

#include "app_rs485.h"
#include "app_sensor.h"
#include "driver_485.h"
#include "driver_uart.h"
#include "hj_wind.h"
#include "utile.h"
#include "dev_io.h"

/* AWS AVR 에서 가져옴
  화진티엔아이 AWS 풍향 풍속계  PROTOCOL 정의
	.Data Table.
	Start Code     	0  	: 0x02 			-> STX
	Unit ID		   	1	: 0x01 - 0x0f 	-> 장비 ID
	Command			2	: 0xXX          -> 01:파라메타 설정, 02:Data Read, 03: Write & Read 
	Data Size       4	: 0x02			-> 데이터의 사이즈
	Data            5	: n				-> 전송되는 데이터 ASCII 형식
	Check Sum		6	: 1      		-> ID - Data n 까지의 합
	End Code 		7	: 0x03			-> ETX
	
*/

//02 01 02 01 01 05 03   풍속
//02 02 02 01 01 06 03   풍향
uint16_t	WindSendFrameMake(uint8_t *sSend, uint8_t id)
{
    uint8_t			cnt= 0;
    uint8_t           len;

    sSend[cnt++] = 0x02;
    sSend[cnt++] = id;						// 01:Wind Speed ID, 02:Wind Direction
    sSend[cnt++] = 0x02;					// Data Read Command
    sSend[cnt++] = 0x01;					// Data Length
    sSend[cnt++] = 0x01;					// Dummy Data
    len = cnt - 1;
    sSend[cnt++] = make_sum(&sSend[1], len);
    sSend[cnt++] = 0x03;

    return(cnt);
}

bool is_hjWindFrame(uint8_t *frame,uint16_t len)
{
  uint8_t sum=0;

  if(frame[0] == 0x02)
  {
    sum = make_sum(&frame[1],frame[3]+3);
    if(sum == frame[len-2])
    {
      return true;
    }
  }

  return false;
}


typedef struct hjWind_cfg_s
{
  driver_t *rs485_io;
}hjWind_cfg_t;





float read_hjWind(void *driver,uint8_t type,uint8_t *err);
void hjWind_set(void *handle, wind_set_option_t option, void *value);

wind_api_t hjWind_api={.read=read_hjWind,.set=hjWind_set};



float read_hjWind(void *driver,uint8_t channel,uint8_t *err)
{
  uint8_t send[10];
  uint8_t recv[10];
  uint16_t len;
  uint16_t windData=0;
  float retVal = NAN;

  hjWind_cfg_t *cfg = ((driver_t *)driver)->cfg;

  len = WindSendFrameMake(send, channel);

  driver_rs485_send(cfg->rs485_io,send,len);

  //10배된 값이 수신됨됨
  len = driver_rs485_recv(cfg->rs485_io,recv,sizeof(recv),50);//독라이트 테스트시 50ms이상은 되어야함함

  if(len && is_hjWindFrame(recv,len))
  {
    if(channel == HJ_WIND_CHANNEL_DIRECTION)
    {
      if(((recv[1] & 0x7f) == 0x02))			
      {		
        *err = 0;
        memcpy(&windData,&recv[4],2);
      }
    }
    else if(channel == HJ_WIND_CHANNEL_SPEED)
    {
      if(((recv[1] & 0x7f) == 0x01))			
      {		
        *err = 0;
        memcpy(&windData,&recv[4],2);
      }
    }
  }
  else
  {
    *err = 1;
    return NAN;
  }
  
  return (float)((float)windData/10.0);
}

void hjWind_set(void *handle, wind_set_option_t option, void *value)
{

}


hjWind_cfg_t hjWind_cfg;
driver_t hjWind;

driver_t *hjwind_open(uint8_t num,void *opt)
{
  uart_config_t uart_config;
  rs485_config_t *rs485_config =  opt;

  

  if(hjWind.opened == true)
  {
    return &hjWind;
  }

  uart_config.baud = rs485_config->baud;
  uart_config.parityIdx = rs485_config->parityIdx;
  uart_config.dataLen = 8;
  uart_config.stop_bit = 1;


  
  hjWind_cfg.rs485_io = driver_rs485_open((int)rs485_config->port,&uart_config);
  hjWind.opened = true;
  hjWind.cfg = &hjWind_cfg;
  hjWind.api = &hjWind_api;

  return &hjWind;
}
