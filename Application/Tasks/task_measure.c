#include <string.h>
#include <math.h>

#include "cmsis_os2.h"

#include "Sensors\temperature\temperature.h"
#include "Sensors\humidity\humidity.h"
#include "Sensors\wind_speed\wind_speed.h"
#include "Sensors\wind_direction\wind_direction.h"
#include "Sensors\snow\snow.h"
#include "Sensors\rain\rain.h"
#include "Sensors\rain_present\rain_present.h"
#include "Sensors\soil_temperature\soil_temperature.h"
#include "Sensors\sunshine\sunshine.h"
#include "Sensors\barometer\barometer.h"
#include "Sensors\solar_radiation\solar_radiation.h"
#include "Sensors\general\sensor_general.h"
#include "Sensors\general\general_adc.h"
#include "app_bsp.h"
#include "app_adc.h"
#include "app_rtc.h"
#include "app_file.h"
#include "aws_data.h"
#include "app_dataLogging.h"
#include "config.h"


#include "driver_do.h"
#include "task_logging.h"
#include "task_measure.h"
#include "usDelay.h"
#include "utile_time.h"
#include "utile.h"
#include "app_measure.h"



#define MEASURE_PERIOD_MS 250
extern uint16_t calcCRC(uint8_t* Buffer, uint32_t length);


const osThreadAttr_t kMeasureTask_attributes = {
  .name = "measureTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityRealtime1,
};



uint32_t g_start_time;
uint32_t g_elased_time;
uint32_t g_elased_max=0;
sensor_t g_sensor_copy[SENSOR_COUNT_MAX];


driver_t *sensor_driver[50];


//센서 모델에 해당하는 드라이버 번호를 넘겨준다.

int32_t get_driverNum(eSENSOR_MODEL_t type)
{
  int32_t num=-1;//항목 없음음

  switch(type)
  {
    case S_T_ADC:
    num = GENERAL_ADC;
    break;
    case S_T_PT100_A:
    num = TEMP_PT100_A;
    break;
    case S_T_PT100_B:
    num = TEMP_PT100_B;
    break;
    case S_T_WIND_SPEED_HJ_485:
    case S_T_WIND_DIRECTION_HJ_485:
    num = WIND_HJ;
    break;
    case S_T_SNOW_HJ_485:
    num = SNOW_HJ_485;
    break;
    case S_T_SNOW_HJ_232:
    num = SNOW_HJ_232;
    break;
    case S_T_RAIN_REED_05MM:
    num = RAIN_REED_05MM;
    break;
    case S_T_RAIN_REED_1MM:
    num = RAIN_REED_1MM;
    break;
    case S_T_RAIN_HALL_05MM:
    num = RAIN_HALL_05MM;
    break;
    case S_T_RAIN_HALL_1MM:
    num = RAIN_HALL_1MM;
    break;


  }
  return num;
}




/**
 * @brief 사용하는 센서츨 초기화한다.
 */
