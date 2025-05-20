#include "dualport.h"

#include <stdbool.h>

#include "app_sensor.h"
#include "aws_data.h"

#include "cmsis_os2.h"
#include "config_app.h"
#include "config_nvm.h"
#include "old_aws_define.h"
#include "schedule.h"
#include "task_measure.h"
#include "user_heap.h"
#include "utile_filter.h"
#include "utile_time.h"
#include "kma3.h"

#define AWS_DATA_ERR_VAL 9999

measure_data_1s_t *g_p_raw = NULL;
uint8_t g_kma_err[SENSOR_LIST_MAX];
measure_data_250ms_t g_raw_250;


void update_sensor_err(eSENSOR_LIST_t sensor, uint8_t code)
{
  g_kma_err[sensor] = code;

  kma_update_sensor_err(sensor,code);
}

uint8_t get_sensor_err(eSENSOR_LIST_t sensor)
{
  return   g_kma_err[sensor];
}

bool is_raining(uint8_t  *sensor_err)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  *sensor_err = p_sensor[A8_RAIN_PRESENT].err;
  
  return p_sensor[A8_RAIN_PRESENT].data.b;
}


void MegaErrorCheck(SYSTEM_INFO_AWS *pSystem, uint16_t  *sRetVal, uint16_t  sCompVal, uint16_t  sMax,
                    uint8_t cErrChan)
{
  if (sCompVal >= sMax)
  {
    if (++pSystem->cMegaErrCnt[cErrChan] > 60)
    {
      pSystem->cMegaErrCnt[cErrChan] = 60;
      *sRetVal = sCompVal;
    }
  }
  else
  {
    pSystem->cMegaErrCnt[cErrChan] = 0;
    *sRetVal = sCompVal;
  }
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

  return (uint16_t)(wind_speed * 10);
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

  return (uint16_t)(wind_direction*10);
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

   return (uint16_t)((temperature + 100) * 10);  // AWS 데이터 형으로 변환 ((측정값+100) *10)
}

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

  return (uint16_t)(pressure * 10);  // AWS 데이터 형으로 변환 측정값 *10
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

  return (uint16_t)(huminity * 10);//AWS 데이터 형으로 변환 측정값 *10
}

uint16_t  SolarRadCalc(uint8_t *sensor_err)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  *sensor_err = 0;
  if (p_sensor[B1_SOLAR_RADIATION].err)
  {
    *sensor_err = 1;
    return AWS_DATA_ERR_VAL;
  }

  return (uint16_t)(p_sensor[B1_SOLAR_RADIATION].data.f * 10);
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
  uint16_t sRet;
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

  return (uint16_t)((temperature + 100) * 10);  // AWS 데이터 형으로 변환 ((측정값+100) *10)
}

//일조 
uint8_t SunshineCalc(uint8_t *err)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  *err = p_sensor[B2_SUNSHINE_DURATION].err;
  
  if (p_sensor[B2_SUNSHINE_DURATION].data.b)
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
  uint8_t err;
  sensor_data_t *p_sensor = g_p_raw->data;
  uint16_t rain=0;;


  //우량은 홀센서인경우에만 에러 체크됨
  *sensor_err = (uint16_t)p_sensor[A6_RAINFALL_DOT5_1MM].err;

  
  rain = (uint16_t)(p_sensor[A6_RAINFALL_DOT5_1MM].data.f*10);;

  p_sensor[A6_RAINFALL_DOT5_1MM].data.f = 0;// 우량은 이전값을 초기화해줘야함
  return rain;
}



