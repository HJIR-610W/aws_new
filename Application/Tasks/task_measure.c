#include <string.h>
#include <math.h>

#include "Sensors\temperature\temperature.h"
#include "Sensors\wind_speed\wind_speed.h"
#include "Sensors\wind_direction\wind_direction.h"
#include "Sensors\snow\snow.h"
#include "Sensors\rain\rain.h"
#include "Sensors\humidity\humidity.h"
#include "Sensors\general\sensor_general.h"
#include "Sensors\soil_temperature\soil_temperature.h"
#include "Sensors\sunshine\sunshine.h"
#include "Sensors\barometer\barometer.h"
#include "app_bsp.h"
#include "app_adc.h"
#include "app_rtc.h"
#include "app_file.h"
#include "aws_data.h"
#include "app_dataLogging.h"
#include "config.h"
#include "cmsis_os.h"
#include "task_logging.h"
#include "task_measure.h"
#include "usDelay.h"
#include "utile_time.h"
#include "utile.h"

const osThreadAttr_t kMeasureTask_attributes = {
  .name = "measureTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityHigh,
};



uint32_t g_start_time;
uint32_t g_elased_time;

void sensor_init(void)
{
  sensor_t *sensor;
  sensor = config.sensor;


  if(sensor[A1_TEMPERATURE].type)
  {
    temperature_init();
  }
  if(sensor[A2_WIND_DIRECTION].type)
  {
    windDirection_init();
  }

  if(sensor[A3_WIND_SPEED].type)
  {
    windSpeed_init();
  }


  if(sensor[A9_SNOW_DEPTH].type)
  {
    snow_init(&sensor[A9_SNOW_DEPTH]);
  }

  if(sensor[A6_RAINFALL_DOT5_1MM].type)
  {
    rain_init(&sensor[A6_RAINFALL_DOT5_1MM]);
  }

  if(sensor[A8_RAIN_PRESENT].type)
  {
    rainPresent_init();
  }

}
kma_data_t g_kma_data;

extern void file_test(void);

int16_t g_data[SENSOR_COUNT_MAX];


