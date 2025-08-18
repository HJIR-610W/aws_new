#include "task_aws.h"

#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "app_sensor.h"
#include "aws_data.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "config_nvm.h"

#include "old_aws_define.h"
#include "schedule.h"
#include "task_measure.h"
#include "user_heap.h"
#include "util_filter.h"
#include "util_time.h"
#include "kma3.h"
#include "dev_io.h"
#include "logging\utile_data.h"
#include "util_memory.h"
#include "sensor_data\rain_data.h"
#include "sensor_data\sunshine_data.h"
#include "wind_data.h"
#include "aws_default_data.h"
#include "app_key.h"
typedef struct filter_data_s
{
  uint8_t delay_count;
  uint16_t data;
  uint8_t err;
}filter_data_t;


filter_data_t g_pre_data[SENSOR_LIST_MAX];

 const float kSunshine_threshold = 0.8f;

#define MAKE_TEMP(x) (uint16_t)((x + 100) * 10)  // 기온, 지면온도, 지중온도, 초상온도
#define MAKE_RADI(x) (uint16_t)((x + 100) * 10)  // 순복사, 전천복사, 반사복사 등
#define MAKE_PRESSURE(x) (uint16_t)((x) * 10)    // 기압
#define MAKE_X10(x) (uint16_t)((x) * 10)         // 풍속, 풍향, 습도, 토양수분 등
#define MAKE_X100(x) (uint16_t)((x) * 100)       // 일사량, 조도량 등
#define MAKE_DIRECT(x) (uint16_t)(x)             // 운고, 시정, 현재일기, 타코미터 등 정수값



#define AWS_DATA_ERR_VAL 9999

measure_data_1s_t *g_p_raw = NULL;
uint8_t g_kma_err[SENSOR_LIST_MAX];
measure_data_250ms_t g_raw_250;


void update_sensor_err(eSENSOR_TYPE_t sensor, uint8_t code)
{
  g_kma_err[sensor] = code;

  kma_update_sensor_err(sensor,code);
}

uint8_t get_sensor_err(eSENSOR_TYPE_t sensor)
{
  return   g_kma_err[sensor];
}

bool is_raining(uint8_t  *sensor_err)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  *sensor_err = p_sensor[A8_RAIN_PRESENT].err;
  
  return p_sensor[A8_RAIN_PRESENT].data.b;
}



//자기 유도식 기준
#define WIND_SPEED_ACCURACY_BELOW_10MPS 0.5f  // 0.5m/s  10m/s 미만
#define WIND_SPEED_ACCURACY_ABOVE_10MPS 0.5f  // 5%      10m/s 이상

#define WIND_SPEED_ACCURACY_LT_10MPS 0.5f  // 0.5m/s  10m/s 미만
#define WIND_SPEED_ACCURACY_GE_10MPS 0.05f  // 5%      10m/s 이상

uint16_t WindSpeedCalc(uint8_t *sensor_err)
{
  uint8_t err = 0;
  sensor_data_t *p_sensor = g_p_raw->data;
  float wind_speed;
  float accuracy;

  *sensor_err = 0;

  err = p_sensor[A3_WIND_SPEED].err;

  if(err)
  {
    *sensor_err = err;
    return AWS_DATA_ERR_VAL;
  }

  wind_speed = p_sensor[A3_WIND_SPEED].data.f;


  if (wind_speed < 10.0f)  // 10m/s 미만이면 0.5m/s 정확도 가져야함함
  {
    wind_speed = validate_sensor_value_min(wind_speed, 0.0f, WIND_SPEED_ACCURACY_LT_10MPS, &err);

    if (err)
    {
      *sensor_err = 1<<4;//값에러는 상위 니블로 표현
      return AWS_DATA_ERR_VAL;
    }

  }
  else //10m/s 이상이면  측정값의 5%
  {
    accuracy = wind_speed * WIND_SPEED_ACCURACY_GE_10MPS;

    wind_speed = validate_sensor_value_max(wind_speed, 75.0f, accuracy, &err);

    if (err)
    {
      *sensor_err = 2 << 4;  // 값에러는 상위 니블로 표현
      return AWS_DATA_ERR_VAL;
    }
  }

  return (uint16_t)(truncate_to_1_decimal(wind_speed) * 10);
}

#define WIND_DIRECTION_ACCURACY 5.0f//5도
uint16_t  WindDirecCalc(uint8_t *sensor_err)
{
  uint8_t err=0;
  sensor_data_t *p_sensor = g_p_raw->data;
  float wind_direction;

  *sensor_err = 0;

  err = p_sensor[A2_WIND_DIRECTION].err;

  if(err)
  { 
    *sensor_err = err;
    return AWS_DATA_ERR_VAL;
  }

  
  wind_direction = p_sensor[A2_WIND_DIRECTION].data.f;

  wind_direction = validate_sensor_value_min(wind_direction, 0, WIND_DIRECTION_ACCURACY, &err);

  if(err)
  {
    *sensor_err = err<<4;
    return AWS_DATA_ERR_VAL;
  }

  wind_direction = validate_sensor_value_max(wind_direction, 359.99, WIND_DIRECTION_ACCURACY, &err);

  if (err)
  {
    *sensor_err = err<<4;
    return AWS_DATA_ERR_VAL;
  }

  return (uint16_t)(truncate_to_1_decimal(wind_direction) * 10);//이미 10배 된 값으로 처리 
}

#define TEMPERATURE_ACCURACY 0.3 //0.3도
uint16_t TempCalc(uint8_t *sensor_err)
 {
   uint8_t err = 0;
   float temperature;
   sensor_data_t *p_sensor = g_p_raw->data;

   *sensor_err = 0;
   err = p_sensor[A1_TEMPERATURE].err;
   
   if (err)
   {
    *sensor_err = err;
     return AWS_DATA_ERR_VAL;
   }

   temperature = p_sensor[A1_TEMPERATURE].data.f;

   temperature = validate_sensor_value_min(temperature, -40.0f, TEMPERATURE_ACCURACY, &err);

   if (err)
   {
     *sensor_err = err<<4;
     return AWS_DATA_ERR_VAL;
   }

   temperature = validate_sensor_value_max(temperature, 60.0f, TEMPERATURE_ACCURACY, &err);

   if (err)
   {
     *sensor_err = err<<4;
     return AWS_DATA_ERR_VAL;
   }


   /**
    * 온도를 소수점 1째자리만 사용함
    * 만약 -0.002도라면 (99.998*10  = 999 가 전송됨)
    * 999를 복구하면 -0.1도가 되버림 따라서 처음부터 소수점 1째리까지만 처리해야함
    */
   return (uint16_t)((truncate_to_1_decimal(temperature) + 100) * 10);  // AWS 데이터 형으로 변환 ((측정값+100) *10)
}

/*
AWS(구)
진양     :RS232
RM YOUNG :
*/
#define PRESSURE_ACCURACY 0.5f //0.5hPa
uint16_t  BarometricCalc(uint8_t *sensor_err)
{
  uint8_t err = 0;
  float pressure;
  sensor_data_t *p_sensor = g_p_raw->data;

  *sensor_err = 0;
  err = p_sensor[A7_PRESSURE].err;
  
  if (err)
  {
    *sensor_err = err;
    return AWS_DATA_ERR_VAL;
  }

  pressure = p_sensor[A7_PRESSURE].data.f;

  pressure = validate_sensor_value_min(pressure, 500.0f, PRESSURE_ACCURACY, &err);

  if (err)
  {
    *sensor_err = err<<4;
    return AWS_DATA_ERR_VAL;
  }

  pressure = validate_sensor_value_max(pressure, 1080.0f, PRESSURE_ACCURACY, &err);

  if (err)
  {
    *sensor_err = err<<4;
    return AWS_DATA_ERR_VAL;
  }

  return (uint16_t)(truncate_to_1_decimal(pressure) * 10);  // AWS 데이터 형으로 변환 측정값 *10
}


#define HUMINITY_0_90_ACCURACY 3.0f    // 3%
#define HUMINITY_91_100_ACCURACY 5.0f  // 5%
//3%로 일괄 적용
uint16_t HumidityCalc(uint8_t *sensor_err)
{
  uint8_t err=0;
  float huminity;
  sensor_data_t *p_sensor = g_p_raw->data;


  *sensor_err = 0;

  err = p_sensor[A10_RELATIVE_HUMIDITY].err;
   if (err)
  {
    *sensor_err = err;
     return AWS_DATA_ERR_VAL;
  }

  huminity = p_sensor[A10_RELATIVE_HUMIDITY].data.f;

  huminity = validate_sensor_value_min(huminity, 0.0f, HUMINITY_0_90_ACCURACY, &err);

  if(err)
  {
    *sensor_err = 1<<4;
    return AWS_DATA_ERR_VAL;
  }

  huminity = validate_sensor_value_max(huminity, 100.0f, HUMINITY_0_90_ACCURACY, &err);

  if(err)
  {
    *sensor_err = 2<<4;
    return AWS_DATA_ERR_VAL;
  }

  return (uint16_t)(truncate_to_1_decimal(huminity) * 10);  // AWS 데이터 형으로 변환 측정값 *10
}