AWS_DATA_STRUCT *get_aws_data(int min)
{
  AWS_DATA_STRUCT *p_aws_data=NULL;
  switch(min)
  {
    case eAWS_DATA_AVG:
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

void update_old_kma(eAWS_DATA_MIN_t min)
{
  AWS_DATA_STRUCT *p_aws_data;

  p_aws_data = get_aws_data(min);

}




void update_kma_real(void)
{
  kma_data_ex_t *p_kma3;

  p_kma3 = get_kma_data(eAWS_DATA_AVG);

  p_kma3->temperature.data = mRealAws.mTemperature.sReal;
  p_kma3->temperature.err = get_sensor_err(A1_TEMPERATURE);
  p_kma3->temperature.max = mRealAws.mTemperature.sMax;
  p_kma3->temperature.min = mRealAws.mTemperature.sMin;

  p_kma3->wind_direction_avg.data = mRealAws.mWind.mDirection.sReal;
  p_kma3->wind_direction_avg.err = get_sensor_err(A2_WIND_DIRECTION);
  p_kma3->wind_direction_avg.max = mRealAws.mWind.mDirection.sMax;

  p_kma3->wind_speed_avg.data = mRealAws.mWind.mSpeed.sReal;
  p_kma3->wind_speed_avg.err = get_sensor_err(A3_WIND_SPEED);
  p_kma3->wind_speed_avg.max = mRealAws.mWind.mSpeed.sMax;

  p_kma3->wind_speed_instant.data = mRealAws.mWind.mSpeed.sMax;
  p_kma3->wind_speed_instant.err = 0;

  p_kma3->wind_direction_instant.data = mRealAws.mWind.mDirection.sMax;
  p_kma3->wind_direction_instant.err = 0;

  p_kma3->precipitation.data = mRealAws.mRainFall.sReal;
  p_kma3->precipitation.err = get_sensor_err(A6_RAINFALL_DOT5_1MM);

  p_kma3->pressure.data = mRealAws.mBarometric.sReal;
  p_kma3->pressure.err = get_sensor_err(A7_PRESSURE);
  p_kma3->pressure.max = mRealAws.mBarometric.sMax;
  p_kma3->pressure.min = mRealAws.mBarometric.sMin;

  p_kma3->precipitation_presence.data = mRealAws.mRainDetect.sReal;
  p_kma3->precipitation_presence.err = get_sensor_err(A8_RAIN_PRESENT);

  p_kma3->snowfall.data = mRealAws.mSnowFall.sReal;
  p_kma3->snowfall.err = get_sensor_err(A9_SNOW_DEPTH);

  p_kma3->relative_humidity.data = mRealAws.mHumidity.sReal;
  p_kma3->relative_humidity.err = get_sensor_err(A10_RELATIVE_HUMIDITY);
  p_kma3->relative_humidity.max = mRealAws.mHumidity.sMax;
  p_kma3->relative_humidity.min = mRealAws.mHumidity.sMin;

  // 강수량 0.1

  p_kma3->solar_radiation.data = mRealAws.mSolarRad.sReal;
  p_kma3->solar_radiation.err = get_sensor_err(B1_SOLAR_RADIATION);
  p_kma3->solar_radiation.max = mRealAws.mSolarRad.sMax;

  p_kma3->sunshine_duration.data = mRealAws.mSunshine.sReal;
  p_kma3->sunshine_duration.err = get_sensor_err(B2_SUNSHINE_DURATION);
  p_kma3->sunshine_duration.max = mRealAws.mSunshine.sMax;

  p_kma3->soil_temperature_5cm.data = mRealAws.mSoilTemp5cm.sReal;
  p_kma3->soil_temperature_5cm.err = get_sensor_err(B5_SOIL_TEMPERATURE_5CM);

  p_kma3->soil_temperature_10cm.data = mRealAws.mSoilTemp10cm.sReal;
  p_kma3->soil_temperature_10cm.err = get_sensor_err(B6_SOIL_TEMPERATURE_10CM);

  p_kma3->soil_temperature_20cm.data = mRealAws.mSoilTemp20cm.sReal;
  p_kma3->soil_temperature_20cm.err = get_sensor_err(B7_SOIL_TEMPERATURE_20CM);

  p_kma3->soil_temperature_30cm.data = mRealAws.mSoilTemp30cm.sReal;
  p_kma3->soil_temperature_30cm.err = get_sensor_err(B8_SOIL_TEMPERATURE_30CM);
  p_kma3->soil_temperature_50cm.data = mRealAws.mSoilTemp50cm.sReal;
  p_kma3->soil_temperature_50cm.err = get_sensor_err(B9_SOIL_TEMPERATURE_50CM);
  ;

  p_kma3->soil_temperature_1m.data = mRealAws.mSoilTemp1_0m.sReal;
  p_kma3->soil_temperature_1m.err = get_sensor_err(B10_SOIL_TEMPERATURE_100CM);

  p_kma3->soil_temperature_1_5m.data = mRealAws.mSoilTemp1_5m.sReal;
  p_kma3->soil_temperature_1_5m.err = get_sensor_err(B11_SOIL_TEMPERATURE_150CM);

  // 추가됨
  p_kma3->soil_moisture_10cm.data = g_kma_raw_ex.soil_moisture_10cm.data;

  set_rainfall_yesterday(Sysinfo.mRain.sBefDayRain / 10.0);
  set_rainfall_today(Sysinfo.mRain.sDayRain / 10.0);
  set_rainfall_hourly(Sysinfo.mRain.sHourRain / 10.0);

  kma3_update_sensor_status(A1_TEMPERATURE, p_kma3->X_sensorStatus, get_sensor_err(A1_TEMPERATURE));
  kma3_update_sensor_status(A2_WIND_DIRECTION, p_kma3->X_sensorStatus,
                            get_sensor_err(A2_WIND_DIRECTION));
  kma3_update_sensor_status(A3_WIND_SPEED, p_kma3->X_sensorStatus, get_sensor_err(A3_WIND_SPEED));
  kma3_update_sensor_status(A6_RAINFALL_DOT5_1MM, p_kma3->X_sensorStatus,
                            get_sensor_err(A6_RAINFALL_DOT5_1MM));
  kma3_update_sensor_status(A7_PRESSURE, p_kma3->X_sensorStatus, get_sensor_err(A7_PRESSURE));
  kma3_update_sensor_status(A8_RAIN_PRESENT, p_kma3->X_sensorStatus,
                            get_sensor_err(A8_RAIN_PRESENT));
  kma3_update_sensor_status(A9_SNOW_DEPTH, p_kma3->X_sensorStatus, get_sensor_err(A9_SNOW_DEPTH));
  kma3_update_sensor_status(A10_RELATIVE_HUMIDITY, p_kma3->X_sensorStatus,
                            get_sensor_err(A10_RELATIVE_HUMIDITY));
  kma3_update_sensor_status(B1_SOLAR_RADIATION, p_kma3->X_sensorStatus,
                            get_sensor_err(B1_SOLAR_RADIATION));
  kma3_update_sensor_status(B2_SUNSHINE_DURATION, p_kma3->X_sensorStatus,
                            get_sensor_err(B2_SUNSHINE_DURATION));

  kma3_update_sensor_status(B5_SOIL_TEMPERATURE_5CM, p_kma3->X_sensorStatus,
                            get_sensor_err(B5_SOIL_TEMPERATURE_5CM));
  kma3_update_sensor_status(B6_SOIL_TEMPERATURE_10CM, p_kma3->X_sensorStatus,
                            get_sensor_err(B6_SOIL_TEMPERATURE_10CM));
  kma3_update_sensor_status(B7_SOIL_TEMPERATURE_20CM, p_kma3->X_sensorStatus,
                            get_sensor_err(B7_SOIL_TEMPERATURE_20CM));
  kma3_update_sensor_status(B8_SOIL_TEMPERATURE_30CM, p_kma3->X_sensorStatus,
                            get_sensor_err(B8_SOIL_TEMPERATURE_30CM));
  kma3_update_sensor_status(B9_SOIL_TEMPERATURE_50CM, p_kma3->X_sensorStatus,
                            get_sensor_err(B9_SOIL_TEMPERATURE_50CM));
  kma3_update_sensor_status(B10_SOIL_TEMPERATURE_100CM, p_kma3->X_sensorStatus,
                            get_sensor_err(B10_SOIL_TEMPERATURE_100CM));
  kma3_update_sensor_status(B11_SOIL_TEMPERATURE_150CM, p_kma3->X_sensorStatus,
                            get_sensor_err(B11_SOIL_TEMPERATURE_150CM));

  BIT_UPDATE(p_kma3->Y_volateStatus, System.dc_error, KMA2_PWRSTAT_DC_INPUT_ERR);
  BIT_UPDATE(p_kma3->Y_volateStatus, System.battery_error, KMA2_PWRSTAT_BATTERY_ERR);
  p_kma3->Y_volateStatus &= 0xF3;
  p_kma3->Y_volateStatus |=System.ac_status<<2;
  BIT_UPDATE(p_kma3->Y_volateStatus, System.door_opened, KMA2_PWRSTAT_DOOR_OPEN);
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

  for (int min = eAWS_DATA_AVG; min <= eAWS_DATA_RAW; min++)
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

#define MAKE_TEMP(x) (uint16_t)((x + 100) * 10)  // 기온, 지면온도, 지중온도, 초상온도
#define MAKE_RADI(x) (uint16_t)((x + 100) * 10)  // 순복사, 전천복사, 반사복사 등
#define MAKE_PRESSURE(x) (uint16_t)((x) * 10)    // 기압
#define MAKE_X10(x) (uint16_t)((x) * 10)         // 풍속, 풍향, 습도, 토양수분 등
#define MAKE_X100(x) (uint16_t)((x) * 100)       // 일사량, 조도량 등
#define MAKE_DIRECT(x) (uint16_t)(x)             // 운고, 시정, 현재일기, 타코미터 등 정수값

void update_raw(void)
{
  kma_data_ex_t *p_kma_data;

  p_kma_data = get_kma_data((eAWS_DATA_MIN_t)eAWS_DATA_RAW);

  p_kma_data->temperature.data = MAKE_TEMP(g_p_raw->data[A1_TEMPERATURE].data.f);
  p_kma_data->wind_direction_avg.data = MAKE_X10(g_p_raw->data[A2_WIND_DIRECTION].data.f);
  p_kma_data->wind_speed_avg.data = MAKE_X10(g_p_raw->data[A3_WIND_SPEED].data.f);
  p_kma_data->precipitation.data = MAKE_DIRECT(g_p_raw->data[A6_RAINFALL_DOT5_1MM].data.i);
  p_kma_data->pressure.data = MAKE_PRESSURE(g_p_raw->data[A7_PRESSURE].data.f);
  p_kma_data->precipitation_presence.data = MAKE_DIRECT(g_p_raw->data[A8_RAIN_PRESENT].data.i);
  p_kma_data->snowfall.data = MAKE_X10(g_p_raw->data[A9_SNOW_DEPTH].data.f);
  p_kma_data->relative_humidity.data = MAKE_X10(g_p_raw->data[A10_RELATIVE_HUMIDITY].data.f);
  p_kma_data->precipitation_fine.data = MAKE_DIRECT(g_p_raw->data[A11_RAINFALL_DOT1MM].data.i);

  p_kma_data->solar_radiation.data = MAKE_X100(g_p_raw->data[B1_SOLAR_RADIATION].data.f);
  p_kma_data->sunshine_duration.data = MAKE_DIRECT(g_p_raw->data[B2_SUNSHINE_DURATION].data.i);
  p_kma_data->surface_temperature.data = MAKE_TEMP(g_p_raw->data[B3_GROUND_TEMPERATURE].data.f);
  p_kma_data->grass_temperature.data = MAKE_TEMP(g_p_raw->data[B4_SURFACE_TEMPERATURE].data.f);
  p_kma_data->soil_temperature_5cm.data = MAKE_TEMP(g_p_raw->data[B5_SOIL_TEMPERATURE_5CM].data.f);
  p_kma_data->soil_temperature_10cm.data =
      MAKE_TEMP(g_p_raw->data[B6_SOIL_TEMPERATURE_10CM].data.f);
  p_kma_data->soil_temperature_20cm.data =
      MAKE_TEMP(g_p_raw->data[B7_SOIL_TEMPERATURE_20CM].data.f);
  p_kma_data->soil_temperature_30cm.data =
      MAKE_TEMP(g_p_raw->data[B8_SOIL_TEMPERATURE_30CM].data.f);
  p_kma_data->soil_temperature_50cm.data =
      MAKE_TEMP(g_p_raw->data[B9_SOIL_TEMPERATURE_50CM].data.f);
  p_kma_data->soil_temperature_1m.data =
      MAKE_TEMP(g_p_raw->data[B10_SOIL_TEMPERATURE_100CM].data.f);
  p_kma_data->soil_temperature_1_5m.data =
      MAKE_TEMP(g_p_raw->data[B11_SOIL_TEMPERATURE_150CM].data.f);
  p_kma_data->soil_temperature_3m.data =
      MAKE_TEMP(g_p_raw->data[B12_SOIL_TEMPERATURE_300CM].data.f);
  p_kma_data->soil_temperature_5m.data =
      MAKE_TEMP(g_p_raw->data[B13_SOIL_TEMPERATURE_500CM].data.f);

  p_kma_data->cloud_height_1st.data = MAKE_DIRECT(g_p_raw->data[C1_CLOUD_BASE1].data.f);
  p_kma_data->cloud_height_2nd.data = MAKE_DIRECT(g_p_raw->data[C2_CLOUD_BASE2].data.f);
  p_kma_data->cloud_height_3rd.data = MAKE_DIRECT(g_p_raw->data[C3_CLOUD_BASE3].data.f);
  p_kma_data->cloud_amount.data = MAKE_DIRECT(g_p_raw->data[C4_CLOUD_COVER].data.f);
  p_kma_data->visibility.data = MAKE_DIRECT(g_p_raw->data[C5_VISIBILITY].data.f);
  p_kma_data->pm10_concentration.data = MAKE_X10(g_p_raw->data[C6_PM10].data.f);
  p_kma_data->pm25_concentration.data = MAKE_X10(g_p_raw->data[C7_PM2DOT5].data.f);
  p_kma_data->net_radiation.data = MAKE_RADI(g_p_raw->data[C8_NET_RADIATION].data.f);
  p_kma_data->total_radiation.data = MAKE_RADI(g_p_raw->data[C9_TOTAL_RADIATION].data.f);
  p_kma_data->reflected_radiation.data = MAKE_RADI(g_p_raw->data[C10_REFLECTED_RADIATION].data.f);
  p_kma_data->direct_radiation.data = MAKE_RADI(g_p_raw->data[C11_DIRECT_SOLAR].data.f);
  p_kma_data->current_weather.data = MAKE_DIRECT(g_p_raw->data[C12_CURRENT_WEATHER].data.i);

  p_kma_data->soil_moisture_10cm.data = MAKE_X10(g_p_raw->data[N1_SOIL_MOISTURE_10CM].data.f);
  p_kma_data->soil_moisture_20cm.data = MAKE_X10(g_p_raw->data[N2_SOIL_MOISTURE_20CM].data.f);
  p_kma_data->soil_moisture_30cm.data = MAKE_X10(g_p_raw->data[N3_SOIL_MOISTURE_30CM].data.f);
  p_kma_data->soil_moisture_50cm.data = MAKE_X10(g_p_raw->data[N4_SOIL_MOISTURE_50CM].data.f);
  p_kma_data->illuminance.data = MAKE_X100(g_p_raw->data[N5_ILLUMINANCE].data.f);
  p_kma_data->wind_speed_1_5m.data = MAKE_X10(g_p_raw->data[N6_WIND_VELOCITY_150CM].data.f);
  p_kma_data->wind_speed_4m.data = MAKE_X10(g_p_raw->data[N7_WIND_VELOCITY_400CM].data.f);
  p_kma_data->instant_wind_speed_1_5m.data =
      MAKE_X10(g_p_raw->data[N8_INSTANT_VELOCITY_150CM].data.f);
  p_kma_data->instant_wind_speed_4m.data =
      MAKE_X10(g_p_raw->data[N9_INSTANT_VELOCITY_400CM].data.f);
  p_kma_data->temperature_0_5m.data = MAKE_TEMP(g_p_raw->data[N10_AIR_TEMPERATURE_50CM].data.f);
  p_kma_data->temperature_4m.data = MAKE_TEMP(g_p_raw->data[N11_AIR_TEMPERATURE_400CM].data.f);
  p_kma_data->humidity_0_5m.data = MAKE_X10(g_p_raw->data[N12_HUMIDITY_50CM].data.f);
  p_kma_data->humidity_4m.data = MAKE_X10(g_p_raw->data[N13_HUMIDITY_400CM].data.f);

  p_kma_data->tacometer.data = MAKE_DIRECT(g_p_raw->data[I1_TACHOMETER].data.f);

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


void DUALPORT_TASK(void *arg)
{
  uint8_t sensor_err = 0;
  uint16_t sTriger = 0;
  uint16_t sSpeed;
  uint16_t sDirec;
  uint16_t sSpeedOld=0;
  uint16_t sDirecOld=0;
  SYSTEM_INFO_AWS *pSystem;
  DATE_TIME_BUF ct;
  DATE_TIME_BUF time_old;
  AWS_DATA_STRUCT *pAws;


  int i, j;
  int nWindCnt12 = 0;
  int nWindCnt40 = 0;


  pAws = &mRealAws;
  pSystem = &Sysinfo;

  // 여기서는 메모리를 아끼기위해 g_p_raw 하나만 사용
  g_p_raw = aws_malloc(sizeof(measure_data_1s_t));

        time_old = Date_Time;
        while (1)
        {
          is_measurement_1s(g_p_raw, 0);                        // 업데이트된 값 없으면 이전값 유지
          if (is_measurement_250(&g_raw_250, osWaitForever))  // 250ms마다 최신값 사용
          {
            g_p_raw->data[A2_WIND_DIRECTION] = g_raw_250.data[eA2_WIND_DIRECTION];
            g_p_raw->data[A3_WIND_SPEED] = g_raw_250.data[eA3_WIND_SPEED];
          }

          ct = Date_Time;
          
          check_sensor_use();
          update_raw();

          pSystem->mRain.sDayCount += get_rain_mm(&sensor_err);
          update_sensor_err(A6_RAINFALL_DOT5_1MM, sensor_err);

          sSpeed = sSpeedOld;
          sDirec = sDirecOld;

          MegaErrorCheck(pSystem, &sSpeed, WindSpeedCalc(&sensor_err), 9999, MEGASPEED_ERR_CHAN);
          update_sensor_err(A3_WIND_SPEED, sensor_err);

          MegaErrorCheck(pSystem, &sDirec, WindDirecCalc(&sensor_err), 9999, MEGADIREC_ERR_CHAN);
          update_sensor_err(A2_WIND_DIRECTION, sensor_err);

          sSpeedOld = sSpeed;
          sDirecOld = sDirec;

          pSystem->mRealWind.sAvg3Speed[nWindCnt12] = sSpeed;   // 풍속  3 초 평균
          pSystem->mRealWind.sAvg10Speed[nWindCnt40] = sSpeed;  // 풍속 10 초 평균

          pSystem->mRealWind.sAvg3Direction[nWindCnt12] = sDirec;   // 풍향  3 초 평균
          pSystem->mRealWind.sAvg10Direction[nWindCnt40] = sDirec;  // 풍향 10 초 평균
          pSystem->mRealWind.sWrFlag[nWindCnt40] = 1;

          if (++nWindCnt12 >= 12)
            nWindCnt12 = 0;
          if (++nWindCnt40 >= 40)
            nWindCnt40 = 0;

          MegaErrorCheck(pSystem, &pAws->mTemperature.sReal, TempCalc(&sensor_err), 9999,
                         MEGATEMP_ERR_CHAN);  // 1초 순간 온도
          update_sensor_err(A1_TEMPERATURE, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mBarometric.sReal, BarometricCalc(&sensor_err), 19999,
                         MEGABARO_ERR_CHAN);  // 1초 순간 기압
          update_sensor_err(A7_PRESSURE, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mHumidity.sReal, HumidityCalc(&sensor_err), 9999,
                         MEGAHUMID_ERR_CHAN);  // 1초 순간 습도
          update_sensor_err(A10_RELATIVE_HUMIDITY, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mSolarRad.sReal, SolarRadCalc(&sensor_err), 9999,
                         MEGASOL_ERR_CHAN);  // 일사
          update_sensor_err(B1_SOLAR_RADIATION, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mSnowFall.sReal, SnowCalc(&sensor_err), 9999,
                         MEGASNOW_ERR_CHAN);  // 현재 적설량
          update_sensor_err(A9_SNOW_DEPTH, sensor_err);

          // 추가 2017. 03. 22 지중 온도 추가  //
          MegaErrorCheck(pSystem, &pAws->mSoilTemp5cm.sReal,
                         TempCalcExt(SOLITEMP5CM_CHN, &sensor_err), 9999,
                         MEGASOLI5TEMP_ERR_CHAN);  // 1초 순간 지중 5Cm온도

          update_sensor_err(B5_SOIL_TEMPERATURE_5CM, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mSoilTemp10cm.sReal,
                         TempCalcExt(SOLITEMP10CM_CHN, &sensor_err), 9999,
                         MEGASOLI10TEMP_ERR_CHAN);  // 1초 순간 지중 10Cm온도

          update_sensor_err(B6_SOIL_TEMPERATURE_10CM, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mSoilTemp20cm.sReal,
                         TempCalcExt(SOLITEMP20CM_CHN, &sensor_err), 9999,
                         MEGASOLI20TEMP_ERR_CHAN);  // 1초 순간 지중 20Cm온도

          update_sensor_err(B7_SOIL_TEMPERATURE_20CM, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mSoilTemp30cm.sReal,
                         TempCalcExt(SOLITEMP30CM_CHN, &sensor_err), 9999,
                         MEGASOLI30TEMP_ERR_CHAN);  // 1초 순간 지중 30Cm온도

          update_sensor_err(B8_SOIL_TEMPERATURE_30CM, sensor_err);

          // 추가 2017. 03. 22 지중 온도 추가 END //
          MegaErrorCheck(pSystem, &pAws->mSoilTemp50cm.sReal,
                         TempCalcExt(SOLITEMP50CM_CHN, &sensor_err), 9999,
                         MEGASOLI10TEMP_ERR_CHAN);  // 1초 순간 지중 50Cm온도

          update_sensor_err(B9_SOIL_TEMPERATURE_50CM, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mSoilTemp1_0m.sReal,
                         TempCalcExt(SOLITEMP1_0M_CHN, &sensor_err), 9999,
                         MEGASOLI20TEMP_ERR_CHAN);  // 1초 순간 지중 1~0m온도

          update_sensor_err(B10_SOIL_TEMPERATURE_100CM, sensor_err);

          MegaErrorCheck(pSystem, &pAws->mSoilTemp1_5m.sReal,
                         TempCalcExt(SOLITEMP1_5M_CHN, &sensor_err), 9999,
                         MEGASOLI30TEMP_ERR_CHAN);  // 1초 순간 지중 1_5m온도
                                                    // 추가 2017. 03. 22 지중 온도 추가 END //

          update_sensor_err(B11_SOIL_TEMPERATURE_150CM, sensor_err);

          // Off Delay 적용 함
          if (is_raining(&sensor_err))  // 강우 감지
          {
            update_sensor_err(A8_RAIN_PRESENT, sensor_err);

            pAws->mRainDetect.sReal = 0x000a;
            pSystem->m_shOffDelayRemain = get_config_app()->m_usRainDtOffDelay;
            pSystem->m_cOffDelayFlag = 1;
          }

          pAws->mSunshine.sReal = SunshineCalc(&sensor_err);  // CSD3 기준 0-1V 신호로 발생됨

          update_sensor_err(B2_SUNSHINE_DURATION, sensor_err);

          // 센서 불량 처리
          pAws->mStatus.sReal = 0;

          if (kma_is_sensor_error(A6_RAINFALL_DOT5_1MM))
            pAws->mStatus.sMin |= RAINFALLFAIL_BIT;
          else
            pAws->mStatus.sMin &= ~(RAINFALLFAIL_BIT);

          if (sSpeed >= 1050)
            pAws->mStatus.sMin |= WINDSPEEDFAIL_BIT;
          else
            pAws->mStatus.sMin &= ~(WINDSPEEDFAIL_BIT);

          if (sDirec >= 3600)
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

          update_kma_real();
          //현재 값연산 없는 항목은 원본값으로 처리
          update_unused_data(get_kma_data(eAWS_DATA_AVG),get_kma_data(eAWS_DATA_RAW));//
          update_unused_data(get_kma_data(eAWS_DATA_1MIN),get_kma_data(eAWS_DATA_RAW));//
        }
}

const osThreadAttr_t KdualportTask_attributes = {
    .name = "DUALPORT_TASK",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityRealtime1,
};

extern void aws_data_task(void *arg) ;



void old_aws_init(void)
{
  SYSTEM_INFO_AWS *pSystem;

  pSystem = &Sysinfo;

  pSystem->mNVram.nYearRain = (uint32_t)(get_config_nvm()->rainfall_yearly*10);
  pSystem->mNVram.nMonthRain = (uint32_t)(get_config_nvm()->rainfall_monthly*10);
  pSystem->mNVram.nYearSunshine = (uint32_t)(get_config_nvm()->sunshine_yearly);
  pSystem->mNVram.nMonthSunshine = (uint32_t)(get_config_nvm()->sunshine_monthly);

  set_rainfall_monthly(pSystem->mNVram.nMonthRain / 10.0);
  set_rainfall_yearly(pSystem->mNVram.nYearRain / 10.0);
}


void dualportTask_init(void)
{
  AwsMinMaxInit();
  old_aws_init();
  
  osThreadNew(DUALPORT_TASK, NULL, &KdualportTask_attributes);
  //osThreadNew(aws_data_task, NULL, &dualportTask_attributes);
}

