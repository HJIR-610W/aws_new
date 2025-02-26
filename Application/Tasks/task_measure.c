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

#include "driver_do.h"
#include "task_logging.h"
#include "task_measure.h"
#include "usDelay.h"
#include "utile_time.h"
#include "utile.h"
#include "app_measure.h"


extern uint16_t calcCRC(uint8_t* Buffer, uint32_t length);


const osThreadAttr_t kMeasureTask_attributes = {
  .name = "measureTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityHigh,
};



uint32_t g_start_time;
uint32_t g_elased_time;
sensor_t g_sensor_copy[SENSOR_COUNT_MAX];


/**
 * @brief 사용하는 센서츨 초기화한다.
 */
void sensor_init(void)
{
  sensor_t *p_sensor;
  p_sensor = g_sensor_copy;

  adc_init();//ADC 항상 초기화 

  if(p_sensor[A1_TEMPERATURE].type)
  {
    temperature_init();
  }
  if(p_sensor[A2_WIND_DIRECTION].type)
  {
    windDirection_init();
  }

  if(p_sensor[A3_WIND_SPEED].type)
  {
    windSpeed_init();
  }

  if(p_sensor[A9_SNOW_DEPTH].type)
  {
    snow_init(&p_sensor[A9_SNOW_DEPTH]);
  }

  if(p_sensor[A6_RAINFALL_DOT5_1MM].type)
  {
    rain_init(&p_sensor[A6_RAINFALL_DOT5_1MM]);
  }

  if(p_sensor[A8_RAIN_PRESENT].type)
  {
    rainPresent_init();
  }



}


extern void file_test(void);



//실제 수집된 데이터를 AWS에서 요구하는 형태로 저장해야한다.

#define AWS_CVT_TEMP(x)           (x==TEMP_ERR_VAL?-9999:(x+100)*10)
#define AWS_CVT_HUMI(x)           (x==HUMI_ERR_VAL?-9999:(x*10))
#define AWS_CVT_BAROMETER(x)      (x==BAROMETER_ERR_VAL?-9999:(x*10))
#define AWS_CVT_WIND_DIRECTION(x) (x==WIND_DIRECTION_ERR_VAL?-9999:(x*10))

#define AWS_CVT_WIND_SPEED(x) (x==WIND_SPEED_ERR_VAL?-9999:(x*10))

#define AWS_CVT_DEFAULT(x) (x*10)


/**
 * @brief 센서 데이터를 AWS 자료형으로 변환환
 */