/*
# 일사

### AWS(구)
 - return값 :: error인 경우 9999, 정상인 경우: 0 - 1000
 - 자료의 표현은 0 - 1600까지 W/m2
  일사값이 9999가 아니면 1초마다합산
  1분마다 합산 결과를 1000으로 나눔 KWJ
  전송시 10으로 나눈 값으로 전송
?문제:1w이하는 0으로 처리하는데 표쥰규격에 분해능이 ㅊ이어서 그런듯

예)합산값/1000000 = MJ/m²이며 여기에 자료처리 표준에 따라 100을 곱해서 전송해야함
    전송되는 최종값은 10000으로 나누는 것과 동일
    AWS(구)1분 합산값을 1000으로 나누어 KJ로 만들고 자료 전송시 10으로 한번더 나누어 전송한다.

?의문:1 WJ/m2 *60초 = 60WJ/m2이다. 10000으로 나누면 0.006이되고 전송되는 값은 0이 된다.


AWS 관측장비 표쥰규격
### 일사 센서: 열전대식
- **민감도(Sensitivity)**: 7 ~ 17 μV/(W/m²)
- **온도특성**: ±2% / -20 ~ +50℃
- **비선형성(Non-linearity)**: ±0.5%
- **안정도**: ±0.8%/년 (Change per year)
- **정확도**: 시간변화 3%, 일변화 2%
- **운용환경**: 기온 -40 ~ +60℃
- **분해능**: 1 W/m²


AWS 자료처리 표준규격
- 자료 단위: 0.01 MJ/m²
- 샘플링 시간: 1초
- 자료처리 시간간격: 1분
  - 1초 간격의 60개 자료를 누적하여 1분 자료를 산출한다.
AWS 자료전송 규격
- **1분 누적 일사량**
- **사용비트**: 14 ~ 0번 비트
- **유효범위**: 0 ~ 32767 (이진 코드)
- **표현범위**: 0 ~ 32767 {누적 값(MJ/m²) × 100}



*/
uint16_t  SolarRadCalc(uint8_t *sensor_err)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  *sensor_err = p_sensor[B1_SOLAR_RADIATION].err;
  if (*sensor_err )
  {
    return AWS_DATA_ERR_VAL;
  }

  return (uint16_t)(p_sensor[B1_SOLAR_RADIATION].data.f);
}


uint16_t SnowCalc(uint8_t *sensor_err)
{
  sensor_data_t *p_sensor = g_p_raw->data;
  int32_t snow;
  uint8_t err;

  *sensor_err = 0;
  err = p_sensor[A9_SNOW_DEPTH].err;

  if(err)
  {
    *sensor_err = err;
    return AWS_DATA_ERR_VAL;
  }

  snow = p_sensor[A9_SNOW_DEPTH].data.i;

  if(snow<0)
  {
    snow=0;
  }

  return (uint16_t)(snow);
}

uint16_t  TempCalcExt(uint8_t ch,uint8_t *sensor_err)
{

  sensor_data_t *p_sensor = g_p_raw->data;
  float temperature;
  uint8_t err=0;

  *sensor_err = 0;

  switch (ch)
  {
    case SOLITEMP5CM_CHN:
      err = p_sensor[B5_SOIL_TEMPERATURE_5CM].err; 
      if (err)
      {
        *sensor_err = err;
        return AWS_DATA_ERR_VAL;
      }
      temperature = p_sensor[B5_SOIL_TEMPERATURE_5CM].data.f;

      break;
    case SOLITEMP10CM_CHN:
      err = p_sensor[B6_SOIL_TEMPERATURE_10CM].err;
      if(err)
      {
        *sensor_err = err;
        return AWS_DATA_ERR_VAL;
      }
      temperature = p_sensor[B6_SOIL_TEMPERATURE_10CM].data.f;
      break;
    case SOLITEMP20CM_CHN:
      err = p_sensor[B7_SOIL_TEMPERATURE_20CM].err;

      if(err)
      {
        *sensor_err = err;
        return AWS_DATA_ERR_VAL;
      }
      temperature = p_sensor[B7_SOIL_TEMPERATURE_20CM].data.f;
      break;
    case SOLITEMP30CM_CHN:
      err = p_sensor[B8_SOIL_TEMPERATURE_30CM].err;
      if(err)
      {
        *sensor_err = err;
        return AWS_DATA_ERR_VAL;
      }
      temperature = p_sensor[B8_SOIL_TEMPERATURE_30CM].data.f;
      break;
    case SOLITEMP50CM_CHN:
      err = p_sensor[B9_SOIL_TEMPERATURE_50CM].err;

      if(err)
      {
        *sensor_err = err;
        return AWS_DATA_ERR_VAL;
      }
      temperature = p_sensor[B9_SOIL_TEMPERATURE_50CM].data.f;
      break;
    case SOLITEMP1_0M_CHN:
      err = p_sensor[B10_SOIL_TEMPERATURE_100CM].err;

      if(err)
      {
        *sensor_err = err;
        return AWS_DATA_ERR_VAL;
      }
      temperature = p_sensor[B10_SOIL_TEMPERATURE_100CM].data.f;
      break;
    case SOLITEMP1_5M_CHN:

      err = p_sensor[B11_SOIL_TEMPERATURE_150CM].err;
      if(err)
      {
        *sensor_err = err;
        return AWS_DATA_ERR_VAL;
      }
      temperature = p_sensor[B11_SOIL_TEMPERATURE_150CM].data.f;
      break;
  }




  temperature = validate_sensor_value_min(temperature, -40.0f, TEMPERATURE_ACCURACY, &err);

  if (err)
  {
    *sensor_err = err << 4;
    return AWS_DATA_ERR_VAL;
  }

  temperature = validate_sensor_value_max(temperature, 60.0f, TEMPERATURE_ACCURACY, &err);

  if (err)
  {
    *sensor_err = err << 4;
    return AWS_DATA_ERR_VAL;
  }

  return (uint16_t)((truncate_to_1_decimal(temperature) + 100) *
                    10);  // AWS 데이터 형으로 변환 ((측정값+100) *10)
}



bool is_over_threshold(float sample, float threshold, float epsilon)
{
  float round_smaple ;

  if (!isnormal(sample))
  {
		return false;
	}

	round_smaple = round_to(sample, 2);


	if (fabsf(round_smaple - threshold) <= epsilon)
    return false;

  return round_smaple > threshold;

	

  }

  // 일조
uint8_t SunshineCalc(uint8_t *err)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  *err = p_sensor[B2_SUNSHINE_DURATION].err;

  if (is_over_threshold(p_sensor[B2_SUNSHINE_DURATION].data.f, kSunshine_threshold,0.01))
  {
      return 1;
  }

  return 0;
}

void dualport_init(void)
{



}

/**
 * @brief mm 우량 
 */
uint16_t get_rain_mm(uint8_t *sensor_err)
{

  uint16_t rain = 0;
  sensor_data_t *p_sensor = g_p_raw->data;

  //우량은 홀센서인 경우에만 에러 체크됨
  *sensor_err = (uint16_t)p_sensor[A6_RAINFALL_DOT5_1MM].err;
  
  rain = MAKE_X10(p_sensor[A6_RAINFALL_DOT5_1MM].data.f);

  p_sensor[A6_RAINFALL_DOT5_1MM].data.f = 0;// 우량은 이전값을 초기화해줘야함

  return rain;
}



AWS_DATA_STRUCT *get_aws_data(int min)
{
  AWS_DATA_STRUCT *p_aws_data=NULL;
  switch(min)
  {
    case eAWS_DATA_REAL:
      p_aws_data = &mRealAws;
      break;
    case eAWS_DATA_1MIN:
      p_aws_data = &mMinAws;
      break;
    case eAWS_DATA_10MIN:
      p_aws_data = &m10MinAws;
      break;
    case eAWS_DATA_HOUR:
      p_aws_data = &mHourAws;
      break;
    default:
      break;
  }

  return p_aws_data;
}