void sensor_init(void)
{
  uint8_t num;
  void *para=NULL;
  sensor_t *p_sensor;
  p_sensor = g_sensor_copy;

  adc_init();//ADC 항상 초기화 


  for(int i = 0 ; i < _countof(g_sensor_copy);i++)
  {
    if(p_sensor[i].type)//사용으로 설정되었는 확인
    {
      switch(i)
      {
      case A1_TEMPERATURE:
        num = get_driverNum(p_sensor[A1_TEMPERATURE].type);
        para =  get_sensor_config(&p_sensor[A1_TEMPERATURE]);
        sensor_driver[A1_TEMPERATURE] = temperature_open(num,para);
        break;
      case A10_RELATIVE_HUMIDITY:
        num = get_driverNum(p_sensor[A10_RELATIVE_HUMIDITY].type);
        para=  get_sensor_config(&p_sensor[A10_RELATIVE_HUMIDITY]);
        sensor_driver[A10_RELATIVE_HUMIDITY] = humidity_open(num,para);
        break;
      case A3_WIND_SPEED:
        num = get_driverNum(p_sensor[A3_WIND_SPEED].type);
        para = get_sensor_config(&p_sensor[A3_WIND_SPEED]);
        sensor_driver[A3_WIND_SPEED]  = windSpeed_open(num,para);
        break;
      case A2_WIND_DIRECTION:
        num = get_driverNum(p_sensor[A2_WIND_DIRECTION].type);
        para =  get_sensor_config(&p_sensor[A2_WIND_DIRECTION]);
        sensor_driver[A2_WIND_DIRECTION]  = windSpeed_open(num,para);
        break;
      case A9_SNOW_DEPTH:
        num = get_driverNum(p_sensor[A9_SNOW_DEPTH].type);
        para = get_sensor_config(&p_sensor[A9_SNOW_DEPTH]);
        sensor_driver[A9_SNOW_DEPTH]  = snow_open(num,para);
        break;
      case A6_RAINFALL_DOT5_1MM:
          num = get_driverNum(p_sensor[A6_RAINFALL_DOT5_1MM].type);
          sensor_driver[A6_RAINFALL_DOT5_1MM]  = rain_open(num,0);
        break;
      case A8_RAIN_PRESENT:
        sensor_driver[A8_RAIN_PRESENT]  = rainPresent_open(RAIN_PRESENT_DI,0);
        break;
      case A7_PRESSURE:
        num = get_driverNum(p_sensor[A7_PRESSURE].type);
        para=  get_sensor_config(&p_sensor[A7_PRESSURE]);
        sensor_driver[A7_PRESSURE]  = barometer_open(num,para);
        break;
      case B5_SOIL_TEMPERATURE_5CM:
        num = get_driverNum(p_sensor[B5_SOIL_TEMPERATURE_5CM].type);
        para=  get_sensor_config(&p_sensor[B5_SOIL_TEMPERATURE_5CM]);
        sensor_driver[B5_SOIL_TEMPERATURE_5CM]  = barometer_open(num,para);
        break;
      case B6_SOIL_TEMPERATURE_10CM:
      num = get_driverNum(p_sensor[B6_SOIL_TEMPERATURE_10CM].type);
      para=  get_sensor_config(&p_sensor[B6_SOIL_TEMPERATURE_10CM]);
      sensor_driver[B6_SOIL_TEMPERATURE_10CM]  = barometer_open(num,para);
      break;
        case B7_SOIL_TEMPERATURE_20CM:
        num = get_driverNum(p_sensor[B7_SOIL_TEMPERATURE_20CM].type);
        para=  get_sensor_config(&p_sensor[B7_SOIL_TEMPERATURE_20CM]);
        sensor_driver[B7_SOIL_TEMPERATURE_20CM]  = barometer_open(num,para);
        break;
        case B8_SOIL_TEMPERATURE_30CM:
        num = get_driverNum(p_sensor[B8_SOIL_TEMPERATURE_30CM].type);
        para=  get_sensor_config(&p_sensor[B8_SOIL_TEMPERATURE_30CM]);
        sensor_driver[B8_SOIL_TEMPERATURE_30CM]  = barometer_open(num,para);
        break;
        case B9_SOIL_TEMPERATURE_50CM:
        num = get_driverNum(p_sensor[B9_SOIL_TEMPERATURE_50CM].type);
        para=  get_sensor_config(&p_sensor[B9_SOIL_TEMPERATURE_50CM]);
        sensor_driver[B9_SOIL_TEMPERATURE_50CM]  = barometer_open(num,para);
        break;
        case B10_SOIL_TEMPERATURE_100CM:
        num = get_driverNum(p_sensor[B10_SOIL_TEMPERATURE_100CM].type);
        para=  get_sensor_config(&p_sensor[B10_SOIL_TEMPERATURE_100CM]);
        sensor_driver[B10_SOIL_TEMPERATURE_100CM]  = barometer_open(num,para);
        break;
        case B11_SOIL_TEMPERATURE_150CM:
        num = get_driverNum(p_sensor[B11_SOIL_TEMPERATURE_150CM].type);
        para=  get_sensor_config(&p_sensor[B11_SOIL_TEMPERATURE_150CM]);
        sensor_driver[B11_SOIL_TEMPERATURE_150CM]  = barometer_open(num,para);
        break;
        case B12_SOIL_TEMPERATURE_300CM:
        num = get_driverNum(p_sensor[B12_SOIL_TEMPERATURE_300CM].type);
        para=  get_sensor_config(&p_sensor[B12_SOIL_TEMPERATURE_300CM]);
        sensor_driver[B12_SOIL_TEMPERATURE_300CM]  = barometer_open(num,para);
        break;
        case B13_SOIL_TEMPERATURE_500CM:
        num = get_driverNum(p_sensor[B13_SOIL_TEMPERATURE_500CM].type);
        para=  get_sensor_config(&p_sensor[B13_SOIL_TEMPERATURE_500CM]);
        sensor_driver[B13_SOIL_TEMPERATURE_500CM]  = barometer_open(num,para);
        break;
        case B2_SUNSHINE_DURATION:
          num = get_driverNum(p_sensor[B2_SUNSHINE_DURATION].type);
          para=  get_sensor_config(&p_sensor[B2_SUNSHINE_DURATION]);
          sensor_driver[B2_SUNSHINE_DURATION]  = sunshine_open(num,para);
        break;
        case B1_SOLAR_RADIATION:
        num = get_driverNum(p_sensor[B1_SOLAR_RADIATION].type);
        para = get_sensor_config(&p_sensor[B1_SOLAR_RADIATION]);
        sensor_driver[B1_SOLAR_RADIATION]  = solarRadiation_open(num,para);
        break;
        case N10_AIR_TEMPERATURE_50CM:
          num = get_driverNum(p_sensor[N10_AIR_TEMPERATURE_50CM].type);
          sensor_driver[N10_AIR_TEMPERATURE_50CM] = temperature_open(num,0);
        break;
      }
    }

















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
#define AWS_CVT_G(x)           ((x+100)*10)


#define UNUSED_SENSOR_VAL -999 
/**
 * @brief 센서 데이터를 AWS 자료형으로 변환환
 */
void  cvt_sensorToAWS(sensor_data_t *p_data,kma_data_t *p_kma)
{
  bool status;
  sensor_t *p_sensor = g_sensor_copy;

  // 1. 기온 (1분 평균)  표현 범위 500~1500 [(관측값 + 100)*100]
  p_kma->temperature = p_sensor[A1_TEMPERATURE].type?
  (int16_t)AWS_CVT_TEMP(p_data[A1_TEMPERATURE].data.f):UNUSED_SENSOR_VAL;

 // 2. 풍향 (1분 평균) 표현범위 → 1 ～ 3599 (관측값 × 10)
  p_kma->wind_direction_avg = p_sensor[A2_WIND_DIRECTION].type?
  (int16_t)AWS_CVT_WIND_DIRECTION(p_data[A2_WIND_DIRECTION].data.f):UNUSED_SENSOR_VAL;

 // 3. 풍속 (1분 평균) 표현범위 → 1 ～ 1000 (관측값 × 10)
 p_kma->wind_speed_avg = p_sensor[A3_WIND_SPEED].type?
 (int16_t)AWS_CVT_WIND_SPEED(p_data[A3_WIND_SPEED].data.f):UNUSED_SENSOR_VAL;

 // 4. 풍향 (1분 순간) 표현범위 → 0 ～ 3599 (관측값 × 10)
 p_kma->wind_direction_instant= p_sensor[A4_INSTANT_WIND_DIRECTION].type?
               (int16_t)AWS_CVT_WIND_DIRECTION(p_data[A4_INSTANT_WIND_DIRECTION].data.f):UNUSED_SENSOR_VAL;

 // 5. 풍속 (1분 순간) 표현범위 → 0 ～ 1000 (관측값 × 10
 p_kma->wind_speed_instant= p_sensor[A5_INSTANT_WIND_SPEED].type?
                        (int16_t)AWS_CVT_WIND_SPEED(p_data[A5_INSTANT_WIND_SPEED].data.f):UNUSED_SENSOR_VAL;

 // 6. 강수량 (0.5/1.0 mm) 표현범위 → 0 ～ 32767 (관측값 × 10)
 p_kma->precipitation= p_sensor[A6_RAINFALL_DOT5_1MM].type?
                  (int16_t)(p_data[A6_RAINFALL_DOT5_1MM].data.i):UNUSED_SENSOR_VAL;

 // 7. 기압 (1분 평균 현지 기압) 표현범위 → 5000 ～ 11000 (관측값 × 10
 p_kma->pressure= p_sensor[A6_RAINFALL_DOT5_1MM].type?
                    (int16_t)AWS_CVT_BAROMETER(p_data[A7_PRESSURE].data.f):UNUSED_SENSOR_VAL;

  if(p_sensor[A8_RAIN_PRESENT].type)
  {
    // 8. 강수 유무
    if(p_data[A8_RAIN_PRESENT].data.b == true)
    {
      p_kma->precipitation_presence = 10;
    }
    else
    {
      p_kma->precipitation_presence = 0;
    }
  }
  else
  {
    p_kma->precipitation_presence = UNUSED_SENSOR_VAL;
  }

 // 9. 적설  표현범위: 0 ~ 4095 (관측값 * 10)
 p_kma->snowfall= p_sensor[A9_SNOW_DEPTH].type?
                (int16_t)AWS_CVT_DEFAULT(p_data[A9_SNOW_DEPTH].data.f):UNUSED_SENSOR_VAL;

 // 10. 상대습도 (1분 평균) 표현범위: 0 ~ 1000 (관측값 * 10)
 p_kma->relative_humidity= p_sensor[A10_RELATIVE_HUMIDITY].type?
                    (int16_t)(AWS_CVT_HUMI(p_data[A10_RELATIVE_HUMIDITY].data.f)):UNUSED_SENSOR_VAL;

 // 11. 강수량 (0.1 mm) 표현범위 → 0 ～ 32767 (관측값 × 10)
 p_kma->precipitation_fine= p_sensor[A11_RAINFALL_DOT1MM].type?
                      (int16_t)(AWS_CVT_DEFAULT(p_data[A11_RAINFALL_DOT1MM].data.f)):UNUSED_SENSOR_VAL;

 // 1. 일사 (누적값)표현범위: 0 ~ 32767 [관측값(MJ/m²) * 100]
 p_kma->solar_radiation= p_sensor[B1_SOLAR_RADIATION].type?
                       (int16_t)(p_data[B1_SOLAR_RADIATION].data.f*100):UNUSED_SENSOR_VAL;

 // 2. 일조 (누적 시간)표현범위: 0 ~ 65535 [누적시간(초 단위)]
 p_kma->sunshine_duration= p_sensor[B2_SUNSHINE_DURATION].type?
 (int16_t)(p_data[B2_SUNSHINE_DURATION].data.f):UNUSED_SENSOR_VAL;

 // 3. 지면온도 (1분 평균)표현범위: 500 ~ 2000 [(관측값 + 100) * 10]
 p_kma->surface_temperature= p_sensor[B3_GROUND_TEMPERATURE].type?
 (int16_t)AWS_CVT_G(p_data[B3_GROUND_TEMPERATURE].data.f):UNUSED_SENSOR_VAL;

 // 4. 초상온도 (1분 평균)표현범위: 500 ~ 2000 [(관측값 + 100) * 10]
 p_kma->grass_temperature= p_sensor[B4_SURFACE_TEMPERATURE].type?
 (int16_t)AWS_CVT_G(p_data[B4_SURFACE_TEMPERATURE].data.f*1000):UNUSED_SENSOR_VAL; 

 // 5. 지중온도 (5cm, 1분 평균)
 p_kma->soil_temperature_5cm= p_sensor[B5_SOIL_TEMPERATURE_5CM].type?
 (int16_t)AWS_CVT_G(p_data[B5_SOIL_TEMPERATURE_5CM].data.f):UNUSED_SENSOR_VAL;

 // 6. 지중온도 (10cm, 1분 평균)
 p_kma->soil_temperature_10cm= p_sensor[B6_SOIL_TEMPERATURE_10CM].type?
 (int16_t)AWS_CVT_G(p_data[B6_SOIL_TEMPERATURE_10CM].data.f):UNUSED_SENSOR_VAL;

 // 7. 지중온도 (20cm, 1분 평균)
 p_kma->soil_temperature_20cm= p_sensor[B7_SOIL_TEMPERATURE_20CM].type?
 (int16_t)AWS_CVT_G(p_data[B7_SOIL_TEMPERATURE_20CM].data.f):UNUSED_SENSOR_VAL;

 // 8. 지중온도 (30cm, 1분 평균)
 p_kma->soil_temperature_30cm= p_sensor[B8_SOIL_TEMPERATURE_30CM].type?
 (int16_t)AWS_CVT_G(p_data[B8_SOIL_TEMPERATURE_30CM].data.f):UNUSED_SENSOR_VAL;

 // 9. 지중온도 (50cm, 1분 평균)
 p_kma->soil_temperature_50cm= p_sensor[B9_SOIL_TEMPERATURE_50CM].type?
 (int16_t)AWS_CVT_G(p_data[B9_SOIL_TEMPERATURE_50CM].data.f):UNUSED_SENSOR_VAL;

 // 10. 지중온도 (1.0m, 1분 평균)
 p_kma->soil_temperature_1m= p_sensor[B10_SOIL_TEMPERATURE_100CM].type?
 (int16_t)AWS_CVT_G(p_data[B10_SOIL_TEMPERATURE_100CM].data.f):UNUSED_SENSOR_VAL;

 // 11. 지중온도 (1.5m, 1분 평균)
 p_kma->soil_temperature_1_5m= p_sensor[B11_SOIL_TEMPERATURE_150CM].type?
 (int16_t)AWS_CVT_G(p_data[B11_SOIL_TEMPERATURE_150CM].data.f):UNUSED_SENSOR_VAL;


 // 12. 지중온도 (3.0m, 1분 평균)
 p_kma->soil_temperature_3m= p_sensor[B12_SOIL_TEMPERATURE_300CM].type?
 (int16_t)AWS_CVT_G(p_data[B12_SOIL_TEMPERATURE_300CM].data.f):UNUSED_SENSOR_VAL;

 // 13. 지중온도 (5.0m, 1분 평균)
 p_kma->soil_temperature_5m= p_sensor[B13_SOIL_TEMPERATURE_500CM].type?
 (int16_t)AWS_CVT_G(p_data[B13_SOIL_TEMPERATURE_500CM].data.f):UNUSED_SENSOR_VAL;


 // 1. 1층 운고 (1분 평균)표현범위: 0 ~ 8000 (관측값[m])
 p_kma->cloud_height_1st= p_sensor[C1_CLOUD_BASE1].type?
 (int16_t)(p_data[C1_CLOUD_BASE1].data.i):UNUSED_SENSOR_VAL;

 // 2. 2층 운고 (1분 평균)표현범위: 0 ~ 8000 (관측값[m])
 p_kma->cloud_height_2nd= p_sensor[C2_CLOUD_BASE2].type?
 (int16_t)(p_data[C2_CLOUD_BASE2].data.i):UNUSED_SENSOR_VAL;

 // 3. 3층 운고 (1분 평균)표현범위: 0 ~ 8000 (관측값[m])
 p_kma->cloud_height_3rd= p_sensor[C3_CLOUD_BASE3].type?
 (int16_t)(p_data[C3_CLOUD_BASE3].data.i):UNUSED_SENSOR_VAL;

 // 4. 운량 표현범위: 0 ~ 10 (관측값)
 p_kma->cloud_amount= p_sensor[C4_CLOUD_COVER].type?
 (int16_t)(p_data[C4_CLOUD_COVER].data.i):UNUSED_SENSOR_VAL;

 // 5. 시정 (1분 평균)표현범위: 0 ~ 50000 (관측값[m])
 p_kma->visibility= p_sensor[C5_VISIBILITY].type?
 (int16_t)(p_data[C5_VISIBILITY].data.i):UNUSED_SENSOR_VAL;

 // 6. PM10 (분진농도)표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)
 p_kma->pm10_concentration= p_sensor[C6_PM10].type?
 (int16_t)(p_data[C6_PM10].data.f*10):UNUSED_SENSOR_VAL;


 // 7. PM2.5 (분진농도)표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)
 p_kma->pm25_concentration= p_sensor[C7_PM2DOT5].type?
 (int16_t)(p_data[C7_PM2DOT5].data.f*1000):UNUSED_SENSOR_VAL;

 // 8. 순복사 (1분 평균)
 p_kma->net_radiation= p_sensor[C8_NET_RADIATION].type?
 (int16_t)(p_data[C8_NET_RADIATION].data.f*1000):UNUSED_SENSOR_VAL;//  표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 9. 전천복사 (1분 평균)
 p_kma->total_radiation=p_sensor[C9_TOTAL_RADIATION].type?
 (int16_t)( p_data[C9_TOTAL_RADIATION].data.f*1000):UNUSED_SENSOR_VAL;//  표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 10. 반사복사 (1분 평균)
 p_kma->reflected_radiation= p_sensor[C10_REFLECTED_RADIATION].type?
 (int16_t)(p_data[C10_REFLECTED_RADIATION].data.f*1000):UNUSED_SENSOR_VAL;// 사 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 11. 직달복사 (1분 평균)
 p_kma->direct_radiation= p_sensor[C11_DIRECT_SOLAR].type?
 (int16_t)(p_data[C11_DIRECT_SOLAR].data.f*1000):UNUSED_SENSOR_VAL;//  표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

 // 12. 현재 일기
 p_kma->current_weather= p_sensor[C12_CURRENT_WEATHER].type?
 (int16_t)(p_data[C12_CURRENT_WEATHER].data.f*1000):UNUSED_SENSOR_VAL;//  표현범위: 0 ~ 99 (관측값)



 // 1. 토양수분 (10 cm)
 p_kma->soil_moisture_10cm=p_sensor[N1_SOIL_MOISTURE_10CM].type?
 (int16_t) (p_data[N1_SOIL_MOISTURE_10CM].data.f*1000):UNUSED_SENSOR_VAL;//  표현범위: 0 ~ 1000 (관측값 * 10)

 // 2. 토양수분 (20 cm)
 p_kma->soil_moisture_20cm=p_sensor[N2_SOIL_MOISTURE_20CM].type?
  (int16_t)(p_data[N2_SOIL_MOISTURE_20CM].data.f*1000):UNUSED_SENSOR_VAL;// 표현범위: 0 ~ 1000 (관측값 * 10)

 // 3. 토양수분 (30 cm)
 p_kma->soil_moisture_30cm=p_sensor[N3_SOIL_MOISTURE_30CM].type?
 (int16_t)(p_data[N3_SOIL_MOISTURE_30CM].data.f*1000):UNUSED_SENSOR_VAL;//  표현범위: 0 ~ 1000 (관측값 * 10)

 // 4. 토양수분 (50 cm)
 p_kma->soil_moisture_50cm= p_sensor[N4_SOIL_MOISTURE_50CM].type?
 (int16_t)(p_data[N4_SOIL_MOISTURE_50CM].data.f*1000):UNUSED_SENSOR_VAL;// 표현범위: 0 ~ 1000 (관측값 * 10)

 // 5. 조도량 (1분 평균)
 p_kma->illuminance= p_sensor[N5_ILLUMINANCE].type?
 (int16_t)(p_data[N5_ILLUMINANCE].data.f*1000):UNUSED_SENSOR_VAL;// 표현범위: 0 ~ 32767 (관측값 * 100)

 // 6. 풍속 (1.5 m, 1분 평균)
 p_kma->wind_speed_1_5m= p_sensor[N6_WIND_VELOCITY_150CM].type?
 (int16_t)(p_data[N6_WIND_VELOCITY_150CM].data.f*1000):UNUSED_SENSOR_VAL;//  표현범위: 0 ~ 1000 (관측값 * 10)

 // 7. 풍속 (4.0 m, 1분 평균)
 p_kma->wind_speed_4m=p_sensor[N7_WIND_VELOCITY_400CM].type?
 (int16_t) (p_data[N7_WIND_VELOCITY_400CM].data.f*1000):UNUSED_SENSOR_VAL;// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 8. 순간 풍속 (1.5 m)
 p_kma->instant_wind_speed_1_5m= p_sensor[N8_INSTANT_VELOCITY_150CM].type?
 (int16_t)(p_data[N8_INSTANT_VELOCITY_150CM].data.f*1000):UNUSED_SENSOR_VAL;// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 9. 순간 풍속 (4.0 m)
 p_kma->instant_wind_speed_4m= p_sensor[N9_INSTANT_VELOCITY_400CM].type?
 (int16_t)(p_data[N9_INSTANT_VELOCITY_400CM].data.f*1000):UNUSED_SENSOR_VAL;// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

 // 10. 기온 (0.5 m)
 p_kma->temperature_0_5m= p_sensor[N10_AIR_TEMPERATURE_50CM].type?
 (int16_t)(p_data[N10_AIR_TEMPERATURE_50CM].data.f*1000):UNUSED_SENSOR_VAL;// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

 // 11. 기온 (4.0 m)
 p_kma->temperature_4m= p_sensor[N11_AIR_TEMPERATURE_400CM].type?
 (int16_t)(p_data[N11_AIR_TEMPERATURE_400CM].data.f*1000):UNUSED_SENSOR_VAL;// 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

 // 12. 습도 (0.5 m, 1분 평균)// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)
 p_kma->humidity_0_5m= p_sensor[N12_HUMIDITY_50CM].type?
                      (int16_t)(p_data[N12_HUMIDITY_50CM].data.f*10):UNUSED_SENSOR_VAL;

 // 13. 습도 (4.0 m, 1분 평균) // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)
 p_kma->humidity_4m = p_sensor[N13_HUMIDITY_400CM].type?
                      (int16_t)(p_data[N13_HUMIDITY_400CM].data.f*10):UNUSED_SENSOR_VAL;
                     
 p_kma->tacometer   = p_sensor[I1_TACHOMETER].type?
                      (int16_t)(p_data[I1_TACHOMETER].data.f*1000):UNUSED_SENSOR_VAL;

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

/**
 * @brief 250ms마다 측정해야 하는 센서들 처리,현재는 풍향,풍속 뿐이다.
 */
void measure_250ms(DATE_TIME_BUF *ct)
{
  uint8_t err;
  uint16_t i;
  float data;
  sensor_t *p_sensor = g_sensor_copy;

  for( i = 0 ;i < _countof(g_sensor_copy); i++)
  {
    if(p_sensor[i].type)
    {
      switch(i)
      {
        case A2_WIND_DIRECTION:
        data =  wind_read(sensor_driver[A2_WIND_DIRECTION],WIND_CHANNEL_DIRECTION,&err);
        if(err==0)
        {

        }
        sensor_data_1s[A2_WIND_DIRECTION].data.f = data;
        break;
        case A3_WIND_SPEED:
        data =  wind_read(sensor_driver[A3_WIND_SPEED],WIND_CHANNEL_SPEED,&err);
        sensor_data_1s[A3_WIND_SPEED].data.f = data;
        break;
      }
    }
  }
}



void measure_1s(DATE_TIME_BUF *ct,sensor_t *sensor,uint16_t sensor_cnt)
{
  int32_t iData;
  float fData;
  int32_t rain_pulse;
  uint8_t err;
  uint8_t sample_cnt=0;
  uint16_t i;
  uint16_t data;
  bool bVal;
  float avg;
  float adc;
  float max;
  float min;
  int32_t ch;
  eSENSOR_MODEL_t type;



  for( i = 0 ;i < sensor_cnt;i++)
  {
      type = sensor[i].type; 
      if(type)
      {
        adc = 0;
        switch(i)
        {
          case A1_TEMPERATURE:
          adc =  temperature_read(sensor_driver[A1_TEMPERATURE],&err);
          sensor_data_1s[A1_TEMPERATURE].data.f = adc;
          if((ct->Sec % 10)==0)//10초마다 샘플링하고 6개 자료의 평균을 1분자료
          {
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
            rain_pulse = read_sensor_rain(sensor_driver[A6_RAINFALL_DOT5_1MM],&err);
            sensor_data_1s[A6_RAINFALL_DOT5_1MM].data.i += rain_pulse;
            sensor_data[A6_RAINFALL_DOT5_1MM].data.i += rain_pulse;//금일 우량
            sensor_data[A6_RAINFALL_DOT5_1MM].status = read_rainHallErr();
          }
            
          break;
          case A7_PRESSURE://10초마다 샘플링하고 6개 자료의 평균을 1분자료
          adc =  read_sensor_barometer(sensor_driver[A7_PRESSURE],&err);
          sensor_data_1s[A7_PRESSURE].data.f = adc;
          if((ct->Sec%10)==0)
            {
              sample_barometer(adc);
              sensor_data[A7_PRESSURE].data.f = get_avg_barometer();
            }
          break;
          case A8_RAIN_PRESENT:
            bVal = read_sensor_rainPresent(sensor_driver[A8_RAIN_PRESENT],&err);
            sensor_data[A8_RAIN_PRESENT].data.b = bVal;
            sensor_data_1s[A8_RAIN_PRESENT].data.b = bVal;
          break;
          case A9_SNOW_DEPTH: //mm
          iData = read_sensor_snow(sensor_driver[A9_SNOW_DEPTH],&err);
          sensor_data[A9_SNOW_DEPTH].data.i = iData;
          sensor_data_1s[A9_SNOW_DEPTH].data.i = iData;
          break;
          case A10_RELATIVE_HUMIDITY://10초마다 샘플링하고 6개 자료의 평균을 1분자료
          adc =  read_sensor_humidity(sensor_driver[A10_RELATIVE_HUMIDITY],&err);
          sensor_data_1s[A10_RELATIVE_HUMIDITY].data.f = adc;
          if((ct->Sec%10)==0)
            {
              sample_humi(adc);
              sensor_data[A10_RELATIVE_HUMIDITY].data.f = get_avg_humi();
            }
          break;
          case B1_SOLAR_RADIATION:
          fData = read_sensor_solarRadiation(sensor_driver[B1_SOLAR_RADIATION],&err);
          sensor_data[B1_SOLAR_RADIATION].data.f    = fData;
          sensor_data_1s[B1_SOLAR_RADIATION].data.f = fData;
          break;
          case B2_SUNSHINE_DURATION:
          fData = read_sensor_sunshine(sensor_driver[B2_SUNSHINE_DURATION],&err);
          sensor_data[B2_SUNSHINE_DURATION].data.f    = fData;
          sensor_data_1s[B2_SUNSHINE_DURATION].data.f = fData;
          break;
          case B5_SOIL_TEMPERATURE_5CM:
          fData = read_sensor_soilTemp(sensor_driver[B5_SOIL_TEMPERATURE_5CM],&err);
          sensor_data[B5_SOIL_TEMPERATURE_5CM].data.f = fData;
          sensor_data_1s[B5_SOIL_TEMPERATURE_5CM].data.f = fData;
          break;
          case B6_SOIL_TEMPERATURE_10CM:
          fData = read_sensor_soilTemp(sensor_driver[B6_SOIL_TEMPERATURE_10CM],&err);
          sensor_data[B6_SOIL_TEMPERATURE_10CM].data.f = fData;
          sensor_data_1s[B6_SOIL_TEMPERATURE_10CM].data.f = fData;
          break;
          case B7_SOIL_TEMPERATURE_20CM:
          fData = read_sensor_soilTemp(sensor_driver[B7_SOIL_TEMPERATURE_20CM],&err);
          sensor_data[B7_SOIL_TEMPERATURE_20CM].data.f = fData;
          sensor_data_1s[B7_SOIL_TEMPERATURE_20CM].data.f = fData;
          break;
          case B8_SOIL_TEMPERATURE_30CM:
          fData = read_sensor_soilTemp(sensor_driver[B8_SOIL_TEMPERATURE_30CM],&err);
          sensor_data[B8_SOIL_TEMPERATURE_30CM].data.f = fData;
          sensor_data_1s[B8_SOIL_TEMPERATURE_30CM].data.f = fData;

          break;
          case B9_SOIL_TEMPERATURE_50CM:
          fData = read_sensor_soilTemp(sensor_driver[B9_SOIL_TEMPERATURE_50CM],&err);
          sensor_data[B9_SOIL_TEMPERATURE_50CM].data.f = fData;
          sensor_data_1s[B9_SOIL_TEMPERATURE_50CM].data.f = fData;

          break;
          case B10_SOIL_TEMPERATURE_100CM:
          fData = read_sensor_soilTemp(sensor_driver[B10_SOIL_TEMPERATURE_100CM],&err);
          sensor_data[B10_SOIL_TEMPERATURE_100CM].data.f = fData;
          sensor_data_1s[B10_SOIL_TEMPERATURE_100CM].data.f = fData;
          break;
          case B11_SOIL_TEMPERATURE_150CM:
          fData = read_sensor_soilTemp(sensor_driver[B11_SOIL_TEMPERATURE_150CM],&err);
          sensor_data[B11_SOIL_TEMPERATURE_150CM].data.f = fData;
          sensor_data_1s[B11_SOIL_TEMPERATURE_150CM].data.f = fData;
          break;

          case B12_SOIL_TEMPERATURE_300CM:
          fData = read_sensor_soilTemp(sensor_driver[B12_SOIL_TEMPERATURE_300CM],&err);
          sensor_data[B12_SOIL_TEMPERATURE_300CM].data.f = fData;
          sensor_data_1s[B12_SOIL_TEMPERATURE_300CM].data.f = fData;
          break;

          case B13_SOIL_TEMPERATURE_500CM:
          fData = read_sensor_soilTemp(sensor_driver[B13_SOIL_TEMPERATURE_500CM],&err);
          sensor_data[B13_SOIL_TEMPERATURE_500CM].data.f = fData;
          sensor_data_1s[B13_SOIL_TEMPERATURE_500CM].data.f = fData;
          break;


          case N10_AIR_TEMPERATURE_50CM:
          adc =  temperature_read(sensor_driver[N10_AIR_TEMPERATURE_50CM],&err);//더빠른값 변화위해 수집은 1초ㄴ
          sensor_data_1s[N10_AIR_TEMPERATURE_50CM].data.f = adc;

          break;
          default:
         // fData =  read_sensorGeneral(&sensor[i],&err);
          sensor_data[i].data.f = fData;
          sensor_data_1s[i].data.f = fData;
          break;
        }
    }
  }



}

/**
 * @brief 수집된 센서 데이터를 AWS자료형으로 변환하고 저장
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
    g_start_time = mcu_get_clk();

    ct = Date_Time;

    measure_250ms(&ct);

    if(ct.Sec != ot.Sec)
    {
      measure_1s(&ct,g_sensor_copy,_countof(g_sensor_copy));
      cvt_sensorToAWS(sensor_data_1s,&g_kma_1s);
      
      if(ct.Min != ot.Min)
      {
        min_process(&ct);
        ot.Min = ct.Min;
      }
      ot.Sec = ct.Sec;
    }
    g_elased_time = cal_elapsed_us(g_start_time);
    if(g_elased_time>g_elased_max)
    {
      g_elased_max = g_elased_time;
    }
    tick_count += MEASURE_PERIOD_MS;
    
    osDelayUntil(tick_count);//남은 지연 시간만큼 지연

  }
}

void measureTask_init(void)
{
    osThreadId_t  threadId;

    threadId = osThreadNew(measureTask, NULL, &kMeasureTask_attributes);

    assert_param(threadId);
}