void  make_loggingData(void)
{
 // 1. 기온 (1분 평균)
 g_kma_data.temperature = (int16_t)(sensor_data[A1_TEMPERATURE].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 (관측값 * 10)

 // 2. 풍향 (1분 평균)
 g_kma_data.wind_direction_avg= (int16_t)(sensor_data[A2_WIND_DIRECTION].data.f*1000);// 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 3599 (관측값 * 10)

 // 3. 풍속 (1분 평균)
 g_kma_data.wind_speed_avg= (int16_t)(sensor_data[A3_WIND_SPEED].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 4. 풍향 (1분 순간)
 g_kma_data.wind_direction_instant= (int16_t)(sensor_data[A4_INSTANT_WIND_DIRECTION].data.f*1000);// 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 3599 (관측값 * 10)

 // 5. 풍속 (1분 순간)
 g_kma_data.wind_speed_instant= (int16_t)(sensor_data[A5_INSTANT_WIND_SPEED].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 6. 강수량 (0.5/1.0 mm)
 g_kma_data.precipitation= (int16_t)(sensor_data[A6_RAINFALL_DOT5_1MM].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767

 // 7. 기압 (1분 평균 현지 기압)
 g_kma_data.pressure= (int16_t)(sensor_data[A7_PRESSURE].data.f*1000);// 사용비트: 13, 유효범위: 0 ~ 16383 (인치 코드), 표현범위: 5000 ~ 11000

 // 8. 강수 유무
 g_kma_data.precipitation_presence= (int16_t)(sensor_data[A8_RAIN_PRESENT].data.f*1000);// 사용비트: 3, 유효범위: 0 ~ 15 (인치 코드), 표현범위: 0 = 강수 없음, 1 = 강수 있음

 // 9. 적설
 g_kma_data.snowfall= (int16_t)(sensor_data[A9_SNOW_DEPTH].data.f*1000);// 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 4095 (관측값 * 10)

 // 10. 상대습도 (1분 평균)
 g_kma_data.relative_humidity= (int16_t)(sensor_data[A10_RELATIVE_HUMIDITY].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 11. 강수량 (0.1 mm)
 g_kma_data.precipitation_fine= (int16_t)(sensor_data[A11_RAINFALL_DOT1MM].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767

 // 1. 일사 (누적값)
 g_kma_data.solar_radiation= (int16_t)(sensor_data[B1_SOLAR_RADIATION].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 [관측값(MJ/m²) * 100]

 // 2. 일조 (누적 시간)
 g_kma_data.sunshine_duration= (int16_t)(sensor_data[B2_SUNSHINE_DURATION].data.f*1000);// 사용비트: 15, 유효범위: 0 ~ 65535 (인치 코드), 표현범위: 0 ~ 65535 [누적시간(초 단위)]

 // 3. 지면온도 (1분 평균)
 g_kma_data.surface_temperature= (int16_t)(sensor_data[B3_GROUND_TEMPERATURE].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 4. 초상온도 (1분 평균)
 g_kma_data.grass_temperature= (int16_t)(sensor_data[B4_SURFACE_TEMPERATURE].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 5. 지중온도 (5cm, 1분 평균)
 g_kma_data.soil_temperature_5cm= (int16_t)(sensor_data[B5_SOIL_TEMPERATURE_5CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 6. 지중온도 (10cm, 1분 평균)
 g_kma_data.soil_temperature_10cm= (int16_t)(sensor_data[B6_SOIL_TEMPERATURE_10CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 7. 지중온도 (20cm, 1분 평균)
 g_kma_data.soil_temperature_20cm= (int16_t)(sensor_data[B7_SOIL_TEMPERATURE_20CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 8. 지중온도 (30cm, 1분 평균)
 g_kma_data.soil_temperature_30cm= (int16_t)(sensor_data[B8_SOIL_TEMPERATURE_30CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 9. 지중온도 (50cm, 1분 평균)
 g_kma_data.soil_temperature_50cm= (int16_t)(sensor_data[B9_SOIL_TEMPERATURE_50CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 10. 지중온도 (1.0m, 1분 평균)
 g_kma_data.soil_temperature_1m= (int16_t)(sensor_data[B10_SOIL_TEMPERATURE_100CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 11. 지중온도 (1.5m, 1분 평균)
 g_kma_data.soil_temperature_1_5m= (int16_t)(sensor_data[B11_SOIL_TEMPERATURE_150CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 12. 지중온도 (3.0m, 1분 평균)
 g_kma_data.soil_temperature_3m= (int16_t)(sensor_data[B12_SOIL_TEMPERATURE_300CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 13. 지중온도 (5.0m, 1분 평균)
 g_kma_data.soil_temperature_5m= (int16_t)(sensor_data[B13_SOIL_TEMPERATURE_500CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]


 // 1. 1층 운고 (1분 평균)
 g_kma_data.cloud_height_1st= (int16_t)(sensor_data[C1_CLOUD_BASE1].data.f*1000);// 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

 // 2. 2층 운고 (1분 평균)
 g_kma_data.cloud_height_2nd= (int16_t)(sensor_data[C2_CLOUD_BASE2].data.f*1000);// 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

 // 3. 3층 운고 (1분 평균)
 g_kma_data.cloud_height_3rd= (int16_t)(sensor_data[C3_CLOUD_BASE3].data.f*1000);// 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

 // 4. 운량
 g_kma_data.cloud_amount= (int16_t)(sensor_data[C4_CLOUD_COVER].data.f*1000);// 사용비트: 3, 유효범위: 0 ~ 15 (인치 코드), 표현범위: 0 ~ 10 (관측값)

 // 5. 시정 (1분 평균)
 g_kma_data.visibility= (int16_t)(sensor_data[C5_VISIBILITY].data.f*1000);// 사용비트: 15, 유효범위: 0 ~ 65535 (인치 코드), 표현범위: 0 ~ 50000 (관측값[m])

 // 6. PM10 (분진농도)
 g_kma_data.pm10_concentration= (int16_t)(sensor_data[C6_PM10].data.f*1000);// 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)

 // 7. PM2.5 (분진농도)
 g_kma_data.pm25_concentration= (int16_t)(sensor_data[C7_PM2DOT5].data.f*1000);// 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)

 // 8. 순복사 (1분 평균)
 g_kma_data.net_radiation= (int16_t)(sensor_data[C8_NET_RADIATION].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 9. 전천복사 (1분 평균)
 g_kma_data.total_radiation=(int16_t)( sensor_data[C9_TOTAL_RADIATION].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 10. 반사복사 (1분 평균)
 g_kma_data.reflected_radiation= (int16_t)(sensor_data[C10_REFLECTED_RADIATION].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 11. 직달복사 (1분 평균)
 g_kma_data.direct_radiation= (int16_t)(sensor_data[C11_DIRECT_SOLAR].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 12. 현재 일기
 g_kma_data.current_weather= (int16_t)(sensor_data[C12_CURRENT_WEATHER].data.f*1000);// 사용비트: 6, 유효범위: 0 ~ 127 (인치 코드), 표현범위: 0 ~ 99 (관측값)



 // 1. 토양수분 (10 cm)
 g_kma_data.soil_moisture_10cm=(int16_t) (sensor_data[N1_SOIL_MOISTURE_10CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 2. 토양수분 (20 cm)
 g_kma_data.soil_moisture_20cm= (int16_t)(sensor_data[N2_SOIL_MOISTURE_20CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 3. 토양수분 (30 cm)
 g_kma_data.soil_moisture_30cm= (int16_t)(sensor_data[N3_SOIL_MOISTURE_30CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 4. 토양수분 (50 cm)
 g_kma_data.soil_moisture_50cm= (int16_t)(sensor_data[N4_SOIL_MOISTURE_50CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 5. 조도량 (1분 평균)
 g_kma_data.illuminance= (int16_t)(sensor_data[N5_ILLUMINANCE].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값 * 100)

 // 6. 풍속 (1.5 m, 1분 평균)
 g_kma_data.wind_speed_1_5m= (int16_t)(sensor_data[N6_WIND_VELOCITY_150CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 7. 풍속 (4.0 m, 1분 평균)
 g_kma_data.wind_speed_4m=(int16_t) (sensor_data[N7_WIND_VELOCITY_400CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 8. 순간 풍속 (1.5 m)
 g_kma_data.instant_wind_speed_1_5m= (int16_t)(sensor_data[N8_INSTANT_VELOCITY_150CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 9. 순간 풍속 (4.0 m)
 g_kma_data.instant_wind_speed_4m= (int16_t)(sensor_data[N9_INSTANT_VELOCITY_400CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 10. 기온 (0.5 m)
 g_kma_data.temperature_0_5m= (int16_t)(sensor_data[N10_AIR_TEMPERATURE_50CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

 // 11. 기온 (4.0 m)
 g_kma_data.temperature_4m= (int16_t)(sensor_data[N11_AIR_TEMPERATURE_400CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

 // 12. 습도 (0.5 m, 1분 평균)
 g_kma_data.humidity_0_5m= (int16_t)(sensor_data[N12_HUMIDITY_50CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 13. 습도 (4.0 m, 1분 평균)
 g_kma_data.humidity_4m = (int16_t)(sensor_data[N13_HUMIDITY_400CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)
 g_kma_data.tacometer   = (int16_t)(sensor_data[I1_TACHOMETER].data.f*1000);

 g_kma_data.sensorStatus[0] = 0; 
 g_kma_data.sensorStatus[1] = 0; 
 g_kma_data.sensorStatus[2] = 0; 
 g_kma_data.sensorStatus[3] = 0; 
 g_kma_data.sensorStatus[4] = 0; 
 g_kma_data.sensorStatus[5] = 0; 
 g_kma_data.sensorStatus[6] = 0; 
 g_kma_data.sensorStatus[7] = 0; 

 g_kma_data.volateStatus = 0; 

}
sensor_t g_sensor_copy[SENSOR_COUNT_MAX];

void measureTask(void *arg)
{
  DATE_TIME_BUF ct;
  DATE_TIME_BUF ot;
  uint8_t err;

  sensor_t *sensor;
  uint32_t i;
  uint32_t cnt = _countof(config.sensor);
  uint8_t sample_cnt=0;
  bool sec_changed = true;
  float avg;
  float adc;
  float max;
  float min;
  

  adc_init();

  sensor_init();
  sensorData_init();

  memcpy(g_sensor_copy,config.sensor,sizeof(g_sensor_copy));

  sensor = g_sensor_copy;
  os_logging_printf("measure task");

  ct = Date_Time;
    
  memset(g_data,0xff,sizeof(g_data));

  driver_t *stm32_adc;

  battery_init();

  while(1)
  {
    ct.Sec = Date_Time.Sec;
    ct.Min = Date_Time.Min;
    
    sec_changed = false;
    if(ct.Sec != ot.Sec)
    {

      
      sec_changed = true;
      if(ct.Min != ot.Min)
      {
      make_loggingData();
      os_write_sensorData(&ct,&g_kma_data,sizeof(g_kma_data),0,1);
        ot.Min = ct.Min;
      }
      ot.Sec = ct.Sec;
    }
    osDelay(250);

    g_start_time = mcu_get_clk();
    for( i = 0 ;i < _countof(config.sensor);i++)
    {
        if(sensor[i].type)
        {
          switch(i)
          {
            case A1_TEMPERATURE:
            if(sec_changed)
            {
              adc =  read_sensor_temperature(&sensor[A1_TEMPERATURE],&err);
              avg = sensor_data[A1_TEMPERATURE].avg;
              sensor_data[A1_TEMPERATURE].sample_cnt++;
              sample_cnt = sensor_data[A1_TEMPERATURE].sample_cnt;
              avg = recursiveAvg(avg,adc,sample_cnt);
              if(sample_cnt == 10)//
              {
                sensor_data[A1_TEMPERATURE].sample_cnt = 0;
                sensor_data[A1_TEMPERATURE].data.f = avg;
              }

           }
            break;
            case A2_WIND_DIRECTION:
            adc =  read_sensor_windDirection(&sensor[A2_WIND_DIRECTION],&err);
            avg = sensor_data[A2_WIND_DIRECTION].avg;
            sensor_data[A2_WIND_DIRECTION].sample_cnt++;
            sample_cnt  = sensor_data[A2_WIND_DIRECTION].sample_cnt;
            avg = recursiveAvg(avg,adc,sample_cnt);
            if(sample_cnt == 12)
            {
              sensor_data[A2_WIND_DIRECTION].sample_cnt = 0;
              sensor_data[A2_WIND_DIRECTION].data.f = avg;
            }
      //      windDirectionSample1Min[windDirectionSample1MinCnt++] = adc;


            break;
            case A3_WIND_SPEED:
            sensor_data[A3_WIND_SPEED].data.f = read_sensor_windDirection(&sensor[A3_WIND_SPEED],&err);
            break;
            case A4_INSTANT_WIND_DIRECTION:
            sensor_data[A4_INSTANT_WIND_DIRECTION].data.f = sensor_data[A2_WIND_DIRECTION].data.f;
            break;
            case A5_INSTANT_WIND_SPEED:
            sensor_data[A5_INSTANT_WIND_SPEED].data.f = sensor_data[A3_WIND_SPEED].data.f;
            break;
            case A6_RAINFALL_DOT5_1MM:
            if(sec_changed)
            {
              sensor_data[A6_RAINFALL_DOT5_1MM].data.i += read_sensor_rain(&sensor[A6_RAINFALL_DOT5_1MM],&err);//금일강수량
            }
            break;
            case A7_PRESSURE:
            if(sec_changed)
            {
              adc =  read_sensor_barometer(&sensor[A7_PRESSURE],&err);
              avg = sensor_data[A7_PRESSURE].avg;
              sensor_data[A7_PRESSURE].sample_cnt++;
              sample_cnt =  sensor_data[A7_PRESSURE].sample_cnt;
              avg = recursiveAvg(avg,adc,sample_cnt);
              if(sample_cnt==10)//
              {
                sensor_data[A7_PRESSURE].sample_cnt = 0;
                sensor_data[A7_PRESSURE].data.f = avg;
              }
            }
            break;
            case A8_RAIN_PRESENT:
            if(sec_changed)
            {
              sensor_data[A8_RAIN_PRESENT].data.i= read_sensor_rainPresent(&sensor[A8_RAIN_PRESENT],&err);
            }
            break;
            case A9_SNOW_DEPTH:
            sensor_data[A9_SNOW_DEPTH].data.i = read_sensor_snow(&sensor[A9_SNOW_DEPTH],&err);
            break;
            case A10_RELATIVE_HUMIDITY:
            if(sec_changed)
            {
              adc =  read_sensor_humidity(&sensor[A10_RELATIVE_HUMIDITY],&err);
              avg = sensor_data[A10_RELATIVE_HUMIDITY].avg;
              sensor_data[A10_RELATIVE_HUMIDITY].sample_cnt++;
              sample_cnt  = sensor_data[A10_RELATIVE_HUMIDITY].sample_cnt;
              avg = recursiveAvg(avg,adc,sample_cnt);
              if(sample_cnt==10)//
              {
                sensor_data[A10_RELATIVE_HUMIDITY].sample_cnt = 0;
                sensor_data[A10_RELATIVE_HUMIDITY].data.f = avg;
              }
            }
            break;
            case B1_SOLAR_RADIATION:
            sensor_data[B1_SOLAR_RADIATION].data.f      = read_sensor_sunshine(&sensor[B1_SOLAR_RADIATION],&err);
            break;
            case B2_SUNSHINE_DURATION:
            sensor_data[B2_SUNSHINE_DURATION].data.f    = read_sensor_sunshine(&sensor[B2_SUNSHINE_DURATION],&err);
            break;
            case B5_SOIL_TEMPERATURE_5CM:
            sensor_data[B5_SOIL_TEMPERATURE_5CM].data.f = read_sensor_soilTemp(&sensor[B5_SOIL_TEMPERATURE_5CM],SOIL_TEMP_5CM,&err);
            break;
            case B6_SOIL_TEMPERATURE_10CM:
            sensor_data[B6_SOIL_TEMPERATURE_10CM].data.f = read_sensor_soilTemp(&sensor[B6_SOIL_TEMPERATURE_10CM],SOIL_TEMP_10CM,&err);
            break;
            case B7_SOIL_TEMPERATURE_20CM:
            sensor_data[B7_SOIL_TEMPERATURE_20CM].data.f = read_sensor_soilTemp(&sensor[B7_SOIL_TEMPERATURE_20CM],SOIL_TEMP_20CM,&err);
            break;
            case B8_SOIL_TEMPERATURE_30CM:
            sensor_data[B8_SOIL_TEMPERATURE_30CM].data.f = read_sensor_soilTemp(&sensor[B8_SOIL_TEMPERATURE_30CM],SOIL_TEMP_30CM,&err);
            break;
            case B9_SOIL_TEMPERATURE_50CM:
            sensor_data[B9_SOIL_TEMPERATURE_50CM].data.f = read_sensor_soilTemp(&sensor[B9_SOIL_TEMPERATURE_50CM],SOIL_TEMP_50CM,&err);
            break;
            case B10_SOIL_TEMPERATURE_100CM:
            sensor_data[B10_SOIL_TEMPERATURE_100CM].data.f = read_sensor_soilTemp(&sensor[B10_SOIL_TEMPERATURE_100CM],SOIL_TEMP_100CM,&err);
            break;
            case B11_SOIL_TEMPERATURE_150CM:
            sensor_data[B11_SOIL_TEMPERATURE_150CM].data.f = read_sensor_soilTemp(&sensor[B11_SOIL_TEMPERATURE_150CM],SOIL_TEMP_150CM,&err);
            break;
            default:
            sensor_data[i].data.f = read_sensorGeneral(&sensor[i],&err);
            break;
          }
      }
    }

   g_elased_time = cal_elapsed_us(g_start_time);

  }
  
}

void measureTask_init(void)
{
  osThreadNew(measureTask, NULL, &kMeasureTask_attributes);
}