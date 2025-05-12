
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "app_version.h"
#include "utile.h"
#include "config_app.h"
#include "utile_time.h"
#include "aws_data.h"
#include "crc16_ccitt.h"
#include "aws_data.h"
#include "aws_kma.h"

//규격서 프로토콜 버전
#define PROTOCOL_YEAR  2018
#define PROTOCOL_MONTH 2
#define PROTOCOL_DAY   1

//규격서 자료형식 번호
#define DATA_TYPE_UNUSED_0       0  // 미사용 (하위호환성)
#define DATA_TYPE_UNUSED_1       1  // 미사용 (하위호환성)
#define DATA_TYPE_UNUSED_2       2  // 미사용 (하위호환성)
#define DATA_TYPE_GENERAL        3  // 일반용
#define DATA_TYPE_AGRICULTURAL   4  // 농관용
#define DATA_TYPE_OBSERVATION    5  // 관측요소에 따라 부여 (5～255 범위)

#define KMA3_DATA_LEN 135 //Ⅶ 자료내용 영역에 전송되는 데이터의 총 길이 고정임


#define             DCFAIL_BIT          0x0001
#define             BATTERYFAIL_BIT     0x0002
#define             AC110V_BIT          0x0000          //     2 3:AC 전압    --> 00:110 V, 01:220V, 11:AC Off 
#define             AC220V_BIT          0x0004
#define             ACOFF_BIT           0x000C
#define             LOGGERDOOR_BIT      0x0010          //     4 : 로거잠금상태 : 0 : 닫힘 , 1 : 열림              
// sMin
#define             WINDSPEEDFAIL_BIT   0x0001
#define             WINDDIRECFAIL_BIT   0x0002
#define             TEMPERATUREFAIL_BIT 0x0004
#define             RAINDETECTFAIL_BIT  0x0008
#define             RAINFALLFAIL_BIT    0x0010
#define             HUMIDITYFAIL_BIT    0x0020
#define             BAROMETRICFAIL_BIT  0x0040
#define             FANFAIL_BIT         0x0080
// sMax
#define             RAINFAIL_BIT        0x0001







    // 센서 상태를 8바이트 *8 총 64bit 전송한다.
    // 미리 센서상태를 설정한다.
uint8_t g_sensorStatus_kma3[8];  // 64개의 센서의 상태 표시
void kma3_set_sensor_error(eSENSOR_LIST_t sensorNum)
{
  int quot;
  int rem;

  quot = sensorNum / sizeof(g_sensorStatus_kma3);
  rem  = sensorNum % sizeof(g_sensorStatus_kma3);

  g_sensorStatus_kma3[quot] |= 1 << rem;
}

void kma3_clear_sensor_error(eSENSOR_LIST_t sensorNum)
{
  int quot;
  int rem;

  quot = sensorNum / sizeof(g_sensorStatus_kma3);
  rem = sensorNum % sizeof(g_sensorStatus_kma3);

  g_sensorStatus_kma3[quot] &= ~(1 << rem);
}

bool kma_is_sensor_error(eSENSOR_LIST_t sensor_num)
{
  int quot;
  int rem;

  quot = sensor_num / sizeof(g_sensorStatus_kma3);
  rem = sensor_num % sizeof(g_sensorStatus_kma3);

  if (g_sensorStatus_kma3[quot] & (1 << rem))
  {
    return true;
  }
  else
  {
    return false;
  }
}


void kma_update_sensor_err(eSENSOR_LIST_t sensor_num, uint8_t err)
{
  if (err)
  {
    kma3_set_sensor_error(sensor_num);
  }
  else
  {
    kma3_clear_sensor_error(sensor_num);
  }
}

/*
bit 0 풍향
bit 1 풍속
bit 2 온도
bit 3 강수유무
bit 4 강수량센서
bit 5 습도 센서
bit 6 기압 센서 

bit 15 FAN
 */