void  cvt_sensorToAWS(sensor_data_t *p_data,kma_data_t *p_kma)
{
  bool status;

 // 1. 기온 (1분 평균)
 p_kma->temperature = (int16_t)AWS_CVT_TEMP(p_data[A1_TEMPERATURE].data.f);

 // 2. 풍향 (1분 평균)
 p_kma->wind_direction_avg = (int16_t)AWS_CVT_WIND_DIRECTION(p_data[A2_WIND_DIRECTION].data.f);

 // 3. 풍속 (1분 평균)
 p_kma->wind_speed_avg = (int16_t)AWS_CVT_WIND_SPEED(p_data[A3_WIND_SPEED].data.f);

 // 4. 풍향 (1분 순간)
 p_kma->wind_direction_instant= (int16_t)AWS_CVT_WIND_DIRECTION(p_data[A4_INSTANT_WIND_DIRECTION].data.f);

 // 5. 풍속 (1분 순간)
 p_kma->wind_speed_instant= (int16_t)AWS_CVT_WIND_SPEED(p_data[A5_INSTANT_WIND_SPEED].data.f);

 // 6. 강수량 (0.5/1.0 mm)
 p_kma->precipitation= (int16_t)AWS_CVT_DEFAULT(p_data[A6_RAINFALL_DOT5_1MM].data.f);

 // 7. 기압 (1분 평균 현지 기압)
 p_kma->pressure= (int16_t)AWS_CVT_BAROMETER(p_data[A7_PRESSURE].data.f);

 // 8. 강수 유무
 if(p_data[A8_RAIN_PRESENT].data.b==true)
 {
  p_kma->precipitation_presence = 10;
 }
 else
 {
  p_kma->precipitation_presence = 0;
 }


 // 9. 적설
 p_kma->snowfall= (int16_t)AWS_CVT_DEFAULT(p_data[A9_SNOW_DEPTH].data.f);// 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 4095 (관측값 * 10)

 // 10. 상대습도 (1분 평균)
 p_kma->relative_humidity= (int16_t)(AWS_CVT_HUMI(p_data[A10_RELATIVE_HUMIDITY].data.f));// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 11. 강수량 (0.1 mm)
 p_kma->precipitation_fine= (int16_t)(p_data[A11_RAINFALL_DOT1MM].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767

 // 1. 일사 (누적값)
 p_kma->solar_radiation= (int16_t)(p_data[B1_SOLAR_RADIATION].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 [관측값(MJ/m²) * 100]

 // 2. 일조 (누적 시간)
 p_kma->sunshine_duration= (int16_t)(p_data[B2_SUNSHINE_DURATION].data.f*1000);// 사용비트: 15, 유효범위: 0 ~ 65535 (인치 코드), 표현범위: 0 ~ 65535 [누적시간(초 단위)]

 // 3. 지면온도 (1분 평균)
 p_kma->surface_temperature= (int16_t)(p_data[B3_GROUND_TEMPERATURE].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 4. 초상온도 (1분 평균)
 p_kma->grass_temperature= (int16_t)(p_data[B4_SURFACE_TEMPERATURE].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 5. 지중온도 (5cm, 1분 평균)
 p_kma->soil_temperature_5cm= (int16_t)(p_data[B5_SOIL_TEMPERATURE_5CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 6. 지중온도 (10cm, 1분 평균)
 p_kma->soil_temperature_10cm= (int16_t)(p_data[B6_SOIL_TEMPERATURE_10CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 7. 지중온도 (20cm, 1분 평균)
 p_kma->soil_temperature_20cm= (int16_t)(p_data[B7_SOIL_TEMPERATURE_20CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 8. 지중온도 (30cm, 1분 평균)
 p_kma->soil_temperature_30cm= (int16_t)(p_data[B8_SOIL_TEMPERATURE_30CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 9. 지중온도 (50cm, 1분 평균)
 p_kma->soil_temperature_50cm= (int16_t)(p_data[B9_SOIL_TEMPERATURE_50CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 10. 지중온도 (1.0m, 1분 평균)
 p_kma->soil_temperature_1m= (int16_t)(p_data[B10_SOIL_TEMPERATURE_100CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 11. 지중온도 (1.5m, 1분 평균)
 p_kma->soil_temperature_1_5m= (int16_t)(p_data[B11_SOIL_TEMPERATURE_150CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 12. 지중온도 (3.0m, 1분 평균)
 p_kma->soil_temperature_3m= (int16_t)(p_data[B12_SOIL_TEMPERATURE_300CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

 // 13. 지중온도 (5.0m, 1분 평균)
 p_kma->soil_temperature_5m= (int16_t)(p_data[B13_SOIL_TEMPERATURE_500CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]


 // 1. 1층 운고 (1분 평균)
 p_kma->cloud_height_1st= (int16_t)(p_data[C1_CLOUD_BASE1].data.f*1000);// 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

 // 2. 2층 운고 (1분 평균)
 p_kma->cloud_height_2nd= (int16_t)(p_data[C2_CLOUD_BASE2].data.f*1000);// 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

 // 3. 3층 운고 (1분 평균)
 p_kma->cloud_height_3rd= (int16_t)(p_data[C3_CLOUD_BASE3].data.f*1000);// 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

 // 4. 운량
 p_kma->cloud_amount= (int16_t)(p_data[C4_CLOUD_COVER].data.f*1000);// 사용비트: 3, 유효범위: 0 ~ 15 (인치 코드), 표현범위: 0 ~ 10 (관측값)

 // 5. 시정 (1분 평균)
 p_kma->visibility= (int16_t)(p_data[C5_VISIBILITY].data.f*1000);// 사용비트: 15, 유효범위: 0 ~ 65535 (인치 코드), 표현범위: 0 ~ 50000 (관측값[m])

 // 6. PM10 (분진농도)
 p_kma->pm10_concentration= (int16_t)(p_data[C6_PM10].data.f*1000);// 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)

 // 7. PM2.5 (분진농도)
 p_kma->pm25_concentration= (int16_t)(p_data[C7_PM2DOT5].data.f*1000);// 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)

 // 8. 순복사 (1분 평균)
 p_kma->net_radiation= (int16_t)(p_data[C8_NET_RADIATION].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 9. 전천복사 (1분 평균)
 p_kma->total_radiation=(int16_t)( p_data[C9_TOTAL_RADIATION].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 10. 반사복사 (1분 평균)
 p_kma->reflected_radiation= (int16_t)(p_data[C10_REFLECTED_RADIATION].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 11. 직달복사 (1분 평균)
 p_kma->direct_radiation= (int16_t)(p_data[C11_DIRECT_SOLAR].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 12. 현재 일기
 p_kma->current_weather= (int16_t)(p_data[C12_CURRENT_WEATHER].data.f*1000);// 사용비트: 6, 유효범위: 0 ~ 127 (인치 코드), 표현범위: 0 ~ 99 (관측값)



 // 1. 토양수분 (10 cm)
 p_kma->soil_moisture_10cm=(int16_t) (p_data[N1_SOIL_MOISTURE_10CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 2. 토양수분 (20 cm)
 p_kma->soil_moisture_20cm= (int16_t)(p_data[N2_SOIL_MOISTURE_20CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 3. 토양수분 (30 cm)
 p_kma->soil_moisture_30cm= (int16_t)(p_data[N3_SOIL_MOISTURE_30CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 4. 토양수분 (50 cm)
 p_kma->soil_moisture_50cm= (int16_t)(p_data[N4_SOIL_MOISTURE_50CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 5. 조도량 (1분 평균)
 p_kma->illuminance= (int16_t)(p_data[N5_ILLUMINANCE].data.f*1000);// 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값 * 100)

 // 6. 풍속 (1.5 m, 1분 평균)
 p_kma->wind_speed_1_5m= (int16_t)(p_data[N6_WIND_VELOCITY_150CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 7. 풍속 (4.0 m, 1분 평균)
 p_kma->wind_speed_4m=(int16_t) (p_data[N7_WIND_VELOCITY_400CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 8. 순간 풍속 (1.5 m)
 p_kma->instant_wind_speed_1_5m= (int16_t)(p_data[N8_INSTANT_VELOCITY_150CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 9. 순간 풍속 (4.0 m)
 p_kma->instant_wind_speed_4m= (int16_t)(p_data[N9_INSTANT_VELOCITY_400CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 10. 기온 (0.5 m)
 p_kma->temperature_0_5m= (int16_t)(p_data[N10_AIR_TEMPERATURE_50CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

 // 11. 기온 (4.0 m)
 p_kma->temperature_4m= (int16_t)(p_data[N11_AIR_TEMPERATURE_400CM].data.f*1000);// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

 // 12. 습도 (0.5 m, 1분 평균)
 p_kma->humidity_0_5m= (int16_t)(p_data[N12_HUMIDITY_50CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 13. 습도 (4.0 m, 1분 평균)
 p_kma->humidity_4m = (int16_t)(p_data[N13_HUMIDITY_400CM].data.f*1000);// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)
 p_kma->tacometer   = (int16_t)(p_data[I1_TACHOMETER].data.f*1000);

 p_kma->sensorStatus[0] = 0; 
 p_kma->sensorStatus[1] = 0; 
 p_kma->sensorStatus[2] = 0; 
 p_kma->sensorStatus[3] = 0; 
 p_kma->sensorStatus[4] = 0; 
 p_kma->sensorStatus[5] = 0; 
 p_kma->sensorStatus[6] = 0; 
 p_kma->sensorStatus[7] = 0; 

 p_kma->volateStatus = 0; 

 p_kma->crc = 0;
 p_kma->init = true;

}


void measure_250ms(DATE_TIME_BUF *ct)
{
  uint8_t err;
  uint16_t i;
  uint8_t sample_cnt=0;
  float avg;
  float adc;
  float max;
  float min;
  sensor_t *p_sensor = g_sensor_copy;

  for( i = 0 ;i < _countof(g_sensor_copy);i++)
  {
      if(p_sensor[i].type)
      {
        switch(i)
        {
          case A2_WIND_DIRECTION:
          adc =  read_sensor_windDirection(&p_sensor[A2_WIND_DIRECTION],&err);
          break;
          case A3_WIND_SPEED:
          adc =  read_sensor_windSpeed(&p_sensor[A3_WIND_SPEED],&err);
          break;

        }
    }
  }
}


void measure_1s(DATE_TIME_BUF *ct,sensor_t *sensor,uint16_t sensor_cnt)
{
  int32_t rain_pulse;
  uint8_t err;
  uint8_t sample_cnt=0;
  uint16_t i;
  uint16_t data;
  float avg;
  float adc;
  float max;
  float min;

  g_start_time = mcu_get_clk();
  for( i = 0 ;i < sensor_cnt;i++)
  {
      if(sensor[i].type)
      {
        switch(i)
        {
          case A1_TEMPERATURE://10초마다 샘플링하고 6개 자료의 평균을 1분자료
            if((ct->Sec%10)==0)
            {
              adc =  read_sensor_temperature(&sensor[A1_TEMPERATURE],&err);
              sample_temperature(adc);//샘플을 수집
              sensor_data[A1_TEMPERATURE].data.f = get_avg_temperature();//평균
            }
          break;
          case A2_WIND_DIRECTION:
          break;
          case A3_WIND_SPEED:
          break;
          case A4_INSTANT_WIND_DIRECTION:
          sensor_data[A4_INSTANT_WIND_DIRECTION].data.f = sensor_data[A2_WIND_DIRECTION].max.f;
          break;
          case A5_INSTANT_WIND_SPEED:
          sensor_data[A5_INSTANT_WIND_SPEED].data.f = sensor_data[A3_WIND_SPEED].max.f;
          break;
          case A6_RAINFALL_DOT5_1MM:
          {
            rain_pulse = read_sensor_rain(&sensor[A6_RAINFALL_DOT5_1MM],&err);//금일강수량
            sensor_data[A6_RAINFALL_DOT5_1MM].data.i += rain_pulse;//금일 우량
            sensor_data[A6_RAINFALL_DOT5_1MM].status = read_rainHallErr();
          }
            
          break;
          case A7_PRESSURE://10초마다 샘플링하고 6개 자료의 평균을 1분자료
            if((ct->Sec%10)==0)
            {
              adc =  read_sensor_barometer(&sensor[A7_PRESSURE],&err);
              sample_barometer(adc);
              sensor_data[A7_PRESSURE].data.f = get_avg_barometer();
            }
          break;
          case A8_RAIN_PRESENT:
            sensor_data[A8_RAIN_PRESENT].data.b = read_sensor_rainPresent(&sensor[A8_RAIN_PRESENT],&err);
          break;
          case A9_SNOW_DEPTH: //mm
          sensor_data[A9_SNOW_DEPTH].data.i = read_sensor_snow(&sensor[A9_SNOW_DEPTH],&err);
          break;
          case A10_RELATIVE_HUMIDITY://10초마다 샘플링하고 6개 자료의 평균을 1분자료
            if((ct->Sec%10)==0)
            {
              adc =  read_sensor_humidity(&sensor[A10_RELATIVE_HUMIDITY],&err);
              sample_humi(adc);
              sensor_data[A10_RELATIVE_HUMIDITY].data.f = get_avg_humi();
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
 update_sensorData1s();//1초마다 갱신

}

/**
 * @brief 수집된 센서 데이터를 AWS자료형으로 변환하고 저장장
 * @example 초가 바뀌고 분이 바뀐다.
 */
void min_process(DATE_TIME_BUF *ct)
{
  cvt_sensorToAWS(sensor_data,&g_kma_1min);
  os_write_sensorData(ct,&g_kma_1min,sizeof(g_kma_1min),0,1);
}

void hour_process(DATE_TIME_BUF *ct)
{
  uint16_t i;
  sensor_t *sensor = g_sensor_copy;

  for( i = 0 ;i < _countof(g_sensor_copy);i++)
  {
      if(sensor[i].type)
      {
        switch (i)
        {
        case A6_RAINFALL_DOT5_1MM:
          {
            rain_data_t *p_rain_data=0;
            p_rain_data =  sensor_data[A6_RAINFALL_DOT5_1MM].opt;
            p_rain_data->hour = 0;
          }

          break;
        
        default:
          break;
        }
      }
  }
}
void day_process(DATE_TIME_BUF *ct)
{
  uint16_t i;
  sensor_t *sensor = g_sensor_copy;
  
  for( i = 0 ;i < _countof(g_sensor_copy);i++)
  {
      if(sensor[i].type)
      {
        switch (i)
        {
        case A6_RAINFALL_DOT5_1MM:
          {
            rain_data_t *p_rain_data=0;
            p_rain_data =  sensor_data[A6_RAINFALL_DOT5_1MM].opt;
            p_rain_data->day = 0;
            sensor_data[A6_RAINFALL_DOT5_1MM].data.i  = 0 ;//금일강수량
          }

          break;
        
        default:
          break;
        }
      }
  }

}
void month_process(DATE_TIME_BUF *ct)
{
  uint16_t i;
  sensor_t *sensor = g_sensor_copy;
  
  for( i = 0 ;i < _countof(g_sensor_copy);i++)
  {
      if(sensor[i].type)
      {
        switch (i)
        {
        case A6_RAINFALL_DOT5_1MM:
          {
            rain_data_t *p_rain_data=0;
            p_rain_data =  sensor_data[A6_RAINFALL_DOT5_1MM].opt;
            p_rain_data->month = 0;
          }
          break;
        default:
          break;
        }
      }
  }

}
void year_process(DATE_TIME_BUF *ct)
{
  uint16_t i;
  sensor_t *sensor = g_sensor_copy;
  
  for( i = 0 ;i < _countof(g_sensor_copy);i++)
  {
      if(sensor[i].type)
      {
        switch (i)
        {
        case A6_RAINFALL_DOT5_1MM:
          {
            rain_data_t *p_rain_data=0;
            p_rain_data =  sensor_data[A6_RAINFALL_DOT5_1MM].opt;
            p_rain_data->year = 0;
          }
          break;
        default:
          break;
        }
      }
  }

}

void measureTask(void *arg)
{
  DATE_TIME_BUF ct;
  DATE_TIME_BUF ot;
  uint32_t tick_count;
 const uint32_t period = 250;


  os_logging_printf("measure task");

  /*
  config의 복사본으로 동작시킨다. 
  config변경해도 장비를 리셋한 후에 적용되도록 한다.
  */
  memcpy(g_sensor_copy,config.sensor,sizeof(g_sensor_copy));

  sensor_init();
  sensorData_init();

  ct = Date_Time;
  ot = ct;


  tick_count = osKernelGetTickCount();
  while(1)
  {
    ct = Date_Time;

    measure_250ms(&ct);

    if(ct.Sec != ot.Sec)
    {
      measure_1s(&ct,g_sensor_copy,_countof(g_sensor_copy));
      cvt_sensorToAWS(sensor_data,&g_kma_1s);
      
      if(ct.Min != ot.Min)
      {
        min_process(&ct);
        ot.Min = ct.Min;
      }
      ot.Sec = ct.Sec;
    }

    tick_count += period;
    
    osDelayUntil(tick_count);

  }
}

void measureTask_init(void)
{
  osThreadNew(measureTask, NULL, &kMeasureTask_attributes);
}