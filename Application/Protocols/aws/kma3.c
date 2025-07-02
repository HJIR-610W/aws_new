
#include "kma3.h"

#include <stdint.h>
#include <string.h>
#include "aws_data.h"
#include "config_app.h"
#include "util_crc16_ccitt.h"
#include "kma_define.h"
#include "util_memory.h"
#include "util_time.h"

#define KMA3_DATA_LEN 135  // Ⅶ 자료내용 영역에 전송되는 데이터의 총 길이 고정임

#define DCFAIL_BIT 0x0001
#define BATTERYFAIL_BIT 0x0002
#define AC110V_BIT 0x0000  //     2 3:AC 전압    --> 00:110 V, 01:220V, 11:AC Off
#define AC220V_BIT 0x0004
#define ACOFF_BIT 0x000C
#define LOGGERDOOR_BIT 0x0010  //     4 : 로거잠금상태 : 0 : 닫힘 , 1 : 열림
// sMin
#define WINDSPEEDFAIL_BIT 0x0001
#define WINDDIRECFAIL_BIT 0x0002
#define TEMPERATUREFAIL_BIT 0x0004
#define RAINDETECTFAIL_BIT 0x0008
#define RAINFALLFAIL_BIT 0x0010
#define HUMIDITYFAIL_BIT 0x0020
#define BAROMETRICFAIL_BIT 0x0040
#define FANFAIL_BIT 0x0080
// sMax
#define RAINFAIL_BIT 0x0001

// 센서 상태를 8바이트 *8 총 64bit 전송한다.
// 미리 센서상태를 설정한다.
uint8_t g_sensorStatus_kma3[8];  // 64개의 센서의 상태 표시
void kma3_set_sensor_error(eSENSOR_TYPE_t sensorNum)
{
  int quot;
  int rem;

  quot = sensorNum / sizeof(g_sensorStatus_kma3);
  rem = sensorNum % sizeof(g_sensorStatus_kma3);

  g_sensorStatus_kma3[quot] |= 1 << rem;
}

void kma3_clear_sensor_error(eSENSOR_TYPE_t sensorNum)
{
  int quot;
  int rem;

  quot = sensorNum / sizeof(g_sensorStatus_kma3);
  rem = sensorNum % sizeof(g_sensorStatus_kma3);

  g_sensorStatus_kma3[quot] &= ~(1 << rem);
}





bool kma_is_sensor_error(eSENSOR_TYPE_t sensor_num)
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

void kma_update_sensor_err(eSENSOR_TYPE_t sensor_num, uint8_t err)
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
  if (status & WINDSPEEDFAIL_BIT)
  {
    kma3_set_sensor_error(A3_WIND_SPEED);
  }
  else
  {
    kma3_clear_sensor_error(A3_WIND_SPEED);
  }

  if (status & WINDDIRECFAIL_BIT)
  {
    kma3_set_sensor_error(A2_WIND_DIRECTION);
  }
  else
  {
    kma3_clear_sensor_error(A2_WIND_DIRECTION);
  }

  if (status & TEMPERATUREFAIL_BIT)
  {
    kma3_set_sensor_error(A1_TEMPERATURE);
  }
  else
  {
    kma3_clear_sensor_error(A1_TEMPERATURE);
  }

  if (status & RAINDETECTFAIL_BIT)
  {
    kma3_set_sensor_error(A8_RAIN_PRESENT);
  }
  else
  {
    kma3_clear_sensor_error(A8_RAIN_PRESENT);
  }

  if (status & RAINFALLFAIL_BIT)
  {
    kma3_set_sensor_error(A6_RAINFALL_DOT5_1MM);
  }
  else
  {
    kma3_clear_sensor_error(A6_RAINFALL_DOT5_1MM);
  }

  if (status & HUMIDITYFAIL_BIT)
  {
    kma3_set_sensor_error(A10_RELATIVE_HUMIDITY);
  }
  else
  {
    kma3_clear_sensor_error(A10_RELATIVE_HUMIDITY);
  }

  if (status & BAROMETRICFAIL_BIT)
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