void make_sensorStatus_kma3(uint8_t sensorState[8], uint8_t status)
{


  if(status & WINDSPEEDFAIL_BIT)
  {
    kma3_set_sensor_error(A3_WIND_SPEED);
  }
  else
  {
    kma3_clear_sensor_error(A3_WIND_SPEED);
  }

  if(status & WINDDIRECFAIL_BIT)
  {
    kma3_set_sensor_error(A2_WIND_DIRECTION);
  }
  else
  {
    kma3_clear_sensor_error(A2_WIND_DIRECTION);
  }

  if(status & TEMPERATUREFAIL_BIT)
  {
    kma3_set_sensor_error(A1_TEMPERATURE);
  }
  else
  {
    kma3_clear_sensor_error(A1_TEMPERATURE);
  }


  if(status & RAINDETECTFAIL_BIT)
  {
    kma3_set_sensor_error(A8_RAIN_PRESENT);
  }
  else
  {
    kma3_clear_sensor_error(A8_RAIN_PRESENT);
  }


  if(status & RAINFALLFAIL_BIT)
  {
    kma3_set_sensor_error(A6_RAINFALL_DOT5_1MM);
  }
  else
  {
    kma3_clear_sensor_error(A6_RAINFALL_DOT5_1MM);
  }


  if(status & HUMIDITYFAIL_BIT)
  {
    kma3_set_sensor_error(A10_RELATIVE_HUMIDITY);
  }
  else
  {
    kma3_clear_sensor_error(A10_RELATIVE_HUMIDITY);
  }


  if(status & BAROMETRICFAIL_BIT)
  {
    kma3_set_sensor_error(A7_PRESSURE);
  }
  else
  {
    kma3_clear_sensor_error(A7_PRESSURE);
  }




  sensorState[0] = g_sensorStatus_kma3[0];
  sensorState[1] = g_sensorStatus_kma3[1];
  sensorState[2] = g_sensorStatus_kma3[2];
  sensorState[3] = g_sensorStatus_kma3[3];
  sensorState[4] = g_sensorStatus_kma3[4];
  sensorState[5] = g_sensorStatus_kma3[5];
  sensorState[6] = g_sensorStatus_kma3[6];
  sensorState[7] = g_sensorStatus_kma3[7];
}



void kma3_unpack(uint8_t *packet,kma_req_t *req)
{
  uint16_t usData;

  usData = GetWord((uint8_t *)&packet[0]);

  req->header = usData;
  req->protocol_year  = packet[2];
  req->protocol_month = packet[3];
  req->protocol_day   = packet[4];

  req->year  = packet[5];
  req->month = packet[6];
  req->day   = packet[7];
  req->hour  = packet[8];
  req->min   = packet[9];
  req->sec   = packet[10];

  usData = GetWord((uint8_t *)&packet[11]);
  req->password = usData;

  usData = GetWord((uint8_t *)&packet[13]);
  req->id = usData;

  memcpy(req->cmd,&packet[15],10);

  usData = GetWord((uint8_t *)&packet[25]);

  req->crc = usData;

  usData = GetWord((uint8_t *)&packet[27]);

  req->end = usData; 
}




uint16_t make_kma3_resp(uint8_t *out,
                        char dataType,
                        uint8_t nt[5],
                        uint8_t dataNum,
                        uint16_t id,
                        uint8_t *data,
                        uint16_t dataLen)
{

  uint16_t  cnt = 0;
  uint8_t version[3];

  switch(config.eth_protocol)
  {
    case 3://KMA3 153바이트형
    version[0] = PROTOCOL_YEAR%100;
    version[1] = PROTOCOL_MONTH;
    version[2] = 1;
    break;
    default:
    version[0] = PROTOCOL_YEAR%100;
    version[1] = PROTOCOL_MONTH;
    version[2] = 1;
    break;
  }




  SetWord(&out[cnt], 0xFAFB);                 //Ⅰ시작 표시 
  cnt    += 2;
  
  out[cnt++] =   version[0];//Ⅱ 프로토콜 버전 년
  out[cnt++] =   version[1];//Ⅱ 프로토콜 버전 월
  out[cnt++] =   version[2];//Ⅱ 프로토콜 버전 월

  out[cnt++] = nt[0];    //Ⅲ 날짜 년
  out[cnt++] = nt[1];    //Ⅲ 날짜 월
  out[cnt++] = nt[2];    //Ⅲ 날짜 일
  out[cnt++] = nt[3];    //Ⅲ 날짜 시
  out[cnt++] = nt[4];    //Ⅲ 날짜 분

  out[cnt++] = dataType; //Ⅳ 자료구분
  out[cnt++] = dataNum;  //Ⅴ 자료형식 번호
  SetWord(&out[cnt],id); //Ⅵ 지점번호
  cnt += 2;

  memcpy(&out[cnt], data, dataLen);//Ⅶ 자료 내용
  cnt            += dataLen;

  SetWord(&out[cnt],crc16_ccitt_table(&out[2], cnt - 2)); //Ⅷ CRC16-CCITT
  cnt            += 2;

  SetWord(&out[cnt],0xFFFE);                  //Ⅸ 끝표시
  cnt            += 2;

  return(cnt);
}