/*
250ms마다 업데이트되는 실시간값을 업데이트
기존 AWS(구)자료형을 AWS(신)자료형으로 변환
자료값은 AWS 자료형으로 저장 
*/
void update_kma_real(void)
{
  kma_data_ex_t *p_kma3;
  kma_data_ex_t *p_raw;

  p_kma3 = get_kma_data(eAWS_DATA_REAL);
  p_raw = get_kma_data(eAWS_DATA_RAW);

  //[사용]
  p_kma3->temperature.data = mRealAws.mTemperature.sReal;
  p_kma3->temperature.err = get_sensor_err(A1_TEMPERATURE);
  p_kma3->temperature.max = mRealAws.mTemperature.sMax;//일간 Max
  p_kma3->temperature.min = mRealAws.mTemperature.sMin;//일갈 Min

  //[사용]
  p_kma3->wind_direction_avg.data = mRealAws.mWind.mDirection.sReal;
  p_kma3->wind_direction_avg.err = get_sensor_err(A2_WIND_DIRECTION);
  p_kma3->wind_direction_avg.max = mRealAws.mWind.mDirection.sMax;
  //[사용]
  p_kma3->wind_speed_avg.data = mRealAws.mWind.mSpeed.sReal;
  p_kma3->wind_speed_avg.err = get_sensor_err(A3_WIND_SPEED);
  p_kma3->wind_speed_avg.max = mRealAws.mWind.mSpeed.sMax;

  p_kma3->wind_speed_instant.data = mRealAws.mWind.mSpeed.sMax;
  p_kma3->wind_speed_instant.err = 0;

  p_kma3->wind_direction_instant.data = mRealAws.mWind.mDirection.sMax;
  p_kma3->wind_direction_instant.err = 0;

  //[사용]
  p_kma3->precipitation.data = mRealAws.mRainFall.sReal;
  p_kma3->precipitation.err = get_sensor_err(A6_RAINFALL_DOT5_1MM);

  //[사용]
  p_kma3->pressure.data = mRealAws.mBarometric.sReal;
  p_kma3->pressure.err = get_sensor_err(A7_PRESSURE);
  p_kma3->pressure.max = mRealAws.mBarometric.sMax;
  p_kma3->pressure.min = mRealAws.mBarometric.sMin;
  //[사용]
  p_kma3->precipitation_presence.data = mRealAws.mRainDetect.sReal;
  p_kma3->precipitation_presence.err = get_sensor_err(A8_RAIN_PRESENT);
  //[사용]
  p_kma3->snowfall.data = mRealAws.mSnowFall.sReal;
  p_kma3->snowfall.err = get_sensor_err(A9_SNOW_DEPTH);
  //[사용]
  p_kma3->relative_humidity.data = mRealAws.mHumidity.sReal;
  p_kma3->relative_humidity.err = get_sensor_err(A10_RELATIVE_HUMIDITY);
  p_kma3->relative_humidity.max = mRealAws.mHumidity.sMax;
  p_kma3->relative_humidity.min = mRealAws.mHumidity.sMin;

  //[미사용] 강수량 0.1 (원본값으로 표현)


  //[사용]
  p_kma3->solar_radiation.data    = (uint16_t)(g_solar_radiation.min_acc/1000.0);

  p_kma3->solar_radiation.day_accu = mRealAws.mSolarRad.sMax;//일간
  p_kma3->solar_radiation.err = get_sensor_err(B1_SOLAR_RADIATION);
  //[사용]
  p_kma3->sunshine_duration.data = g_sunshine.today;
  p_kma3->sunshine_duration.err = get_sensor_err(B2_SUNSHINE_DURATION);


  //[미사용] 3. 지면온도 (1분 평균)
  p_kma3->surface_temperature.data = p_raw->surface_temperature.data;
  p_kma3->surface_temperature.err = get_sensor_err(B3_GROUND_TEMPERATURE);

  //[미사용] 4. 초상온도 (1분 평균)


  //[사용]  5. 지중온도 (5cm, 1분 평균)
  p_kma3->soil_temperature_5cm.data = mRealAws.mSoilTemp5cm.sReal;
  p_kma3->soil_temperature_5cm.min = mRealAws.mSoilTemp5cm.sMin;
  p_kma3->soil_temperature_5cm.max = mRealAws.mSoilTemp5cm.sMax; 
  p_kma3->soil_temperature_5cm.err = get_sensor_err(B5_SOIL_TEMPERATURE_5CM);
  //[사용] 6. 지중온도 (10cm, 1분 평균)
  p_kma3->soil_temperature_10cm.data = mRealAws.mSoilTemp10cm.sReal;
  p_kma3->soil_temperature_10cm.min = mRealAws.mSoilTemp10cm.sMin;
  p_kma3->soil_temperature_10cm.max = mRealAws.mSoilTemp10cm.sMax;
  p_kma3->soil_temperature_10cm.err = get_sensor_err(B6_SOIL_TEMPERATURE_10CM);
  //[사용] 7. 지중온도 (20cm, 1분 평균)
  p_kma3->soil_temperature_20cm.data = mRealAws.mSoilTemp20cm.sReal;
  p_kma3->soil_temperature_20cm.min = mRealAws.mSoilTemp20cm.sMin;
  p_kma3->soil_temperature_20cm.max = mRealAws.mSoilTemp20cm.sMax;
  p_kma3->soil_temperature_20cm.err = get_sensor_err(B7_SOIL_TEMPERATURE_20CM);
  //[사용]  8. 지중온도 (30cm, 1분 평균)
  p_kma3->soil_temperature_30cm.data = mRealAws.mSoilTemp30cm.sReal;
  p_kma3->soil_temperature_30cm.min = mRealAws.mSoilTemp30cm.sMin;
  p_kma3->soil_temperature_30cm.max = mRealAws.mSoilTemp30cm.sMax;
  p_kma3->soil_temperature_30cm.err = get_sensor_err(B8_SOIL_TEMPERATURE_30CM);
  //[사용]  9. 지중온도 (50cm, 1분 평균)
  p_kma3->soil_temperature_50cm.data = mRealAws.mSoilTemp50cm.sReal;
  p_kma3->soil_temperature_50cm.min = mRealAws.mSoilTemp50cm.sMin;
  p_kma3->soil_temperature_50cm.max = mRealAws.mSoilTemp50cm.sMax;
  p_kma3->soil_temperature_50cm.err = get_sensor_err(B9_SOIL_TEMPERATURE_50CM);
  //[사용] 10. 지중온도 (1.0m, 1분 평균)
  p_kma3->soil_temperature_1m.data = mRealAws.mSoilTemp1_0m.sReal;
  p_kma3->soil_temperature_1m.min = mRealAws.mSoilTemp1_0m.sMin;
  p_kma3->soil_temperature_1m.max = mRealAws.mSoilTemp1_0m.sMax;
  p_kma3->soil_temperature_1m.err = get_sensor_err(B10_SOIL_TEMPERATURE_100CM);
  //[사용] 11. 지중온도 (1.5m, 1분 평균)
  p_kma3->soil_temperature_1_5m.data = mRealAws.mSoilTemp1_5m.sReal;
  p_kma3->soil_temperature_1_5m.min = mRealAws.mSoilTemp1_5m.sMin;
  p_kma3->soil_temperature_1_5m.max = mRealAws.mSoilTemp1_5m.sMax;
  p_kma3->soil_temperature_1_5m.err = get_sensor_err(B11_SOIL_TEMPERATURE_150CM);

  // [사용] 12. 지중온도(3.0m, 1분 평균) 
  p_kma3 -> soil_temperature_3m.data =  p_raw->soil_temperature_3m.data;
  p_kma3->soil_temperature_3m.err = get_sensor_err(B12_SOIL_TEMPERATURE_300CM);
  p_kma3->soil_temperature_5m.data = p_raw->soil_temperature_5m.data;
  p_kma3->soil_temperature_5m.err = get_sensor_err(B13_SOIL_TEMPERATURE_500CM);


  //에러 변수 업데이트 
  //[사용]
  kma3_update_sensor_status(A1_TEMPERATURE, p_kma3->X_sensorStatus, get_sensor_err(A1_TEMPERATURE));
  kma3_update_sensor_status(A2_WIND_DIRECTION, p_kma3->X_sensorStatus,
                            get_sensor_err(A2_WIND_DIRECTION));
  //[사용]
  kma3_update_sensor_status(A3_WIND_SPEED, p_kma3->X_sensorStatus, get_sensor_err(A3_WIND_SPEED));
  //[사용]
  kma3_update_sensor_status(A6_RAINFALL_DOT5_1MM, p_kma3->X_sensorStatus,
                            get_sensor_err(A6_RAINFALL_DOT5_1MM));
  //[사용]
  kma3_update_sensor_status(A7_PRESSURE, p_kma3->X_sensorStatus, get_sensor_err(A7_PRESSURE));
  //[사용]
  kma3_update_sensor_status(A8_RAIN_PRESENT, p_kma3->X_sensorStatus,
                            get_sensor_err(A8_RAIN_PRESENT));
  //[사용]
  kma3_update_sensor_status(A9_SNOW_DEPTH, p_kma3->X_sensorStatus, get_sensor_err(A9_SNOW_DEPTH));
  //[사용]
  kma3_update_sensor_status(A10_RELATIVE_HUMIDITY, p_kma3->X_sensorStatus,
                            get_sensor_err(A10_RELATIVE_HUMIDITY));
  //[사용]
  kma3_update_sensor_status(B1_SOLAR_RADIATION, p_kma3->X_sensorStatus,   get_sensor_err(B1_SOLAR_RADIATION));
  //[사용]
  kma3_update_sensor_status(B2_SUNSHINE_DURATION, p_kma3->X_sensorStatus,  get_sensor_err(B2_SUNSHINE_DURATION));
  //[사용]
  kma3_update_sensor_status(B5_SOIL_TEMPERATURE_5CM, p_kma3->X_sensorStatus,  get_sensor_err(B5_SOIL_TEMPERATURE_5CM));
  //[사용]
  kma3_update_sensor_status(B6_SOIL_TEMPERATURE_10CM, p_kma3->X_sensorStatus,   get_sensor_err(B6_SOIL_TEMPERATURE_10CM));
  //[사용]
  kma3_update_sensor_status(B7_SOIL_TEMPERATURE_20CM, p_kma3->X_sensorStatus, get_sensor_err(B7_SOIL_TEMPERATURE_20CM));
  //[사용]
  kma3_update_sensor_status(B8_SOIL_TEMPERATURE_30CM, p_kma3->X_sensorStatus,  get_sensor_err(B8_SOIL_TEMPERATURE_30CM));
  //[사용]
  kma3_update_sensor_status(B9_SOIL_TEMPERATURE_50CM, p_kma3->X_sensorStatus, get_sensor_err(B9_SOIL_TEMPERATURE_50CM));
  //[사용]
  kma3_update_sensor_status(B10_SOIL_TEMPERATURE_100CM, p_kma3->X_sensorStatus,  get_sensor_err(B10_SOIL_TEMPERATURE_100CM));
  //[사용]
  kma3_update_sensor_status(B11_SOIL_TEMPERATURE_150CM, p_kma3->X_sensorStatus, get_sensor_err(B11_SOIL_TEMPERATURE_150CM));

  //[사용]
  kma3_update_sensor_status(B12_SOIL_TEMPERATURE_300CM, p_kma3->X_sensorStatus, get_sensor_err(B12_SOIL_TEMPERATURE_300CM));
  //[사용]
  kma3_update_sensor_status(B13_SOIL_TEMPERATURE_500CM, p_kma3->X_sensorStatus,     get_sensor_err(B13_SOIL_TEMPERATURE_500CM));


  BIT_UPDATE(p_kma3->Y_volateStatus, System.dc_error, KMA2_PWRSTAT_DC_INPUT_ERR);
  BIT_UPDATE(p_kma3->Y_volateStatus, System.battery_error, KMA2_PWRSTAT_BATTERY_ERR);
  p_kma3->Y_volateStatus &= 0xF3;
  p_kma3->Y_volateStatus |=System.ac_status<<2;
  BIT_UPDATE(p_kma3->Y_volateStatus, System.door_opened, KMA2_PWRSTAT_DOOR_OPEN);

  p_kma3->time = Date_Time;
  send_kma_data(eKMA_DATA_Q_AVG,p_kma3);//실시간값을 공유자원 충돌없이 AI요청시 처리하기위한 목적
}