uint32_t make_kma3_data_unusedSesor(uint8_t *lpSend, uint16_t lpSendSize, kma_data_ex_t *aws)
{
  uint32_t cnt = 0;
  const int16_t unusedSensor = -999;

  memset(lpSend, 0, lpSendSize);

  if (lpSendSize < KMA3_DATA_LEN)
  {
    return 0;
  }

  SetWord(&lpSend[cnt],
          aws->temperature.enable ? aws->temperature.data : unusedSensor);  // A-1 기온
  cnt += 2;
  SetWord(&lpSend[cnt], aws->wind_direction_avg.enable ? aws->wind_direction_avg.data
                                                       : unusedSensor);  // A-2 풍향
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->wind_speed_avg.enable ? aws->wind_speed_avg.data : unusedSensor);  // A-3 풍속
  cnt += 2;
  SetWord(&lpSend[cnt], aws->wind_direction_instant.enable ? aws->wind_direction_instant.data
                                                           : unusedSensor);  // A-4 순간 풍향
  cnt += 2;
  SetWord(&lpSend[cnt], aws->wind_speed_instant.enable ? aws->wind_speed_instant.data
                                                       : unusedSensor);  // A-5 순간 풍속
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->precipitation.enable ? aws->precipitation.data : unusedSensor);  // A-6 강수량
  cnt += 2;
  SetWord(&lpSend[cnt], aws->pressure.enable ? aws->pressure.data : unusedSensor);  // A-7 기압
  cnt += 2;
  SetWord(&lpSend[cnt], aws->precipitation_presence.enable ? aws->precipitation_presence.data
                                                           : unusedSensor);  // A-8 강수유무
  cnt += 2;
  SetWord(&lpSend[cnt], aws->snowfall.enable ? aws->snowfall.data : unusedSensor);  // A-9 적설
  cnt += 2;
  SetWord(&lpSend[cnt], aws->relative_humidity.enable ? aws->relative_humidity.data
                                                      : unusedSensor);  // A-10 상대습도
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->solar_radiation.enable ? aws->solar_radiation.data : unusedSensor);  // A-11 강수량
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->solar_radiation.enable ? aws->solar_radiation.data : unusedSensor);  // B-1 일사
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->sunshine_duration.enable ? aws->sunshine_duration.data : unusedSensor);  // B-2 일조
  cnt += 2;
  SetWord(&lpSend[cnt], aws->surface_temperature.enable ? aws->surface_temperature.data
                                                        : unusedSensor);  // B-3 지면온도
  cnt += 2;
  SetWord(&lpSend[cnt], aws->grass_temperature.enable ? aws->grass_temperature.data
                                                      : unusedSensor);  // B-4 초상온도
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_5cm.enable ? aws->soil_temperature_5cm.data
                                                         : unusedSensor);  // B-5 지중온도 5cm
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_10cm.enable ? aws->soil_temperature_10cm.data
                                                          : unusedSensor);  // B-6 지중온도 10cm
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_20cm.enable ? aws->soil_temperature_20cm.data
                                                          : unusedSensor);  // B-7 지중온도 20cm
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_30cm.enable ? aws->soil_temperature_30cm.data
                                                          : unusedSensor);  // B-8 지중온도 30cm
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_50cm.enable ? aws->soil_temperature_50cm.data
                                                          : unusedSensor);  // B-9 지중온도 50cm
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_1m.enable ? aws->soil_temperature_1m.data
                                                        : unusedSensor);  // B-10 지중온도 1.0m
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_1_5m.enable ? aws->soil_temperature_1_5m.data
                                                          : unusedSensor);  // B-11 지중온도 1.5m
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_3m.enable ? aws->soil_temperature_3m.data
                                                        : unusedSensor);  // B-12 지중온도 3.0m
  cnt += 2;
  SetWord(&lpSend[cnt], aws->soil_temperature_5m.enable ? aws->soil_temperature_5m.data
                                                        : unusedSensor);  // B-13 지중온도 5.0m
  cnt += 2;
  SetWord(&lpSend[cnt], aws->cloud_height_1st.enable ? aws->cloud_height_1st.data
                                                     : unusedSensor);  // C-1 1층 운고
  cnt += 2;
  SetWord(&lpSend[cnt], aws->cloud_height_2nd.enable ? aws->cloud_height_2nd.data
                                                     : unusedSensor);  // C-2 2층 운고
  cnt += 2;
  SetWord(&lpSend[cnt], aws->cloud_height_3rd.enable ? aws->cloud_height_3rd.data
                                                     : unusedSensor);  // C-3 3층 운고
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->cloud_amount.enable ? aws->cloud_amount.data : unusedSensor);  // C-4 운량
  cnt += 2;
  SetWord(&lpSend[cnt], aws->visibility.enable ? aws->visibility.data : unusedSensor);  // C-5 시정
  cnt += 2;
  SetWord(&lpSend[cnt], aws->pm10_concentration.enable ? aws->pm10_concentration.data
                                                       : unusedSensor);  // C-6 PM10
  cnt += 2;
  SetWord(&lpSend[cnt], aws->pm25_concentration.enable ? aws->pm25_concentration.data
                                                       : unusedSensor);  // C-7 PM2.5
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->net_radiation.enable ? aws->net_radiation.data : unusedSensor);  // C-8 순복사
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->total_radiation.enable ? aws->total_radiation.data : unusedSensor);  // C-9 전천복사
  cnt += 2;
  SetWord(&lpSend[cnt], aws->reflected_radiation.enable ? aws->reflected_radiation.data
                                                        : unusedSensor);  // C-10 반사복사
  cnt += 2;
  SetWord(&lpSend[cnt], aws->direct_radiation.enable ? aws->direct_radiation.data
                                                     : unusedSensor);  // C-11 직달일사
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->current_weather.enable ? aws->current_weather.data : unusedSensor);  // C-12 현재일기
  cnt += 2;


  SetWord(&lpSend[cnt], aws->temp0_0.enable ? aws->temp0_0.data : unusedSensor);
  cnt += 2;
  SetWord(&lpSend[cnt], aws->temp0_1.enable ? aws->temp0_1.data : unusedSensor);
  cnt += 2;
  SetWord(&lpSend[cnt], aws->temp0_2.enable ? aws->temp0_2.data : unusedSensor);
  cnt += 2;
  SetWord(&lpSend[cnt], aws->temp0_3.enable ? aws->temp0_3.data : unusedSensor);
  cnt += 2;

  SetWord(&lpSend[cnt],
          aws->soil_moisture_10cm.enable ? aws->soil_moisture_10cm.data : unusedSensor);  // N-1
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->soil_moisture_20cm.enable ? aws->soil_moisture_20cm.data : unusedSensor);  // N-2
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->soil_moisture_30cm.enable ? aws->soil_moisture_30cm.data : unusedSensor);  // N-3
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->soil_moisture_50cm.enable ? aws->soil_moisture_50cm.data : unusedSensor);  // N-4
  cnt += 2;
  SetWord(&lpSend[cnt], aws->illuminance.enable ? aws->illuminance.data : unusedSensor);  // N-5
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->wind_speed_1_5m.enable ? aws->wind_speed_1_5m.data : unusedSensor);  // N-6
  cnt += 2;
  SetWord(&lpSend[cnt], aws->wind_speed_4m.enable ? aws->wind_speed_4m.data : unusedSensor);  // N-7
  cnt += 2;
  SetWord(&lpSend[cnt], aws->instant_wind_speed_1_5m.enable ? aws->instant_wind_speed_1_5m.data
                                                            : unusedSensor);  // N-8
  cnt += 2;
  SetWord(&lpSend[cnt], aws->instant_wind_speed_4m.enable ? aws->instant_wind_speed_4m.data
                                                          : unusedSensor);  // N-9
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->temperature_0_5m.enable ? aws->temperature_0_5m.data : unusedSensor);  // N-10
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->temperature_4m.enable ? aws->temperature_4m.data : unusedSensor);  // N-11
  cnt += 2;
  SetWord(&lpSend[cnt],
          aws->humidity_0_5m.enable ? aws->humidity_0_5m.data : unusedSensor);  // N-12
  cnt += 2;
  SetWord(&lpSend[cnt], aws->humidity_4m.enable ? aws->humidity_4m.data : unusedSensor);  // N-13
  cnt += 2;

  // temp1 예비

    SetWord(&lpSend[cnt], aws->temp1_0.enable ? aws->temp1_0.data : unusedSensor);
    cnt += 2;

    SetWord(&lpSend[cnt], aws->temp1_1.enable ? aws->temp1_1.data : unusedSensor);
    cnt += 2;
    
    SetWord(&lpSend[cnt], aws->temp1_2.enable ? aws->temp1_2.data : unusedSensor);
    cnt += 2;
    
    SetWord(&lpSend[cnt], aws->temp1_3.enable ? aws->temp1_3.data : unusedSensor);
    cnt += 2;
    
    SetWord(&lpSend[cnt], aws->temp1_4.enable ? aws->temp1_4.data : unusedSensor);
    cnt += 2;
    
    SetWord(&lpSend[cnt], aws->temp1_5.enable ? aws->temp1_5.data : unusedSensor);
    cnt += 2;
    
    SetWord(&lpSend[cnt], aws->temp1_6.enable ? aws->temp1_6.data : unusedSensor);
    cnt += 2;
    
    SetWord(&lpSend[cnt], aws->temp1_7.enable ? aws->temp1_7.data : unusedSensor);
    cnt += 2;
    
    SetWord(&lpSend[cnt], aws->temp1_8.enable ? aws->temp1_8.data : unusedSensor);
    cnt += 2;

  SetWord(&lpSend[cnt],
          aws->tacometer.enable ? aws->tacometer.data : unusedSensor);  // I-1 타코미터
  cnt += 2;

  for(int i = 0 ; i< 8; i++)
  {
    lpSend[cnt++]  = aws->X_sensorStatus[i];
  }

  lpSend[cnt++] = (uint8_t)aws->Y_volateStatus;  // 상태 (DC 전압, 밧데리, 전압, 로거 잠금)

  return (cnt);
}