int16_t get_sensorVal_kma3(sensor_t *sensor)
{

  int16_t val = -999;  

  return val;
}

uint32_t make_kma3_data_unusedSesor(uint8_t *lpSend, uint16_t lpSendSize, kma_data_ex_t *aws)
{
  uint32_t cnt = 0;
  const int16_t unusedSensor = -999;
  int16_t temp=0;
  memset(lpSend,0,lpSendSize);

  if(lpSendSize < KMA3_DATA_LEN)
  {
    return 0;
  }


SetWord(&lpSend[cnt], aws->temperature.enable ? aws->temperature.data : unusedSensor); // A-1 기온
cnt += 2;
SetWord(&lpSend[cnt], aws->wind_direction_avg.enable ? aws->wind_direction_avg.data : unusedSensor); // A-2 풍향
cnt += 2;
SetWord(&lpSend[cnt], aws->wind_speed_avg.enable ? aws->wind_speed_avg.data : unusedSensor); // A-3 풍속
cnt += 2;
SetWord(&lpSend[cnt], aws->wind_direction_instant.enable ? aws->wind_direction_instant.data : unusedSensor); // A-4 순간 풍향
cnt += 2;
SetWord(&lpSend[cnt], aws->wind_speed_instant.enable ? aws->wind_speed_instant.data : unusedSensor); // A-5 순간 풍속
cnt += 2;
SetWord(&lpSend[cnt], aws->precipitation.enable ? aws->precipitation.data : unusedSensor); // A-6 강수량
cnt += 2;
SetWord(&lpSend[cnt], aws->pressure.enable ? aws->pressure.data : unusedSensor); // A-7 기압
cnt += 2;
SetWord(&lpSend[cnt], aws->precipitation_presence.enable ? aws->precipitation_presence.data : unusedSensor); // A-8 강수유무
cnt += 2;
SetWord(&lpSend[cnt], aws->snowfall.enable ? aws->snowfall.data : unusedSensor); // A-9 적설
cnt += 2;
SetWord(&lpSend[cnt], aws->relative_humidity.enable ? aws->relative_humidity.data : unusedSensor); // A-10 상대습도
cnt += 2;
SetWord(&lpSend[cnt], aws->solar_radiation.enable ? aws->solar_radiation.data : unusedSensor); // A-11 강수량
cnt += 2;
SetWord(&lpSend[cnt], aws->solar_radiation.enable ? aws->solar_radiation.data : unusedSensor); // B-1 일사
cnt += 2;
SetWord(&lpSend[cnt], aws->sunshine_duration.enable ? aws->sunshine_duration.data : unusedSensor); // B-2 일조
cnt += 2;
SetWord(&lpSend[cnt], aws->surface_temperature.enable ? aws->surface_temperature.data : unusedSensor); // B-3 지면온도
cnt += 2;
SetWord(&lpSend[cnt], aws->grass_temperature.enable ? aws->grass_temperature.data : unusedSensor); // B-4 초상온도
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_5cm.enable ? aws->soil_temperature_5cm.data : unusedSensor); // B-5 지중온도 5cm
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_10cm.enable ? aws->soil_temperature_10cm.data : unusedSensor); // B-6 지중온도 10cm
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_20cm.enable ? aws->soil_temperature_20cm.data : unusedSensor); // B-7 지중온도 20cm
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_30cm.enable ? aws->soil_temperature_30cm.data : unusedSensor); // B-8 지중온도 30cm
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_50cm.enable ? aws->soil_temperature_50cm.data : unusedSensor); // B-9 지중온도 50cm
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_1m.enable ? aws->soil_temperature_1m.data : unusedSensor); // B-10 지중온도 1.0m
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_1_5m.enable ? aws->soil_temperature_1_5m.data : unusedSensor); // B-11 지중온도 1.5m
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_3m.enable ? aws->soil_temperature_3m.data : unusedSensor); // B-12 지중온도 3.0m
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_temperature_5m.enable ? aws->soil_temperature_5m.data : unusedSensor); // B-13 지중온도 5.0m
cnt += 2;
SetWord(&lpSend[cnt], aws->cloud_height_1st.enable ? aws->cloud_height_1st.data : unusedSensor); // C-1 1층 운고
cnt += 2;
SetWord(&lpSend[cnt], aws->cloud_height_2nd.enable ? aws->cloud_height_2nd.data : unusedSensor); // C-2 2층 운고
cnt += 2;
SetWord(&lpSend[cnt], aws->cloud_height_3rd.enable ? aws->cloud_height_3rd.data : unusedSensor); // C-3 3층 운고
cnt += 2;
SetWord(&lpSend[cnt], aws->cloud_amount.enable ? aws->cloud_amount.data : unusedSensor); // C-4 운량
cnt += 2;
SetWord(&lpSend[cnt], aws->visibility.enable ? aws->visibility.data : unusedSensor); // C-5 시정
cnt += 2;
SetWord(&lpSend[cnt], aws->pm10_concentration.enable ? aws->pm10_concentration.data : unusedSensor); // C-6 PM10
cnt += 2;
SetWord(&lpSend[cnt], aws->pm25_concentration.enable ? aws->pm25_concentration.data : unusedSensor); // C-7 PM2.5
cnt += 2;
SetWord(&lpSend[cnt], aws->net_radiation.enable ? aws->net_radiation.data : unusedSensor); // C-8 순복사
cnt += 2;
SetWord(&lpSend[cnt], aws->total_radiation.enable ? aws->total_radiation.data : unusedSensor); // C-9 전천복사
cnt += 2;
SetWord(&lpSend[cnt], aws->reflected_radiation.enable ? aws->reflected_radiation.data : unusedSensor); // C-10 반사복사
cnt += 2;
SetWord(&lpSend[cnt], aws->direct_radiation.enable ? aws->direct_radiation.data : unusedSensor); // C-11 직달일사
cnt += 2;
SetWord(&lpSend[cnt], aws->current_weather.enable ? aws->current_weather.data : unusedSensor); // C-12 현재일기
cnt += 2;