/**
 * @brief 센서 사용여부 설정
 * @details 
 * TODO: 센서는 사용여부는 부팅시 결정되기때문에 한번만 호출 되면 됨
 * 한번만 호출 되도록 개선 필요요
 * 
 */
void check_sensor_use(void)
{
  kma_data_ex_t *p_kma_data;

  for (int min = eAWS_DATA_REAL; min <= eAWS_DATA_RAW; min++)
  {
    p_kma_data = get_kma_data((eAWS_DATA_MIN_t)min);

    p_kma_data->temperature.enable = g_p_raw->data[A1_TEMPERATURE].enable;
    p_kma_data->wind_direction_avg.enable = g_p_raw->data[A2_WIND_DIRECTION].enable;
    p_kma_data->wind_speed_avg.enable = g_p_raw->data[A3_WIND_SPEED].enable;

    if (p_kma_data->wind_direction_avg.enable)
    {
      p_kma_data->wind_direction_instant.enable = true;
    }

    if (p_kma_data->wind_speed_avg.enable)
    {
      p_kma_data->wind_speed_instant.enable = true;
    }

    p_kma_data->precipitation.enable = g_p_raw->data[A6_RAINFALL_DOT5_1MM].enable;
    p_kma_data->pressure.enable = g_p_raw->data[A7_PRESSURE].enable;
    p_kma_data->precipitation_presence.enable = g_p_raw->data[A8_RAIN_PRESENT].enable;
    p_kma_data->snowfall.enable = g_p_raw->data[A9_SNOW_DEPTH].enable;
    p_kma_data->relative_humidity.enable = g_p_raw->data[A10_RELATIVE_HUMIDITY].enable;
    p_kma_data->precipitation_fine.enable = g_p_raw->data[A11_RAINFALL_DOT1MM].enable;
    p_kma_data->solar_radiation.enable = g_p_raw->data[B1_SOLAR_RADIATION].enable;
    p_kma_data->sunshine_duration.enable = g_p_raw->data[B2_SUNSHINE_DURATION].enable;
    p_kma_data->surface_temperature.enable = g_p_raw->data[B3_GROUND_TEMPERATURE].enable;
    p_kma_data->grass_temperature.enable = g_p_raw->data[B4_SURFACE_TEMPERATURE].enable;
    p_kma_data->soil_temperature_5cm.enable = g_p_raw->data[B5_SOIL_TEMPERATURE_5CM].enable;
    p_kma_data->soil_temperature_10cm.enable = g_p_raw->data[B6_SOIL_TEMPERATURE_10CM].enable;
    p_kma_data->soil_temperature_20cm.enable = g_p_raw->data[B7_SOIL_TEMPERATURE_20CM].enable;
    p_kma_data->soil_temperature_30cm.enable = g_p_raw->data[B8_SOIL_TEMPERATURE_30CM].enable;
    p_kma_data->soil_temperature_50cm.enable = g_p_raw->data[B9_SOIL_TEMPERATURE_50CM].enable;
    p_kma_data->soil_temperature_1m.enable = g_p_raw->data[B10_SOIL_TEMPERATURE_100CM].enable;
    p_kma_data->soil_temperature_1_5m.enable = g_p_raw->data[B11_SOIL_TEMPERATURE_150CM].enable;
    p_kma_data->soil_temperature_3m.enable = g_p_raw->data[B12_SOIL_TEMPERATURE_300CM].enable;
    p_kma_data->soil_temperature_5m.enable = g_p_raw->data[B13_SOIL_TEMPERATURE_500CM].enable;
    p_kma_data->cloud_height_1st.enable = g_p_raw->data[C1_CLOUD_BASE1].enable;
    p_kma_data->cloud_height_2nd.enable = g_p_raw->data[C2_CLOUD_BASE2].enable;
    p_kma_data->cloud_height_3rd.enable = g_p_raw->data[C3_CLOUD_BASE3].enable;
    p_kma_data->cloud_amount.enable = g_p_raw->data[C4_CLOUD_COVER].enable;
    p_kma_data->visibility.enable = g_p_raw->data[C5_VISIBILITY].enable;

    p_kma_data->pm10_concentration.enable = g_p_raw->data[C6_PM10].enable;
    p_kma_data->pm25_concentration.enable = g_p_raw->data[C7_PM2DOT5].enable;
    p_kma_data->net_radiation.enable = g_p_raw->data[C8_NET_RADIATION].enable;
    p_kma_data->total_radiation.enable = g_p_raw->data[C9_TOTAL_RADIATION].enable;
    p_kma_data->reflected_radiation.enable = g_p_raw->data[C10_REFLECTED_RADIATION].enable;
    p_kma_data->direct_radiation.enable = g_p_raw->data[C11_DIRECT_SOLAR].enable;
    p_kma_data->current_weather.enable = g_p_raw->data[C12_CURRENT_WEATHER].enable;
    p_kma_data->soil_moisture_10cm.enable = g_p_raw->data[N1_SOIL_MOISTURE_10CM].enable;
    p_kma_data->soil_moisture_20cm.enable = g_p_raw->data[N2_SOIL_MOISTURE_20CM].enable;
    p_kma_data->soil_moisture_30cm.enable = g_p_raw->data[N3_SOIL_MOISTURE_30CM].enable;
    p_kma_data->soil_moisture_50cm.enable = g_p_raw->data[N4_SOIL_MOISTURE_50CM].enable;
    p_kma_data->illuminance.enable = g_p_raw->data[N5_ILLUMINANCE].enable;
    p_kma_data->wind_speed_1_5m.enable = g_p_raw->data[N6_WIND_VELOCITY_150CM].enable;
    p_kma_data->wind_speed_4m.enable = g_p_raw->data[N7_WIND_VELOCITY_400CM].enable;
    p_kma_data->instant_wind_speed_1_5m.enable = g_p_raw->data[N8_INSTANT_VELOCITY_150CM].enable;
    p_kma_data->instant_wind_speed_4m.enable = g_p_raw->data[N9_INSTANT_VELOCITY_400CM].enable;
    p_kma_data->temperature_0_5m.enable = g_p_raw->data[N10_AIR_TEMPERATURE_50CM].enable;
    p_kma_data->temperature_4m.enable = g_p_raw->data[N11_AIR_TEMPERATURE_400CM].enable;
    p_kma_data->humidity_0_5m.enable = g_p_raw->data[N12_HUMIDITY_50CM].enable;
    p_kma_data->humidity_4m.enable = g_p_raw->data[N13_HUMIDITY_400CM].enable;
    p_kma_data->tacometer.enable = g_p_raw->data[I1_TACHOMETER].enable;
  }
}