uint16_t make_kma3_resp(uint8_t *out, char dataType,  uint8_t dataNum, uint16_t id,
                        uint8_t *data, uint16_t dataLen)
{
  uint16_t cnt = 0;


  SetWord(&out[cnt], 0xFAFB);  // Ⅰ시작 표시
  cnt += 2;

  out[cnt++] = KMA3_PROTOCOL_YEAR % 100;  // Ⅱ 프로토콜 버전 년
  out[cnt++] = KMA3_PROTOCOL_MONTH;       // Ⅱ 프로토콜 버전 월
  out[cnt++] = KMA3_PROTOCOL_DAY;         // Ⅱ 프로토콜 버전 월

  //년도가 설정되어있지 않다면
  if(out[cnt]==0)
  {
    out[cnt++] = Date_Time.Year%100;  // Ⅲ 날짜 년
    out[cnt++] = Date_Time.Month;    // Ⅲ 날짜 월
    out[cnt++] = Date_Time.Day;     // Ⅲ 날짜 일
    out[cnt++] = Date_Time.Hour;    // Ⅲ 날짜 시
    out[cnt++] = Date_Time.Min;     // Ⅲ 날짜 분
  }
  else
  {
    cnt += 5;
  }

  out[cnt++] = dataType;   // Ⅳ 자료구분
  out[cnt++] = dataNum;    // Ⅴ 자료형식 번호
  SetWord(&out[cnt], id);  // Ⅵ 지점번호
  cnt += 2;

  memcpy(&out[cnt], data, dataLen);  // Ⅶ 자료 내용
  cnt += dataLen;

  SetWord(&out[cnt], crc16_ccitt_table(&out[2], cnt - 2));  // Ⅷ CRC16-CCITT
  cnt += 2;

  SetWord(&out[cnt], 0xFFFE);  // Ⅸ 끝표시
  cnt += 2;

  return (cnt);
}