// temp0 예비 데이터는 enable 여부 없는 경우 생략 가능 (필요시 모두 넣을 수 있음)
SetWord(&lpSend[cnt], aws->temp0[0].enable ? aws->temp0[0].data : unusedSensor);
cnt += 2;
SetWord(&lpSend[cnt], aws->temp0[1].enable ? aws->temp0[1].data : unusedSensor);
cnt += 2;
SetWord(&lpSend[cnt], aws->temp0[2].enable ? aws->temp0[2].data : unusedSensor);
cnt += 2;
SetWord(&lpSend[cnt], aws->temp0[3].enable ? aws->temp0[3].data : unusedSensor);
cnt += 2;

SetWord(&lpSend[cnt], aws->soil_moisture_10cm.enable ? aws->soil_moisture_10cm.data : unusedSensor); // N-1
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_moisture_20cm.enable ? aws->soil_moisture_20cm.data : unusedSensor); // N-2
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_moisture_30cm.enable ? aws->soil_moisture_30cm.data : unusedSensor); // N-3
cnt += 2;
SetWord(&lpSend[cnt], aws->soil_moisture_50cm.enable ? aws->soil_moisture_50cm.data : unusedSensor); // N-4
cnt += 2;
SetWord(&lpSend[cnt], aws->illuminance.enable ? aws->illuminance.data : unusedSensor); // N-5
cnt += 2;
SetWord(&lpSend[cnt], aws->wind_speed_1_5m.enable ? aws->wind_speed_1_5m.data : unusedSensor); // N-6
cnt += 2;
SetWord(&lpSend[cnt], aws->wind_speed_4m.enable ? aws->wind_speed_4m.data : unusedSensor); // N-7
cnt += 2;
SetWord(&lpSend[cnt], aws->instant_wind_speed_1_5m.enable ? aws->instant_wind_speed_1_5m.data : unusedSensor); // N-8
cnt += 2;
SetWord(&lpSend[cnt], aws->instant_wind_speed_4m.enable ? aws->instant_wind_speed_4m.data : unusedSensor); // N-9
cnt += 2;
SetWord(&lpSend[cnt], aws->temperature_0_5m.enable ? aws->temperature_0_5m.data : unusedSensor); // N-10
cnt += 2;
SetWord(&lpSend[cnt], aws->temperature_4m.enable ? aws->temperature_4m.data : unusedSensor); // N-11
cnt += 2;
SetWord(&lpSend[cnt], aws->humidity_0_5m.enable ? aws->humidity_0_5m.data : unusedSensor); // N-12
cnt += 2;
SetWord(&lpSend[cnt], aws->humidity_4m.enable ? aws->humidity_4m.data : unusedSensor); // N-13
cnt += 2;