/*
원본값을 업데이트한다.
원본값을 aws 자료형으로 보관한다.
*/
/*
원본값을 업데이트한다.
원본값을 aws 자료형으로 보관한다.
*/
void update_raw(void)
{
  kma_data_ex_t *p_kma_data;

  p_kma_data = get_kma_data((eAWS_DATA_MIN_t)eAWS_DATA_RAW);

  p_kma_data->temperature.raw.f = g_p_raw->data[A1_TEMPERATURE].data.f;
  p_kma_data->temperature.err = g_p_raw->data[A1_TEMPERATURE].err;

  p_kma_data->wind_direction_avg.raw.f = g_p_raw->data[A2_WIND_DIRECTION].data.f;
  p_kma_data->wind_direction_avg.err = g_p_raw->data[A2_WIND_DIRECTION].err;

  p_kma_data->wind_speed_avg.raw.f = g_p_raw->data[A3_WIND_SPEED].data.f;
  p_kma_data->wind_speed_avg.err = g_p_raw->data[A3_WIND_SPEED].err;

  p_kma_data->precipitation.raw.f = g_p_raw->data[A6_RAINFALL_DOT5_1MM].data.f;
  p_kma_data->precipitation.err = g_p_raw->data[A6_RAINFALL_DOT5_1MM].err;

  // 강수량은 250ms마다 처리되기대문에 이전값 유지가 없어서 마지막으로 우량이 발생한 시간으로
  // 처리한다.
  if (p_kma_data->precipitation.raw.f)
    p_kma_data->precipitation.last_time = time_timestamp();

  p_kma_data->pressure.raw.f = g_p_raw->data[A7_PRESSURE].data.f;
  p_kma_data->pressure.err = g_p_raw->data[A7_PRESSURE].err;

  p_kma_data->precipitation_presence.raw.b = g_p_raw->data[A8_RAIN_PRESENT].data.b;
  p_kma_data->precipitation_presence.err = g_p_raw->data[A8_RAIN_PRESENT].err;

  p_kma_data->snowfall.raw.f = g_p_raw->data[A9_SNOW_DEPTH].data.i;
  p_kma_data->snowfall.err = g_p_raw->data[A9_SNOW_DEPTH].err;

  p_kma_data->relative_humidity.raw.f = g_p_raw->data[A10_RELATIVE_HUMIDITY].data.f;
  p_kma_data->relative_humidity.err = g_p_raw->data[A10_RELATIVE_HUMIDITY].err;

  p_kma_data->precipitation_fine.raw.f = g_p_raw->data[A11_RAINFALL_DOT1MM].data.i;
  p_kma_data->precipitation_fine.err = g_p_raw->data[A11_RAINFALL_DOT1MM].err;

  p_kma_data->solar_radiation.raw.f = g_p_raw->data[B1_SOLAR_RADIATION].data.f;
  p_kma_data->solar_radiation.err = g_p_raw->data[B1_SOLAR_RADIATION].err;

  // 일조는 기준값과 차이가 0.01차이라면 같은 값으로 처리하자
  // 전압이 특정전압 이상인경우 1(일조 있음)으로 처리리
  if (is_over_threshold(g_p_raw->data[B2_SUNSHINE_DURATION].data.f, kSunshine_threshold, 0.01))
  {
    p_kma_data->sunshine_duration.raw.f = 1;
  }
  else
  {
    p_kma_data->sunshine_duration.raw.f = 0;
  }

  p_kma_data->sunshine_duration.err = g_p_raw->data[B2_SUNSHINE_DURATION].err;

  p_kma_data->surface_temperature.raw.f = g_p_raw->data[B3_GROUND_TEMPERATURE].data.f;
  p_kma_data->surface_temperature.err = g_p_raw->data[B3_GROUND_TEMPERATURE].err;

  p_kma_data->grass_temperature.raw.f = g_p_raw->data[B4_SURFACE_TEMPERATURE].data.f;
  p_kma_data->grass_temperature.err = g_p_raw->data[B4_SURFACE_TEMPERATURE].err;

  p_kma_data->soil_temperature_5cm.raw.f = g_p_raw->data[B5_SOIL_TEMPERATURE_5CM].data.f;
  p_kma_data->soil_temperature_5cm.err = g_p_raw->data[B5_SOIL_TEMPERATURE_5CM].err;

  p_kma_data->soil_temperature_10cm.raw.f = g_p_raw->data[B6_SOIL_TEMPERATURE_10CM].data.f;
  p_kma_data->soil_temperature_10cm.err = g_p_raw->data[B6_SOIL_TEMPERATURE_10CM].err;

  p_kma_data->soil_temperature_20cm.raw.f = g_p_raw->data[B7_SOIL_TEMPERATURE_20CM].data.f;
  p_kma_data->soil_temperature_20cm.err = g_p_raw->data[B7_SOIL_TEMPERATURE_20CM].err;

  p_kma_data->soil_temperature_30cm.raw.f = g_p_raw->data[B8_SOIL_TEMPERATURE_30CM].data.f;
  p_kma_data->soil_temperature_30cm.err = g_p_raw->data[B8_SOIL_TEMPERATURE_30CM].err;

  p_kma_data->soil_temperature_50cm.raw.f = g_p_raw->data[B9_SOIL_TEMPERATURE_50CM].data.f;
  p_kma_data->soil_temperature_50cm.err = g_p_raw->data[B9_SOIL_TEMPERATURE_50CM].err;

  p_kma_data->soil_temperature_1m.raw.f = g_p_raw->data[B10_SOIL_TEMPERATURE_100CM].data.f;
  p_kma_data->soil_temperature_1m.err = g_p_raw->data[B10_SOIL_TEMPERATURE_100CM].err;

  p_kma_data->soil_temperature_1_5m.raw.f = g_p_raw->data[B11_SOIL_TEMPERATURE_150CM].data.f;
  p_kma_data->soil_temperature_1_5m.err = g_p_raw->data[B11_SOIL_TEMPERATURE_150CM].err;

  p_kma_data->soil_temperature_3m.raw.f = g_p_raw->data[B12_SOIL_TEMPERATURE_300CM].data.f;
  p_kma_data->soil_temperature_3m.err = g_p_raw->data[B12_SOIL_TEMPERATURE_300CM].err;

  p_kma_data->soil_temperature_5m.raw.f = g_p_raw->data[B13_SOIL_TEMPERATURE_500CM].data.f;
  p_kma_data->soil_temperature_5m.err = g_p_raw->data[B13_SOIL_TEMPERATURE_500CM].err;

  p_kma_data->cloud_height_1st.raw.f = g_p_raw->data[C1_CLOUD_BASE1].data.f;
  p_kma_data->cloud_height_1st.err = g_p_raw->data[C1_CLOUD_BASE1].err;

  p_kma_data->cloud_height_2nd.raw.f = g_p_raw->data[C2_CLOUD_BASE2].data.f;
  p_kma_data->cloud_height_2nd.err = g_p_raw->data[C2_CLOUD_BASE2].err;

  p_kma_data->cloud_height_3rd.raw.f = g_p_raw->data[C3_CLOUD_BASE3].data.f;
  p_kma_data->cloud_height_3rd.err = g_p_raw->data[C3_CLOUD_BASE3].err;

  p_kma_data->cloud_amount.raw.f = g_p_raw->data[C4_CLOUD_COVER].data.f;
  p_kma_data->cloud_amount.err = g_p_raw->data[C4_CLOUD_COVER].err;

  p_kma_data->visibility.raw.f = g_p_raw->data[C5_VISIBILITY].data.f;
  p_kma_data->visibility.err = g_p_raw->data[C5_VISIBILITY].err;

  p_kma_data->pm10_concentration.raw.f = g_p_raw->data[C6_PM10].data.f;
  p_kma_data->pm10_concentration.err = g_p_raw->data[C6_PM10].err;

  p_kma_data->pm25_concentration.raw.f = g_p_raw->data[C7_PM2DOT5].data.f;
  p_kma_data->pm25_concentration.err = g_p_raw->data[C7_PM2DOT5].err;

  p_kma_data->net_radiation.raw.f = g_p_raw->data[C8_NET_RADIATION].data.f;
  p_kma_data->net_radiation.err = g_p_raw->data[C8_NET_RADIATION].err;

  p_kma_data->total_radiation.raw.f = g_p_raw->data[C9_TOTAL_RADIATION].data.f;
  p_kma_data->total_radiation.err = g_p_raw->data[C9_TOTAL_RADIATION].err;

  p_kma_data->reflected_radiation.raw.f = g_p_raw->data[C10_REFLECTED_RADIATION].data.f;
  p_kma_data->reflected_radiation.err = g_p_raw->data[C10_REFLECTED_RADIATION].err;

  p_kma_data->direct_radiation.raw.f = g_p_raw->data[C11_DIRECT_SOLAR].data.f;
  p_kma_data->direct_radiation.err = g_p_raw->data[C11_DIRECT_SOLAR].err;

  p_kma_data->current_weather.raw.f = g_p_raw->data[C12_CURRENT_WEATHER].data.i;
  p_kma_data->current_weather.err = g_p_raw->data[C12_CURRENT_WEATHER].err;

  p_kma_data->soil_moisture_10cm.raw.f = g_p_raw->data[N1_SOIL_MOISTURE_10CM].data.f;
  p_kma_data->soil_moisture_10cm.err = g_p_raw->data[N1_SOIL_MOISTURE_10CM].err;

  p_kma_data->soil_moisture_20cm.raw.f = g_p_raw->data[N2_SOIL_MOISTURE_20CM].data.f;
  p_kma_data->soil_moisture_20cm.err = g_p_raw->data[N2_SOIL_MOISTURE_20CM].err;

  p_kma_data->soil_moisture_30cm.raw.f = g_p_raw->data[N3_SOIL_MOISTURE_30CM].data.f;
  p_kma_data->soil_moisture_30cm.err = g_p_raw->data[N3_SOIL_MOISTURE_30CM].err;

  p_kma_data->soil_moisture_50cm.raw.f = g_p_raw->data[N4_SOIL_MOISTURE_50CM].data.f;
  p_kma_data->soil_moisture_50cm.err = g_p_raw->data[N4_SOIL_MOISTURE_50CM].err;

  p_kma_data->illuminance.raw.f = g_p_raw->data[N5_ILLUMINANCE].data.f;
  p_kma_data->illuminance.err = g_p_raw->data[N5_ILLUMINANCE].err;

  p_kma_data->wind_speed_1_5m.raw.f = g_p_raw->data[N6_WIND_VELOCITY_150CM].data.f;
  p_kma_data->wind_speed_1_5m.err = g_p_raw->data[N6_WIND_VELOCITY_150CM].err;

  p_kma_data->wind_speed_4m.raw.f = g_p_raw->data[N7_WIND_VELOCITY_400CM].data.f;
  p_kma_data->wind_speed_4m.err = g_p_raw->data[N7_WIND_VELOCITY_400CM].err;

  p_kma_data->instant_wind_speed_1_5m.raw.f = g_p_raw->data[N8_INSTANT_VELOCITY_150CM].data.f;
  p_kma_data->instant_wind_speed_1_5m.err = g_p_raw->data[N8_INSTANT_VELOCITY_150CM].err;

  p_kma_data->instant_wind_speed_4m.raw.f = g_p_raw->data[N9_INSTANT_VELOCITY_400CM].data.f;
  p_kma_data->instant_wind_speed_4m.err = g_p_raw->data[N9_INSTANT_VELOCITY_400CM].err;

  p_kma_data->temperature_0_5m.raw.f = g_p_raw->data[N10_AIR_TEMPERATURE_50CM].data.f;
  p_kma_data->temperature_0_5m.err = g_p_raw->data[N10_AIR_TEMPERATURE_50CM].err;

  p_kma_data->temperature_4m.raw.f = g_p_raw->data[N11_AIR_TEMPERATURE_400CM].data.f;
  p_kma_data->temperature_4m.err = g_p_raw->data[N11_AIR_TEMPERATURE_400CM].err;

  p_kma_data->humidity_0_5m.raw.f = g_p_raw->data[N12_HUMIDITY_50CM].data.f;
  p_kma_data->humidity_0_5m.err = g_p_raw->data[N12_HUMIDITY_50CM].err;

  p_kma_data->humidity_4m.raw.f = g_p_raw->data[N13_HUMIDITY_400CM].data.f;
  p_kma_data->humidity_4m.err = g_p_raw->data[N13_HUMIDITY_400CM].err;

  p_kma_data->tacometer.raw.f = g_p_raw->data[I1_TACHOMETER].data.f;
  p_kma_data->tacometer.err = g_p_raw->data[I1_TACHOMETER].err;
}