uint16_t make_kma3_resp_RODTWC(uint8_t *out, uint16_t outSize, uint16_t id, uint8_t cmd,
        const char *result)
{
uint16_t cnt = 0;


if (outSize < 16)
{
  return 0;
}

memset(out, 0, outSize);

SetWord(&out[cnt], 0xFAFB);
cnt += 2;

out[cnt++] = KMA3_PROTOCOL_YEAR % 100; // Ⅱ 프로토콜 버전 년
out[cnt++] = KMA3_PROTOCOL_MONTH;      // Ⅱ 프로토콜 버전 월
out[cnt++] = KMA3_PROTOCOL_DAY;        // Ⅱ 프로토콜 버전 월

SetWord(&out[cnt], id);
cnt += 2;

out[cnt++] = cmd;  // AR,O,D,T,W,C

memcpy(&out[cnt], result, 4);
cnt += 4;

SetWord(&out[cnt], crc16_ccitt_table(&out[2], cnt - 2));
cnt += 2;

SetWord(&out[cnt], 0xFFFE);
cnt += 2;

return cnt;
}

void kma3_set_sensor_status(eSENSOR_TYPE_t sensor_num, uint8_t sensor[8])
{
  int quot;
  int rem;

  quot = sensor_num / sizeof(sensor);
  rem = sensor_num % sizeof(sensor);

  sensor[quot] |= 1 << rem;
}

void kma3_clear_sensor_status(eSENSOR_TYPE_t sensor_num, uint8_t sensor[8])
{
  int quot;
  int rem;

  quot = sensor_num / sizeof(sensor);
  rem = sensor_num % sizeof(sensor);

  sensor[quot] &= ~(1 << rem);
}

bool kma3_is_sensor_error(eSENSOR_TYPE_t sensor_num, uint8_t sensor[8])
{
  int quot;
  int rem;

  quot = sensor_num / sizeof(sensor);
  rem = sensor_num % sizeof(sensor);

  if (sensor[quot] & (1 << rem))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void kma3_update_sensor_status(eSENSOR_TYPE_t sensor_num, uint8_t sensor[8], uint8_t err)
{
  if (err)
  {
    kma3_set_sensor_status(sensor_num, sensor);
  }
  else
  {
    kma3_clear_sensor_status(sensor_num, sensor);
  }
}