// temp1 예비
for (int i = 0; i < 9; i++) {
  SetWord(&lpSend[cnt], aws->temp1[i].enable ? aws->temp1[i].data : unusedSensor);
  cnt += 2;
}

SetWord(&lpSend[cnt], aws->tacometer.enable ? aws->tacometer.data : unusedSensor); // I-1 타코미터
cnt += 2;


  make_sensorStatus_kma3(&lpSend[cnt],0);              //8바이트 
  cnt += 8;

  lpSend[cnt++] = (uint8_t)aws->volateStatus;  // 상태 (DC 전압, 밧데리, 전압, 로거 잠금)

  return(cnt);

}


void check_sensor_emul(void)
{
  
}

//순간 자료
uint16_t kma3_cmd_AI(uint8_t *recv,uint8_t *send)
{
  uint8_t nt[5];
  uint8_t data[200];
  uint16_t len;
  kma_req_t *req = (kma_req_t *)recv;
  DATE_TIME_BUF *pDate;
  kma_data_ex_t *kma_data = &g_kma_inst_ex;



  pDate  = &Date_Time;
  nt[0]  = pDate->Year%100;
  nt[1]  = pDate->Month;
  nt[2]  = pDate->Day;
  nt[3]  = pDate->Hour;
  nt[4]  = pDate->Min;

  len = make_kma3_data_unusedSesor(data,sizeof(data),kma_data);
  len = make_kma3_resp(send,req->cmd[1],nt,DATA_TYPE_GENERAL,config.id,data,len);

  return len;
}


//1분 자료
uint16_t kma3_cmd_AB(uint8_t *recv,uint8_t *send)
{
  uint8_t nt[5];
  uint8_t data[200];
  uint16_t len;
  kma_req_t *req = (kma_req_t *)recv;
  DATE_TIME_BUF *pDate;
  kma_data_ex_t *aws = &g_kma_1min_ex;

  pDate  = &Date_Time;
  nt[0]  = pDate->Year%100;
  nt[1]  = pDate->Month;
  nt[2]  = pDate->Day;
  nt[3]  = pDate->Hour;
  nt[4]  = pDate->Min;


  len = make_kma3_data_unusedSesor(data,sizeof(data),aws);
  len = make_kma3_resp(send,req->cmd[1],nt,DATA_TYPE_GENERAL,config.id,data,len);

  return len;
}




//1분 과거 자료
uint16_t kma3_cmd_AQ(uint8_t *recv,uint8_t *send)
{
  uint8_t data[200];
  uint16_t len;
  kma_req_t *req = (kma_req_t *)recv;
  DATE_TIME_BUF mOldDate;
  DATE_TIME_BUF *pDate;
  uint8_t nt[5];

  kma_data_ex_t *aws = NULL;
  time_t cur_t, befhour_t, poll_t;
  int nIdx;


  mOldDate.Year   = req->year + 2000;
  mOldDate.Month  = req->month; 
  mOldDate.Day    = req->day;
  mOldDate.Hour   = req->hour;
  mOldDate.Min    = req->min;
  mOldDate.Sec    = req->sec;

  pDate           = &mOldDate;


  nt[0]  = req->year;
  nt[1]  = req->month;
  nt[2]  = req->day;
  nt[3]  = req->hour;
  nt[4]  = req->min;

    poll_t 		  = SetTime(pDate->Year, pDate->Month, pDate->Day, pDate->Hour, pDate->Min, 0);
    befhour_t 	= SetTime(Date_Time.Year, Date_Time.Month, Date_Time.Day, Date_Time.Hour, 0, 0);	
    cur_t     	= SetTime(Date_Time.Year, Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min, 0);
    if((poll_t == cur_t) && (Date_Time.Sec < 2))
      return 0;//

    if((poll_t >= befhour_t) && (poll_t <= cur_t))
    {	// Memory에서 가져온다
        nIdx        = (pDate->Min + 59) % 60;
       // memcpy((char *)pAws, (char *)&mMinAwsLog[nIdx],  sizeof(AWS_DATA_STRUCT));
    }
    else
    {	// SD Card에서 가져온다	
     //  MinLogRead(pDate, pAws);
    }


  len = make_kma3_data_unusedSesor(data,sizeof(data),aws);
  len = make_kma3_resp(send,req->cmd[1],nt,DATA_TYPE_GENERAL,config.id,data,len);

  return len;
}