void update_unused_data(kma_data_ex_t *p_dest, kma_data_ex_t *p_source)
{
  //p_dest->temperature.data = p_source->temperature.data;
  //p_dest->wind_direction_avg.data = p_source->wind_direction_avg.data;
  //p_dest->wind_speed_avg.data = p_source->wind_speed_avg.data;
  //p_dest->wind_direction_instant.data = p_source->wind_direction_instant.data;
  //p_dest->wind_speed_instant.data = p_source->wind_speed_instant.data;
  //p_dest->precipitation.data = p_source->precipitation.data;
 // p_dest->pressure.data = p_source->pressure.data;
  //p_dest->precipitation_presence.data = p_source->precipitation_presence.data;
 // p_dest->snowfall.data = p_source->snowfall.data;
  //p_dest->relative_humidity.data = p_source->relative_humidity.data;
  p_dest->precipitation_fine.data = p_source->precipitation_fine.data;
  //p_dest->solar_radiation.data = p_source->solar_radiation.data;
 // p_dest->sunshine_duration.data = p_source->sunshine_duration.data;
  p_dest->surface_temperature.data = p_source->surface_temperature.data;
  p_dest->grass_temperature.data = p_source->grass_temperature.data;
  //p_dest->soil_temperature_5cm.data = p_source->soil_temperature_5cm.data;
  //p_dest->soil_temperature_10cm.data = p_source->soil_temperature_10cm.data;
  //p_dest->soil_temperature_20cm.data = p_source->soil_temperature_20cm.data;
  //p_dest->soil_temperature_30cm.data = p_source->soil_temperature_30cm.data;
  //p_dest->soil_temperature_50cm.data = p_source->soil_temperature_50cm.data;
  //p_dest->soil_temperature_1m.data = p_source->soil_temperature_1m.data;
  //p_dest->soil_temperature_1_5m.data = p_source->soil_temperature_1_5m.data;
  //p_dest->soil_temperature_3m.data = p_source->soil_temperature_3m.data;
  //p_dest->soil_temperature_5m.data = p_source->soil_temperature_5m.data;
  p_dest->cloud_height_1st.data = p_source->cloud_height_1st.data;
  p_dest->cloud_height_2nd.data = p_source->cloud_height_2nd.data;
  p_dest->cloud_height_3rd.data = p_source->cloud_height_3rd.data;
  p_dest->cloud_amount.data = p_source->cloud_amount.data;
  p_dest->visibility.data = p_source->visibility.data;
  p_dest->pm10_concentration.data = p_source->pm10_concentration.data;
  p_dest->pm25_concentration.data = p_source->pm25_concentration.data;
  p_dest->net_radiation.data = p_source->net_radiation.data;
  p_dest->total_radiation.data = p_source->total_radiation.data;
  p_dest->reflected_radiation.data = p_source->reflected_radiation.data;
  p_dest->direct_radiation.data = p_source->direct_radiation.data;
  p_dest->current_weather.data = p_source->current_weather.data;
  p_dest->soil_moisture_10cm.data = p_source->soil_moisture_10cm.data;
  p_dest->soil_moisture_20cm.data = p_source->soil_moisture_20cm.data;
  p_dest->soil_moisture_30cm.data = p_source->soil_moisture_30cm.data;
  p_dest->soil_moisture_50cm.data = p_source->soil_moisture_50cm.data;
  p_dest->illuminance.data = p_source->illuminance.data;
  p_dest->wind_speed_1_5m.data = p_source->wind_speed_1_5m.data;
  p_dest->wind_speed_4m.data = p_source->wind_speed_4m.data;
  p_dest->instant_wind_speed_1_5m.data = p_source->instant_wind_speed_1_5m.data;
  p_dest->instant_wind_speed_4m.data = p_source->instant_wind_speed_4m.data;
  p_dest->temperature_0_5m.data = p_source->temperature_0_5m.data;
  p_dest->temperature_4m.data = p_source->temperature_4m.data;
  p_dest->humidity_0_5m.data = p_source->humidity_0_5m.data;
  p_dest->humidity_4m.data = p_source->humidity_4m.data;
  p_dest->tacometer.data = p_source->tacometer.data;
}



#define SENSOR_FAIL_TIMEOUT_SEC 10

#define MS_TO_SCAN_CNT(sec) ((uint16_t)((float)sec/0.25))
/**
 * 에러가 존재하면 타임아웃 전까지는 이전값 유지
 */
uint16_t filter_data(eSENSOR_TYPE_t sensor_index,uint16_t data, uint8_t error,uint8_t *f_err)
{
  uint8_t delay=0;
  uint16_t ret_data;

  delay = g_pre_data[sensor_index].delay_count; // 처음호출시에는 dealy를 MS_TO_SCAN_CNT 같은값 설정
  if(error)//에러가 존재하면 최초의 에러는 무조건 에러로 처리 
  {
    delay++;
    if (delay >= MS_TO_SCAN_CNT(SENSOR_FAIL_TIMEOUT_SEC))//에러가 계속 발생하면
    {
      delay = 0;
      ret_data = data;//현재값을 실제 값으로 처리
      g_pre_data[sensor_index].data = ret_data;
      g_pre_data[sensor_index].err = 1;
    }
    else
    {
      ret_data = g_pre_data[sensor_index].data;//에러는 있지만 타임아웃 전이면 이전값으로 처리
    }
    g_pre_data[sensor_index].delay_count = delay;
  }
  else
  {
    g_pre_data[sensor_index].data = data; //에러가 없으면 즉시 현재값 사용
    g_pre_data[sensor_index].err = 0;
    g_pre_data[sensor_index].delay_count = 0;
    ret_data = data;
  }

  *f_err = g_pre_data[sensor_index].err;

  return ret_data;
}


void filter_init(void)
{
  for(int i = 0 ; i< _countof(g_pre_data);i++)
  {
    g_pre_data[i].err = 0xFF;
    g_pre_data[i].delay_count = MS_TO_SCAN_CNT(SENSOR_FAIL_TIMEOUT_SEC); // 제품 부팅시에는 처음 측정하는 값을 즉시 반영
  }


}





void upate_wind(void)
{
  int32_t wind_speed_max;
  int32_t wind_direction_max;

  AWS_DATA_STRUCT *pAws[] = {&mRealAws, & m10MinAws, &mHourAws, &mDayAws};
  eWIND_MAX_t wind[] = {eWIND_MAX_REAL, eWIND_MAX_10MIN,
                        eWIND_MAX_HOUR,
                        eWIND_MAX_DAY};

  for (int i = 0; i < _countof(pAws); i++)
  {
    read_wind_max(wind[i], &wind_speed_max, &wind_direction_max);
    pAws[i]->mWind.mDirection.sMax = wind_direction_max;
    pAws[i]->mWind.mSpeed.sMax = wind_speed_max;
  }


}

//실시간 최대 최소값은 1분 최대 최소 값을 사용한다.
void update_sensor_real(void)
{
  AWS_DATA_STRUCT *pAws = &mRealAws;


  // 온도
  pAws->mTemperature.sMin = read_current_data_min(eAVG_TEMPERATURE, g_1min_min_max);
  pAws->mTemperature.sMax = read_current_data_max(eAVG_TEMPERATURE, g_1min_min_max);

  // 기압
  pAws->mBarometric.sMin = read_current_data_min(eAVG_PRESSURE, g_1min_min_max);
  pAws->mBarometric.sMax = read_current_data_max(eAVG_PRESSURE, g_1min_min_max);

  // 습도
  pAws->mHumidity.sMin = read_current_data_min(eAVG_RELATIVE_HUMIDITY, g_1min_min_max);
  pAws->mHumidity.sMax = read_current_data_max(eAVG_RELATIVE_HUMIDITY, g_1min_min_max);

  // 지면온도
  pAws->mGndTemp.sReal = mRealAws.mGndTemp.sReal;
  pAws->mGndTemp.sMin = read_current_data_min(eAVG_GROUND_TEMPERATURE, g_1min_min_max);
  pAws->mGndTemp.sMax = read_current_data_max(eAVG_GROUND_TEMPERATURE, g_1min_min_max);
  // 초상 온도
  pAws->mGrassTemp.sReal = mRealAws.mGrassTemp.sReal;
  pAws->mGrassTemp.sMin = read_current_data_min(eAVG_SURFACE_TEMPERATURE, g_1min_min_max);
  pAws->mGrassTemp.sMax = read_current_data_max(eAVG_SURFACE_TEMPERATURE, g_1min_min_max);

  // 지중온도 처리 2017.04.03
  pAws->mSoilTemp5cm.sMin = read_current_data_min(eAVG_SOIL_TEMPERATURE_5CM, g_1min_min_max);
  pAws->mSoilTemp5cm.sMax = read_current_data_max(eAVG_SOIL_TEMPERATURE_5CM, g_1min_min_max);

  pAws->mSoilTemp10cm.sMin = read_current_data_min(eAVG_SOIL_TEMPERATURE_10CM, g_1min_min_max);
  pAws->mSoilTemp10cm.sMax = read_current_data_max(eAVG_SOIL_TEMPERATURE_10CM, g_1min_min_max);

  pAws->mSoilTemp20cm.sMin = read_current_data_min(eAVG_SOIL_TEMPERATURE_20CM, g_1min_min_max);
  pAws->mSoilTemp20cm.sMax = read_current_data_max(eAVG_SOIL_TEMPERATURE_20CM, g_1min_min_max);

  pAws->mSoilTemp30cm.sMin = read_current_data_min(eAVG_SOIL_TEMPERATURE_30CM, g_1min_min_max);
  pAws->mSoilTemp30cm.sMax = read_current_data_max(eAVG_SOIL_TEMPERATURE_30CM, g_1min_min_max);

  pAws->mSoilTemp50cm.sMin = read_current_data_min(eAVG_SOIL_TEMPERATURE_50CM, g_1min_min_max);
  pAws->mSoilTemp50cm.sMax = read_current_data_max(eAVG_SOIL_TEMPERATURE_50CM, g_1min_min_max);

  pAws->mSoilTemp1_0m.sMin = read_current_data_min(eAVG_SOIL_TEMPERATURE_100CM, g_1min_min_max);
  pAws->mSoilTemp1_0m.sMax = read_current_data_max(eAVG_SOIL_TEMPERATURE_100CM, g_1min_min_max);

  pAws->mSoilTemp1_5m.sMin = read_current_data_min(eAVG_SOIL_TEMPERATURE_150CM, g_1min_min_max);
  pAws->mSoilTemp1_5m.sMax = read_current_data_max(eAVG_SOIL_TEMPERATURE_150CM, g_1min_min_max);
}