uint16_t make_kma3_resp_RODTWC(uint8_t *out,uint16_t outSize,
                                 uint16_t id,uint8_t cmd,const char *result)
{
  uint16_t cnt = 0 ;
  uint8_t version[3];

  switch(config.eth_protocol)
  {
    case 3://KMA3 153바이트형
    version[0] = PROTOCOL_YEAR%100;
    version[1] = PROTOCOL_MONTH;
    version[2] = PROTOCOL_DAY;
    break;
    default:
    version[0] = PROTOCOL_YEAR%100;
    version[1] = PROTOCOL_MONTH;
    version[2] = PROTOCOL_DAY;
    break;
  }

  if(outSize<16)
  {
    return 0;
  }
  memset(out,0,outSize);

  SetWord(&out[cnt],0xFAFB); 
  cnt += 2;

  out[cnt++] = version[0];//Ⅱ 프로토콜 버전 년
  out[cnt++] =  version[1];//Ⅱ 프로토콜 버전 월
  out[cnt++] = version[2];//Ⅱ 프로토콜 버전 월

  SetWord(&out[cnt],id); 
  cnt += 2;

  out[cnt++] = cmd;//AR,O,D,T,W,C

  memcpy(&out[cnt],result,4);
  cnt += 4;

  SetWord(&out[cnt],crc16_ccitt_table(&out[2],cnt-2)); 
  cnt +=2;

  SetWord(&out[cnt],0xFFFE); 
  cnt +=2;

  return cnt;
}
//로거 버전 응답
uint32_t kma3_cmdAV(uint8_t *packet,uint8_t *txBuff)
{
  uint8_t data[30];
  uint16_t cnt = 0 ;
  uint8_t version[3];

  switch(config.eth_protocol)
  {
    case 3://KMA3 153바이트형
    version[0] = PROTOCOL_YEAR%100;
    version[1] = PROTOCOL_MONTH;
    version[2] = PROTOCOL_DAY;
    break;
    default:
    version[0] = PROTOCOL_YEAR%100;
    version[1] = PROTOCOL_MONTH;
    version[2] = PROTOCOL_DAY;
    break;
  }


  memset(data,0,sizeof(data));

  SetWord(&data[cnt],0xFAFB); 
  cnt += 2;

  data[cnt++] = version[0];//Ⅱ 프로토콜 버전 년
  data[cnt++] = version[0];//Ⅱ 프로토콜 버전 월
  data[cnt++] = version[0];//Ⅱ 프로토콜 버전 월

  SetWord(&data[cnt],config.id); 
  cnt += 2;

  memcpy(&data[cnt],"1.0.0             ",18);
  cnt += 18;

  SetWord(&data[cnt],crc16_ccitt_table(&data[2],cnt-2)); 
  cnt +=2;

  SetWord(&data[cnt],0xFFFE); 
  cnt +=2;

  memcpy(txBuff,data,cnt);

  return cnt;
}


//리셋
uint16_t kma3_cmd_AR(uint8_t *recv,uint8_t *send)
{
  uint8_t packet[50];
  uint16_t len;
  kma_req_t *req = (kma_req_t *)recv;

  len = make_kma3_resp_RODTWC(packet,sizeof(packet),config.id,req->cmd[1],"OKAY");

  memcpy(send,packet,len);

  return len;
}


uint16_t kma3_cmd_AO(uint8_t *recv,uint8_t *send)
{
  uint8_t packet[50];
  uint16_t len;
  kma_req_t *req = (kma_req_t *)recv;

  len = make_kma3_resp_RODTWC(packet,sizeof(packet),config.id,req->cmd[1],"OKAY");

  memcpy(send,packet,len);

  return len;
}


//시간 설정
uint16_t kma3_cmd_AT(uint8_t *recv,uint8_t *send)
{
  uint8_t packet[50];
  uint16_t len;
  kma_req_t *req = (kma_req_t *)recv;
  DATE_TIME_BUF nt;


    // 날짜 시간 설정                
    nt.Year   = req->year + 2000;
    nt.Month  = req->month; 
    nt.Day    = req->day;
    nt.Hour   = req->hour;
    nt.Min    = req->min;
    nt.Sec    = req->sec;

   // RtccWriteDateTime(&nt);

  len = make_kma3_resp_RODTWC(packet,sizeof(packet),config.id,req->cmd[1],"OKAY");

  memcpy(send,packet,len);

  return len;
}