void DUALPORT_TASK(void *arg)
{
  uint8_t f_err = 0;
  uint8_t sensor_err = 0;
  int32_t wind_speed=0;
  int32_t wind_direction=0;
  uint16_t data;
  uint16_t rain_p_on_delay = 0;
  uint16_t rain_p_off_delay=0;
  float wind_speed_mavg = 0;
  float wind_direction_mavg = 0;
  float adj_wind_speed;
  float adj_wind_direction;
  DATE_TIME_BUF ct;
  DATE_TIME_BUF time_old;
  AWS_DATA_STRUCT *pAws;
  sensor_t *p_sensor_config = get_sensor_config_copy();

  pAws = &mRealAws;

  calculate_rain(); 
  calculate_sunshine();
  filter_init();

  /*
  메모리를 아끼기위해 g_p_raw 하나만 사용
  */
  g_p_raw = user_malloc(sizeof(measure_data_1s_t));
  memset(g_p_raw,0,sizeof(measure_data_1s_t));

  osDelay(2000);// task measure 측정이 최소 1회 수행 후 동작하도록 지연 

  time_old = Date_Time;
  while (1)
  {
    is_measurement_1s(g_p_raw, 0);                     // 업데이트된 값 없으면 이전값 유지

    if (is_measurement_250(&g_raw_250, osWaitForever)) // 250ms마다 최신값 사용
    {
      g_p_raw->data[A2_WIND_DIRECTION] = g_raw_250.data[eA2_WIND_DIRECTION];
      g_p_raw->data[A3_WIND_SPEED] = g_raw_250.data[eA3_WIND_SPEED];
      g_p_raw->data[A6_RAINFALL_DOT5_1MM] = g_raw_250.data[eA6_RAINFALL_DOT5_1MM];
    }

    ct = Date_Time;

    check_sensor_use();
    update_raw();

      // 온도
      data = TempCalc(&sensor_err);
      pAws->mTemperature.sReal = filter_data(A1_TEMPERATURE, data, sensor_err, &f_err);
      update_sensor_err(A1_TEMPERATURE, f_err);

      // 풍향
      data = WindDirecCalc(&sensor_err);
      wind_direction = filter_data(A2_WIND_DIRECTION, data, sensor_err, &f_err);
      update_sensor_err(A2_WIND_DIRECTION, f_err);

      // 풍속
      data = WindSpeedCalc(&sensor_err);
      wind_speed = filter_data(A3_WIND_SPEED, data, sensor_err, &f_err);
      update_sensor_err(A3_WIND_SPEED, f_err);

     /*
     평균 풍향 풍속은 함께 처리되는데 한개의 값이 잘못되어도 다른 값이 정상처리 되도록 
     풍속이 고장나면 오직 각도 산출만을 위해 0.1보다 작은값으로 설정
     풍향이 고장나면 풍속만 표시되도록 각도를 0으로 고정
     */
      adj_wind_speed = (wind_speed == AWS_DATA_ERR_VAL)?0.01:wind_speed/10.0f;
      adj_wind_direction = (wind_direction == AWS_DATA_ERR_VAL) ? 0 : wind_direction / 10.0f;

      add_wind_sample(adj_wind_speed, adj_wind_direction);
      calculate_wind_moving_avg(&wind_speed_mavg, &wind_direction_mavg);
      update_wind_vector_avg_1min(wind_speed_mavg, wind_direction_mavg);

      g_aws_inst.wind_speed = (uint16_t)(wind_speed_mavg * 10);
      g_aws_inst.wind_direction = (uint16_t)(wind_direction_mavg * 10);
      pAws->mWind.mSpeed.sReal = (uint16_t)(wind_speed_mavg * 10);
      pAws->mWind.mDirection.sReal = (uint16_t)(wind_direction_mavg * 10);

      calculate_wind_max(eWIND_MAX_REAL, g_aws_inst.wind_speed, g_aws_inst.wind_direction);
      calculate_wind_max(eWIND_MAX_1MIN, g_aws_inst.wind_speed, g_aws_inst.wind_direction);
      calculate_wind_max(eWIND_MAX_10MIN, g_aws_inst.wind_speed, g_aws_inst.wind_direction);
      calculate_wind_max(eWIND_MAX_HOUR, g_aws_inst.wind_speed, g_aws_inst.wind_direction);
      calculate_wind_max(eWIND_MAX_DAY, g_aws_inst.wind_speed, g_aws_inst.wind_direction);

      upate_wind();

      g_rainfall.current += get_rain_mm(&sensor_err);
      update_sensor_err(A6_RAINFALL_DOT5_1MM, sensor_err);

      // 기압
      data = BarometricCalc(&sensor_err);
      pAws->mBarometric.sReal = filter_data(A7_PRESSURE, data, sensor_err, &f_err);
      update_sensor_err(A7_PRESSURE, f_err);

      // 강우 감지
      if (is_raining(&sensor_err)) // Off Delay 적용 함
      {
        rain_p_on_delay++;
        rain_p_off_delay = 0;
        if (rain_p_on_delay >= MS_TO_SCAN_CNT(get_rain_present_config()->delay_sec))
        {
          rain_p_on_delay =0;
          update_sensor_err(A8_RAIN_PRESENT, sensor_err);
          pAws->mRainDetect.sReal = 0x000a;

        }
      }
      else
      {
        rain_p_on_delay = 0;
        if (rain_p_off_delay++ > MS_TO_SCAN_CNT(get_rain_present_config()->delay_sec ))
        {
          rain_p_off_delay=0;
          pAws->mRainDetect.sReal = 0x0000;
        }
      }

      // 적설
      data = SnowCalc(&sensor_err);
      pAws->mSnowFall.sReal = filter_data(A9_SNOW_DEPTH, data, sensor_err, &f_err);
      update_sensor_err(A9_SNOW_DEPTH, f_err);

      // 상대습도
      data = HumidityCalc(&sensor_err);
      pAws->mHumidity.sReal = filter_data(A10_RELATIVE_HUMIDITY, data, sensor_err, &f_err);
      update_sensor_err(A10_RELATIVE_HUMIDITY, f_err);

      // 일사
      data = SolarRadCalc(&sensor_err);
      pAws->mSolarRad.sReal = filter_data(B1_SOLAR_RADIATION, data, sensor_err, &f_err);
      update_sensor_err(B1_SOLAR_RADIATION, f_err);

      // 일조 
      data = SunshineCalc(&sensor_err);
      pAws->mSunshine.sReal = filter_data(B2_SUNSHINE_DURATION, data, sensor_err, &f_err);
      update_sensor_err(B2_SUNSHINE_DURATION, f_err);

      // 지중온도 5cm
      data = TempCalcExt(SOLITEMP5CM_CHN, &sensor_err);
      pAws->mSoilTemp5cm.sReal = filter_data(B5_SOIL_TEMPERATURE_5CM, data, sensor_err, &f_err);
      update_sensor_err(B5_SOIL_TEMPERATURE_5CM, f_err);

      // 지중온도 10cm
      data = TempCalcExt(SOLITEMP10CM_CHN, &sensor_err);
      pAws->mSoilTemp10cm.sReal = filter_data(B6_SOIL_TEMPERATURE_10CM, data, sensor_err, &f_err);
      update_sensor_err(B6_SOIL_TEMPERATURE_10CM, f_err);

      // 지중온도 20cm
      data = TempCalcExt(SOLITEMP20CM_CHN, &sensor_err);
      pAws->mSoilTemp20cm.sReal = filter_data(B7_SOIL_TEMPERATURE_20CM, data, sensor_err, &f_err);
      update_sensor_err(B7_SOIL_TEMPERATURE_20CM, f_err);

      // 지중온도 30cm
    data = TempCalcExt(SOLITEMP30CM_CHN, &sensor_err);
    pAws->mSoilTemp30cm.sReal = filter_data(B8_SOIL_TEMPERATURE_30CM, data, sensor_err, &f_err);
    update_sensor_err(B8_SOIL_TEMPERATURE_30CM, f_err);

    // 지중온도 50cm
    data = TempCalcExt(SOLITEMP50CM_CHN, &sensor_err);
    pAws->mSoilTemp50cm.sReal = filter_data(B9_SOIL_TEMPERATURE_50CM, data, sensor_err, &f_err);
    update_sensor_err(B9_SOIL_TEMPERATURE_50CM, f_err);
    // 지중온도 1m
    data = TempCalcExt(SOLITEMP1_0M_CHN, &sensor_err);
    pAws->mSoilTemp1_0m.sReal = filter_data(B10_SOIL_TEMPERATURE_100CM, data, sensor_err, &f_err);
    update_sensor_err(B10_SOIL_TEMPERATURE_100CM, f_err);

    // 지중온도 1.5m
    data = TempCalcExt(SOLITEMP1_5M_CHN, &sensor_err);
    pAws->mSoilTemp1_5m.sReal = filter_data(B11_SOIL_TEMPERATURE_150CM, data, sensor_err, &f_err);
    update_sensor_err(B11_SOIL_TEMPERATURE_150CM, f_err);



    // 센서 불량 처리
    pAws->mStatus.sReal = 0;

    if (kma_is_sensor_error(A6_RAINFALL_DOT5_1MM))
      pAws->mStatus.sMin |= RAINFALLFAIL_BIT;
    else
      pAws->mStatus.sMin &= ~(RAINFALLFAIL_BIT);

    if (wind_speed >= 1050)
      pAws->mStatus.sMin |= WINDSPEEDFAIL_BIT;
    else
      pAws->mStatus.sMin &= ~(WINDSPEEDFAIL_BIT);

    if (wind_direction >= 3600)
      pAws->mStatus.sMin |= WINDDIRECFAIL_BIT;
    else
      pAws->mStatus.sMin &= ~(WINDDIRECFAIL_BIT);

    if (pAws->mTemperature.sReal >= 9999)
      pAws->mStatus.sMin |= TEMPERATUREFAIL_BIT;
    else
      pAws->mStatus.sMin &= ~(TEMPERATUREFAIL_BIT);

    if (pAws->mBarometric.sReal >= 19999)
      pAws->mStatus.sMin |= BAROMETRICFAIL_BIT;
    else
      pAws->mStatus.sMin &= ~(BAROMETRICFAIL_BIT);

    if (pAws->mHumidity.sReal >= 19999)
      pAws->mStatus.sMin |= HUMIDITYFAIL_BIT;
    else
      pAws->mStatus.sMin &= ~(HUMIDITYFAIL_BIT);

    schedule_process(&ct, &time_old);
    update_sensor_real();
    update_kma_real();
    // 현재 값연산 없는 항목은 원본값으로 처리
    //update_unused_data(get_kma_data(eAWS_DATA_REAL), get_kma_data(eAWS_DATA_RAW));
    update_unused_data(get_kma_data(eAWS_DATA_1MIN), get_kma_data(eAWS_DATA_RAW));
  }
}

const osThreadAttr_t KdualportTask_attributes = {
    .name = "DUALPORT_TASK",
    .stack_size = TASK_STACK(TASK_DUALPORT_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_DUALPORT_DEF),
};
void dualportTask_init(void)
{
  aws_min_max_init();
  kma_data_q_init();

  osThreadNew(DUALPORT_TASK, NULL, &KdualportTask_attributes);

}