//암호 설정
uint16_t kma3_cmd_AW(uint8_t *recv,uint8_t *send)
{
  char temp[10];

  uint8_t packet[50];
  uint16_t len;
  kma_req_t *req = ( kma_req_t*)recv;


  snprintf(temp,sizeof(temp),"%d",req->password);
 // memcpy(config.password,temp,4);//TODO:

  len = make_kma3_resp_RODTWC(packet,sizeof(packet),config.id,req->cmd[1],"OKAY");

  memcpy(send,packet,len);

  return len;
}

//데이터 삭제
uint16_t kma3_cmd_AC(uint8_t *recv,uint8_t *send)
{
  uint8_t packet[50];
  uint16_t len;
  kma_req_t *req = (kma_req_t *)recv;

  len = make_kma3_resp_RODTWC(packet,sizeof(packet),config.id,req->cmd[1],"OKAY");

  memcpy(send,packet,len);

  return len;
}


uint16_t kma3_cmd_AP(uint8_t *recv,uint8_t *send)
{
  uint8_t packet[50];
  uint16_t len;
  uint32_t i;

  kma_req_t *req = (kma_req_t *)recv;

  for(i = 0; i < 4; i++)
  {
  //  Config.m_cCDMAIP[i] = req->cmd[3+i];
  }

 // Config.m_usCDMATcpPort = GetWord((uchar *)&req->cmd[7]);



  len = make_kma3_resp_RODTWC(packet,sizeof(packet),config.id,req->cmd[1],"OKAY");




  memcpy(send,packet,len);


  return len;
}



int32_t kma2_cmd_handler(uint8_t *packet,uint16_t paLen,uint8_t *txBuff,uint16_t tLen,uint8_t source)//,uint16_t packetLen)
{

  int32_t i;
  uint32_t len;
  kma_req_t *req = (kma_req_t*)packet;
  kma_req_t request;
  uint16_t cmd_cnt;

  
  kma3_unpack(packet,&request);

  req = &request;

  if(req->cmd[1] == 'D')//지점번호 설정
  {
    if(req->password == config.id)
    {
      config.id = req->id;
    }
  }

  if(req->id != config.id)
  {
    return 0;
  }

  cmd_cnt = coutntof_kma_cmd();

  for(i = 0 ;i < cmd_cnt ;i++)
  {
    if(strncmp(kma_cmd[i].cmdName,req->cmd,strlen(kma_cmd[i].cmdName))==0)
    {
      switch(kma_cmd[i].cmd)
      {
        case eAI://순간자료
        len = kma3_cmd_AI((uint8_t *)req,txBuff);
        break;
        case eAB://1분자료(최근 1분 자료 요구)
        len = kma3_cmd_AB((uint8_t *)req,txBuff);//동일한 함수로 처리
        break;
        case eAQ://1분 과거 자료(C한에 과거 시간을 입력하여 과거자료 요구)
        len = kma3_cmd_AQ((uint8_t *)req,txBuff);//동일한 함수로 처리
        break;
        case eAV://데이터로거 버전
        len = kma3_cmdAV((uint8_t *)req,txBuff);
        break;
        case eAR://데이터로거 리셋
        len = kma3_cmd_AR((uint8_t *)req,txBuff);
        osDelay(10);
        break;
        case eAO://전원리셋 또는 모뎀 리셋 또는 적설센서 리셋
        len = kma3_cmd_AO((uint8_t *)req,txBuff);
        break;
        case eAD://지점번호 설정

        break;
        case eAT://날짜,시간 설정
        len = kma3_cmd_AT((uint8_t *)req,txBuff);
        break;
        case eAW://암호 설정
        len = kma3_cmd_AW((uint8_t *)req,txBuff);
        break;
        case eAC://저장데이터 삭제
        len =  kma3_cmd_AC((uint8_t *)req,txBuff);
        break;
        case eAP: // 국립공원 Protocol전용 : 원격 CDMA 원격 TCP  IP & PORT 변경							
        len = kma3_cmd_AP((uint8_t *)req,txBuff);
         break;
      }
      break;
    }
  }

  return len;
}
