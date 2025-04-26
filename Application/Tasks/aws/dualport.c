#include <stdbool.h>

#include "cmsis_os2.h"

#include  "old_aws_define.h"
#include "schedule.h"

#include "utile_time.h"

#include "app_sensor.h"
#include "task_measure.h"
#include "schedule.h"
#include "user_heap.h"
#include "aws_data.h"
#include "utile_filter.h"
#include "aws_kma3.h"
#define AWS_DATA_ERR_VAL 9999
measure_data_t *g_p_raw = NULL;




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
  if (p_sensor[A3_WIND_SPEED].err)
  {
    *sensor_err = 1;
    return AWS_DATA_ERR_VAL;
  }

  wind_speed = p_sensor[A3_WIND_SPEED].data.f;


  if (wind_speed < 10.0f)  // 10m/s 미만이면 0.5m/s 정확도 가져야함함
  {
    wind_speed = validate_sensor_value_min(wind_speed, 0.0f, WIND_SPEED_ACCURACY_LT_10MPS, &err);

    if (err)
    {
      *sensor_err = 1;
      return AWS_DATA_ERR_VAL;
    }

  }
  else //10m/s 이상이면  측정값의 5%
  {
    accuracy = wind_speed * WIND_SPEED_ACCURACY_GE_10MPS;

    wind_speed = validate_sensor_value_max(wind_speed, 75.0f, accuracy, &err);

    if (err)
    {
      *sensor_err = 1;
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
  if (p_sensor[A2_WIND_DIRECTION].err)
  {
    return 9999;
  }
  *sensor_err=0;
  
  wind_direction = p_sensor[A2_WIND_DIRECTION].data.f;
  *sensor_err |= p_sensor[A2_WIND_DIRECTION].err;

  wind_direction = validate_sensor_value_min(wind_direction, 0, WIND_DIRECTION_ACCURACY, &err);

  if(err)
  {
    *sensor_err |= err;
    return AWS_DATA_ERR_VAL;
  }

  wind_direction = validate_sensor_value_max(wind_direction, 359.99, WIND_DIRECTION_ACCURACY, &err);

  if (err)
  {
    *sensor_err |= err;
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
   if (p_sensor[A1_TEMPERATURE].err)
   {
    *sensor_err = 1;
     return AWS_DATA_ERR_VAL;
   }

   temperature = p_sensor[A1_TEMPERATURE].data.f;

   temperature = validate_sensor_value_min(temperature, -40.0f, TEMPERATURE_ACCURACY, &err);

   if (err)
   {
     *sensor_err = err;
     return AWS_DATA_ERR_VAL;
   }

   temperature = validate_sensor_value_max(temperature, 60.0f, TEMPERATURE_ACCURACY, &err);

   if (err)
   {
     *sensor_err = err;
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
  if (p_sensor[A7_PRESSURE].err)
  {
    *sensor_err = 1;
    return AWS_DATA_ERR_VAL;
  }

  pressure = p_sensor[A7_PRESSURE].data.f;

  pressure = validate_sensor_value_min(pressure, 500.0f, PRESSURE_ACCURACY, &err);

  if (err)
  {
    *sensor_err = err;
    return AWS_DATA_ERR_VAL;
  }

  pressure = validate_sensor_value_max(pressure, 1080.0f, PRESSURE_ACCURACY, &err);

  if (err)
  {
    *sensor_err = err;
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

  if (p_sensor[A10_RELATIVE_HUMIDITY].err)
  {
    *sensor_err = 1;
     return AWS_DATA_ERR_VAL;
  }

  huminity = p_sensor[A10_RELATIVE_HUMIDITY].data.f;

  huminity = validate_sensor_value_min(huminity, 0.0f, HUMINITY_0_90_ACCURACY, &err);

  if(err)
  {
    *sensor_err = 1;
    return AWS_DATA_ERR_VAL;
  }

  huminity = validate_sensor_value_max(huminity, 100.0f, HUMINITY_0_90_ACCURACY, &err);

  if(err)
  {
    *sensor_err = 1;
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

  *sensor_err = 0;
  if (p_sensor[A9_SNOW_DEPTH].err)
  {
      *sensor_err = 1;
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

   *sensor_err = 0;

  switch (ch)
  {
    case SOLITEMP5CM_CHN:
      if (p_sensor[B5_SOIL_TEMPERATURE_5CM].err)
      {
        *sensor_err = 1;
        return AWS_DATA_ERR_VAL;
      }
      sRet = p_sensor[B5_SOIL_TEMPERATURE_5CM].data.i * 10;
      break;
    case SOLITEMP10CM_CHN:
      if (p_sensor[B6_SOIL_TEMPERATURE_10CM].err)
      {
        *sensor_err = 1;
        return AWS_DATA_ERR_VAL;
      }
      sRet = p_sensor[B6_SOIL_TEMPERATURE_10CM].data.i * 10;
      break;
    case SOLITEMP20CM_CHN:
      if (p_sensor[B7_SOIL_TEMPERATURE_20CM].err)
      {
        *sensor_err = 1;
        return AWS_DATA_ERR_VAL;
      }
      sRet = p_sensor[B7_SOIL_TEMPERATURE_20CM].data.i * 10;
      break;
    case SOLITEMP30CM_CHN:
      if (p_sensor[B8_SOIL_TEMPERATURE_30CM].err)
      {
        *sensor_err = 1;
        return AWS_DATA_ERR_VAL;
      }
      sRet = p_sensor[B8_SOIL_TEMPERATURE_30CM].data.i * 10;
      break;
    case SOLITEMP50CM_CHN:
      if (p_sensor[B9_SOIL_TEMPERATURE_50CM].err)
      {
        *sensor_err = 1;
        return AWS_DATA_ERR_VAL;
      }
      sRet = p_sensor[B9_SOIL_TEMPERATURE_50CM].data.i * 10;
      break;
    case SOLITEMP1_0M_CHN:
      if (p_sensor[B10_SOIL_TEMPERATURE_100CM].err)
      {
        *sensor_err = 1;
        return AWS_DATA_ERR_VAL;
      }
      sRet = p_sensor[B10_SOIL_TEMPERATURE_100CM].data.i * 10;
      break;
    case SOLITEMP1_5M_CHN:
      if (p_sensor[B11_SOIL_TEMPERATURE_150CM].err)
      {
        *sensor_err = 1;
        return AWS_DATA_ERR_VAL;
      }
      sRet = p_sensor[B11_SOIL_TEMPERATURE_150CM].data.i * 10;
      break;
      break;
  }

  return sRet;
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
  uint16_t rain_pulse;
  sensor_data_t *p_sensor = g_p_raw->data;

  *sensor_err = (uint16_t)p_sensor[A6_RAINFALL_DOT5_1MM].err;

  rain_pulse = (uint16_t)p_sensor[A6_RAINFALL_DOT5_1MM].data.i;

  return rain_pulse;
}

typedef enum aws_data_min_s
{
  eAWS_DATA_REAL,
  eAWS_DATA_1MIN,
  eAWS_DATA_10MIN,
  eAWS_DATA_HOUR
}eAWS_DATA_MIN_t;

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

void update_old_kma(eAWS_DATA_MIN_t min)
{
  AWS_DATA_STRUCT *p_aws_data;

  p_aws_data = get_aws_data(min);

}

void update_old_kma_real(void)
{
  g_kma_inst_ex.temperature.data = mRealAws.mTemperature.sReal;
  g_kma_inst_ex.temperature.err = g_kma_inst_ex.temperature.data == 9999 ? 1 : 0;
  g_kma_inst_ex.temperature.max = mRealAws.mTemperature.sMax;
  g_kma_inst_ex.temperature.min = mRealAws.mTemperature.sMin;

  g_kma_inst_ex.relative_humidity.data = mRealAws.mHumidity.sReal;
  g_kma_inst_ex.relative_humidity.err = g_kma_inst_ex.relative_humidity.data == 9999 ? 1 : 0;
  g_kma_inst_ex.relative_humidity.max = mRealAws.mHumidity.sMax;
  g_kma_inst_ex.relative_humidity.min = mRealAws.mHumidity.sMin;

  g_kma_inst_ex.wind_speed_avg.data = mRealAws.mWind.mSpeed.sReal;
  g_kma_inst_ex.wind_speed_avg.err = g_kma_inst_ex.wind_speed_avg.data == 9999 ? 1 : 0;
  g_kma_inst_ex.wind_speed_avg.max = mRealAws.mWind.mSpeed.sMax;

  g_kma_inst_ex.wind_direction_avg.data = mRealAws.mWind.mDirection.sReal;
  g_kma_inst_ex.wind_direction_avg.err = g_kma_inst_ex.wind_direction_avg.data == 9999 ? 1 : 0;
  g_kma_inst_ex.wind_direction_avg.max = mRealAws.mWind.mDirection.sMax;

  g_kma_inst_ex.wind_speed_instant.data = mRealAws.mWind.mSpeed.sMax;
  g_kma_inst_ex.wind_speed_instant.err = g_kma_inst_ex.wind_speed_instant.data == 9999 ? 1 : 0;

  g_kma_inst_ex.wind_direction_instant.data = mRealAws.mWind.mDirection.sMax;
  g_kma_inst_ex.wind_direction_instant.err =
      g_kma_inst_ex.wind_direction_instant.data == 9999 ? 1 : 0;

  g_kma_inst_ex.sunshine_duration.data = mRealAws.mSunshine.sReal;
  g_kma_inst_ex.sunshine_duration.err = g_kma_inst_ex.sunshine_duration.data == 9999 ? 1 : 0;
  g_kma_inst_ex.sunshine_duration.max = mRealAws.mSunshine.sMax;

  g_kma_inst_ex.solar_radiation.data = mRealAws.mSolarRad.sReal;
  g_kma_inst_ex.solar_radiation.err = g_kma_inst_ex.solar_radiation.data == 9999 ? 1 : 0;
  g_kma_inst_ex.solar_radiation.max = mRealAws.mSolarRad.sMax;

  g_kma_inst_ex.precipitation_presence.data = mRealAws.mRainDetect.sReal;
  g_kma_inst_ex.precipitation_presence.err =
      g_kma_inst_ex.precipitation_presence.data == 9999 ? 1 : 0;

  g_kma_inst_ex.snowfall.data = mRealAws.mSnowFall.sReal;
  g_kma_inst_ex.snowfall.err = g_kma_inst_ex.snowfall.data == 9999 ? 1 : 0;

  g_kma_inst_ex.pressure.data = mRealAws.mBarometric.sReal;
  g_kma_inst_ex.pressure.err = g_kma_inst_ex.pressure.data == 9999 ? 1 : 0;
  g_kma_inst_ex.pressure.max = mRealAws.mBarometric.sMax;
  g_kma_inst_ex.pressure.min = mRealAws.mBarometric.sMin;

  g_kma_inst_ex.soil_temperature_5cm.data = mRealAws.mSoilTemp5cm.sReal;
  g_kma_inst_ex.soil_temperature_5cm.err = g_kma_inst_ex.soil_temperature_5cm.data == 9999 ? 1 : 0;

  g_kma_inst_ex.soil_temperature_10cm.data = mRealAws.mSoilTemp10cm.sReal;
  g_kma_inst_ex.soil_temperature_10cm.err =
      g_kma_inst_ex.soil_temperature_10cm.data == 9999 ? 1 : 0;

  g_kma_inst_ex.soil_temperature_20cm.data = mRealAws.mSoilTemp20cm.sReal;
  g_kma_inst_ex.soil_temperature_20cm.err =
      g_kma_inst_ex.soil_temperature_20cm.data == 9999 ? 1 : 0;

  g_kma_inst_ex.soil_temperature_30cm.data = mRealAws.mSoilTemp30cm.sReal;
  g_kma_inst_ex.soil_temperature_30cm.err =
      g_kma_inst_ex.soil_temperature_30cm.data == 9999 ? 1 : 0;

  g_kma_inst_ex.soil_temperature_50cm.data = mRealAws.mSoilTemp50cm.sReal;
  g_kma_inst_ex.soil_temperature_50cm.err =
      g_kma_inst_ex.soil_temperature_50cm.data == 9999 ? 1 : 0;

  g_kma_inst_ex.soil_temperature_1m.data = mRealAws.mSoilTemp1_0m.sReal;
  g_kma_inst_ex.soil_temperature_1m.err = g_kma_inst_ex.soil_temperature_1m.data == 9999 ? 1 : 0;

  g_kma_inst_ex.soil_temperature_1_5m.data = mRealAws.mSoilTemp1_5m.sReal;
  g_kma_inst_ex.soil_temperature_1_5m.err =
      g_kma_inst_ex.soil_temperature_1_5m.data == 9999 ? 1 : 0;
}

void update_old_kma_10min(void)
{
  g_kma_10min_ex.temperature.data = m10MinAws.mTemperature.sReal;
  g_kma_10min_ex.temperature.err = g_kma_10min_ex.temperature.data == 9999 ? 1 : 0;
  g_kma_10min_ex.temperature.max = m10MinAws.mTemperature.sMax;
  g_kma_10min_ex.temperature.min = m10MinAws.mTemperature.sMin;

  g_kma_10min_ex.relative_humidity.data = m10MinAws.mHumidity.sReal;
  g_kma_10min_ex.relative_humidity.err = g_kma_10min_ex.relative_humidity.data == 9999 ? 1 : 0;
  g_kma_10min_ex.relative_humidity.max = m10MinAws.mHumidity.sMax;
  g_kma_10min_ex.relative_humidity.min = m10MinAws.mHumidity.sMin;

  g_kma_10min_ex.wind_speed_avg.data = m10MinAws.mWind.mSpeed.sReal;
  g_kma_10min_ex.wind_speed_avg.err = g_kma_10min_ex.wind_speed_avg.data == 9999 ? 1 : 0;
  g_kma_10min_ex.wind_speed_avg.max = m10MinAws.mWind.mSpeed.sMax;

  g_kma_10min_ex.wind_direction_avg.data = m10MinAws.mWind.mDirection.sReal;
  g_kma_10min_ex.wind_direction_avg.err = g_kma_10min_ex.wind_direction_avg.data == 9999 ? 1 : 0;
  g_kma_10min_ex.wind_direction_avg.max = m10MinAws.mWind.mDirection.sMax;

  g_kma_10min_ex.wind_speed_instant.data = m10MinAws.mWind.mSpeed.sMax;
  g_kma_10min_ex.wind_speed_instant.err = g_kma_10min_ex.wind_speed_instant.data == 9999 ? 1 : 0;

  g_kma_10min_ex.wind_direction_instant.data = m10MinAws.mWind.mDirection.sMax;
  g_kma_10min_ex.wind_direction_instant.err =
      g_kma_10min_ex.wind_direction_instant.data == 9999 ? 1 : 0;

  g_kma_10min_ex.sunshine_duration.data = m10MinAws.mSunshine.sReal;
  g_kma_10min_ex.sunshine_duration.err = g_kma_10min_ex.sunshine_duration.data == 9999 ? 1 : 0;
  g_kma_10min_ex.sunshine_duration.max = m10MinAws.mSunshine.sMax;

  g_kma_10min_ex.solar_radiation.data = m10MinAws.mSolarRad.sReal;
  g_kma_10min_ex.solar_radiation.err = g_kma_10min_ex.solar_radiation.data == 9999 ? 1 : 0;
  g_kma_10min_ex.solar_radiation.max = m10MinAws.mSolarRad.sMax;

  g_kma_10min_ex.precipitation_presence.data = m10MinAws.mRainDetect.sReal;
  g_kma_10min_ex.precipitation_presence.err =
      g_kma_10min_ex.precipitation_presence.data == 9999 ? 1 : 0;

  g_kma_10min_ex.snowfall.data = m10MinAws.mSnowFall.sReal;
  g_kma_10min_ex.snowfall.err = g_kma_10min_ex.snowfall.data == 9999 ? 1 : 0;

  g_kma_10min_ex.pressure.data = m10MinAws.mBarometric.sReal;
  g_kma_10min_ex.pressure.err = g_kma_10min_ex.pressure.data == 9999 ? 1 : 0;
  g_kma_10min_ex.pressure.max = m10MinAws.mBarometric.sMax;
  g_kma_10min_ex.pressure.min = m10MinAws.mBarometric.sMin;

  g_kma_10min_ex.soil_temperature_5cm.data = m10MinAws.mSoilTemp5cm.sReal;
  g_kma_10min_ex.soil_temperature_5cm.err =
      g_kma_10min_ex.soil_temperature_5cm.data == 9999 ? 1 : 0;

  g_kma_10min_ex.soil_temperature_10cm.data = m10MinAws.mSoilTemp10cm.sReal;
  g_kma_10min_ex.soil_temperature_10cm.err =
      g_kma_10min_ex.soil_temperature_10cm.data == 9999 ? 1 : 0;

  g_kma_10min_ex.soil_temperature_20cm.data = m10MinAws.mSoilTemp20cm.sReal;
  g_kma_10min_ex.soil_temperature_20cm.err =
      g_kma_10min_ex.soil_temperature_20cm.data == 9999 ? 1 : 0;

  g_kma_10min_ex.soil_temperature_30cm.data = m10MinAws.mSoilTemp30cm.sReal;
  g_kma_10min_ex.soil_temperature_30cm.err =
      g_kma_10min_ex.soil_temperature_30cm.data == 9999 ? 1 : 0;

  g_kma_10min_ex.soil_temperature_50cm.data = m10MinAws.mSoilTemp50cm.sReal;
  g_kma_10min_ex.soil_temperature_50cm.err =
      g_kma_10min_ex.soil_temperature_50cm.data == 9999 ? 1 : 0;

  g_kma_10min_ex.soil_temperature_1m.data = m10MinAws.mSoilTemp1_0m.sReal;
  g_kma_10min_ex.soil_temperature_1m.err = g_kma_10min_ex.soil_temperature_1m.data == 9999 ? 1 : 0;

  g_kma_10min_ex.soil_temperature_1_5m.data = m10MinAws.mSoilTemp1_5m.sReal;
  g_kma_10min_ex.soil_temperature_1_5m.err =
      g_kma_10min_ex.soil_temperature_1_5m.data == 9999 ? 1 : 0;
}


  void check_sensor_use(void)
  {
    g_kma_inst_ex.temperature.enable = g_p_raw->data[A1_TEMPERATURE].enable;
    g_kma_inst_ex.wind_direction_avg.enable = g_p_raw->data[A2_WIND_DIRECTION].enable;
    g_kma_inst_ex.wind_speed_avg.enable = g_p_raw->data[A3_WIND_SPEED].enable;

    g_kma_inst_ex.precipitation.enable = g_p_raw->data[A6_RAINFALL_DOT5_1MM].enable;
    g_kma_inst_ex.pressure.enable = g_p_raw->data[A7_PRESSURE].enable;
    g_kma_inst_ex.precipitation_presence.enable = g_p_raw->data[A8_RAIN_PRESENT].enable;
    g_kma_inst_ex.snowfall.enable = g_p_raw->data[A9_SNOW_DEPTH].enable;
    g_kma_inst_ex.relative_humidity.enable = g_p_raw->data[A10_RELATIVE_HUMIDITY].enable;
    g_kma_inst_ex.precipitation_fine.enable = g_p_raw->data[A11_RAINFALL_DOT1MM].enable;
    g_kma_inst_ex.solar_radiation.enable = g_p_raw->data[B1_SOLAR_RADIATION].enable;
    g_kma_inst_ex.sunshine_duration.enable = g_p_raw->data[B2_SUNSHINE_DURATION].enable;
    g_kma_inst_ex.surface_temperature.enable = g_p_raw->data[B3_GROUND_TEMPERATURE].enable;
    g_kma_inst_ex.grass_temperature.enable = g_p_raw->data[B4_SURFACE_TEMPERATURE].enable;
    g_kma_inst_ex.soil_temperature_5cm.enable = g_p_raw->data[B5_SOIL_TEMPERATURE_5CM].enable;
    g_kma_inst_ex.soil_temperature_10cm.enable = g_p_raw->data[B6_SOIL_TEMPERATURE_10CM].enable;
    g_kma_inst_ex.soil_temperature_20cm.enable = g_p_raw->data[B7_SOIL_TEMPERATURE_20CM].enable;
    g_kma_inst_ex.soil_temperature_30cm.enable = g_p_raw->data[B8_SOIL_TEMPERATURE_30CM].enable;
    g_kma_inst_ex.soil_temperature_50cm.enable = g_p_raw->data[B9_SOIL_TEMPERATURE_50CM].enable;
    g_kma_inst_ex.soil_temperature_1m.enable = g_p_raw->data[B10_SOIL_TEMPERATURE_100CM].enable;
    g_kma_inst_ex.soil_temperature_1_5m.enable = g_p_raw->data[B11_SOIL_TEMPERATURE_150CM].enable;
    g_kma_inst_ex.soil_temperature_3m.enable = g_p_raw->data[B12_SOIL_TEMPERATURE_300CM].enable;
    g_kma_inst_ex.soil_temperature_5m.enable = g_p_raw->data[B13_SOIL_TEMPERATURE_500CM].enable;
    g_kma_inst_ex.cloud_height_1st.enable = g_p_raw->data[C1_CLOUD_BASE1].enable;
    g_kma_inst_ex.cloud_height_2nd.enable = g_p_raw->data[C2_CLOUD_BASE2].enable;
    g_kma_inst_ex.cloud_height_3rd.enable = g_p_raw->data[C3_CLOUD_BASE3].enable;
    g_kma_inst_ex.cloud_amount.enable = g_p_raw->data[C4_CLOUD_COVER].enable;
    g_kma_inst_ex.visibility.enable = g_p_raw->data[C5_VISIBILITY].enable;

    g_kma_inst_ex.pm10_concentration.enable = g_p_raw->data[C6_PM10].enable;
    g_kma_inst_ex.pm25_concentration.enable = g_p_raw->data[C7_PM2DOT5].enable;
    g_kma_inst_ex.net_radiation.enable = g_p_raw->data[C8_NET_RADIATION].enable;
    g_kma_inst_ex.total_radiation.enable = g_p_raw->data[C9_TOTAL_RADIATION].enable;
    g_kma_inst_ex.reflected_radiation.enable = g_p_raw->data[C10_REFLECTED_RADIATION].enable;
    g_kma_inst_ex.direct_radiation.enable = g_p_raw->data[C11_DIRECT_SOLAR].enable;
    g_kma_inst_ex.current_weather.enable = g_p_raw->data[C12_CURRENT_WEATHER].enable;
    g_kma_inst_ex.soil_moisture_10cm.enable = g_p_raw->data[N1_SOIL_MOISTURE_10CM].enable;
    g_kma_inst_ex.soil_moisture_20cm.enable = g_p_raw->data[N2_SOIL_MOISTURE_20CM].enable;
    g_kma_inst_ex.soil_moisture_30cm.enable = g_p_raw->data[N3_SOIL_MOISTURE_30CM].enable;
    g_kma_inst_ex.soil_moisture_50cm.enable = g_p_raw->data[N4_SOIL_MOISTURE_50CM].enable;
    g_kma_inst_ex.illuminance.enable = g_p_raw->data[N5_ILLUMINANCE].enable;
    g_kma_inst_ex.wind_speed_1_5m.enable = g_p_raw->data[N6_WIND_VELOCITY_150CM].enable;
    g_kma_inst_ex.wind_speed_4m.enable = g_p_raw->data[N7_WIND_VELOCITY_400CM].enable;
    g_kma_inst_ex.instant_wind_speed_1_5m.enable = g_p_raw->data[N8_INSTANT_VELOCITY_150CM].enable;
    g_kma_inst_ex.instant_wind_speed_4m.enable = g_p_raw->data[N9_INSTANT_VELOCITY_400CM].enable;
    g_kma_inst_ex.temperature_0_5m.enable = g_p_raw->data[N10_AIR_TEMPERATURE_50CM].enable;
    g_kma_inst_ex.temperature_4m.enable = g_p_raw->data[N11_AIR_TEMPERATURE_400CM].enable;
    g_kma_inst_ex.humidity_0_5m.enable = g_p_raw->data[N12_HUMIDITY_50CM].enable;
    g_kma_inst_ex.humidity_4m.enable = g_p_raw->data[N13_HUMIDITY_400CM].enable;
    g_kma_inst_ex.tacometer.enable = g_p_raw->data[I1_TACHOMETER].enable;
  }


void update_kma2_status(void)
{

}


  void DUALPORT_TASK(void *arg)
  {
    uint16_t sTriger = 0;
    SYSTEM_INFO_AWS *pSystem;
    SYSTEM_CONFIG_AWS *pConfig;
    DATE_TIME_BUF ct;
    AWS_DATA_STRUCT *pAws;
    uint16_t sSpeed;
    uint16_t sDirec;
    uint16_t sSpeedOld;
    uint16_t sDirecOld;
    uint16_t barometer;
    uint16_t year;
    uint8_t day;
    uint8_t min;
    uint8_t sec;
    uint8_t sensor_err=0;
    int i, j;
    int nWindCnt12 = 0;
    int nWindCnt40 = 0;

    pAws = &mRealAws;
    pSystem = &Sysinfo;
    pConfig = &Config;

    g_p_raw = aws_malloc(sizeof(measure_data_t));  // 250ms 마다 측정한 데이터

    while (1)
    {
      if (is_measurement(g_p_raw) == false)  // 데이터가 있는지 확인,250ms마다 업데이트 됨
      {
        continue;
      }
      ct = Date_Time;


      check_sensor_use();

      pSystem->mRain.sDayCount += get_rain_mm(&sensor_err);
      kma_update_sensor_err(A6_RAINFALL_DOT5_1MM,sensor_err);

      sSpeed = sSpeedOld;
      sDirec = sDirecOld;

      MegaErrorCheck(pSystem, &sSpeed, WindSpeedCalc(&sensor_err), 9999, MEGASPEED_ERR_CHAN);
      kma_update_sensor_err(A3_WIND_SPEED, sensor_err);

      MegaErrorCheck(pSystem, &sDirec, WindDirecCalc(&sensor_err), 9999, MEGADIREC_ERR_CHAN);
      kma_update_sensor_err(A2_WIND_DIRECTION, sensor_err);

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
      kma_update_sensor_err(A1_TEMPERATURE, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mBarometric.sReal, BarometricCalc(&sensor_err), 19999,
                     MEGABARO_ERR_CHAN);  // 1초 순간 기압
      kma_update_sensor_err(A7_PRESSURE, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mHumidity.sReal, HumidityCalc(&sensor_err), 9999,
                     MEGAHUMID_ERR_CHAN);  // 1초 순간 습도
      kma_update_sensor_err(A10_RELATIVE_HUMIDITY, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mSolarRad.sReal, SolarRadCalc(&sensor_err), 9999,
                     MEGASOL_ERR_CHAN);  // 일사
      kma_update_sensor_err(B1_SOLAR_RADIATION, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mSnowFall.sReal, SnowCalc(&sensor_err), 9999,
                     MEGASNOW_ERR_CHAN);  // 현재 적설량
      kma_update_sensor_err(A9_SNOW_DEPTH, sensor_err);

      // 추가 2017. 03. 22 지중 온도 추가  //
      MegaErrorCheck(pSystem, &pAws->mSoilTemp5cm.sReal, TempCalcExt(SOLITEMP5CM_CHN,&sensor_err), 9999,
                     MEGASOLI5TEMP_ERR_CHAN);  // 1초 순간 지중 5Cm온도

      kma_update_sensor_err(B5_SOIL_TEMPERATURE_5CM, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mSoilTemp10cm.sReal, TempCalcExt(SOLITEMP10CM_CHN,&sensor_err), 9999,
                     MEGASOLI10TEMP_ERR_CHAN);  // 1초 순간 지중 10Cm온도

      kma_update_sensor_err(B6_SOIL_TEMPERATURE_10CM, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mSoilTemp20cm.sReal,
                     TempCalcExt(SOLITEMP20CM_CHN, &sensor_err), 9999,
                     MEGASOLI20TEMP_ERR_CHAN);  // 1초 순간 지중 20Cm온도

      kma_update_sensor_err(B7_SOIL_TEMPERATURE_20CM, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mSoilTemp30cm.sReal,
                     TempCalcExt(SOLITEMP30CM_CHN, &sensor_err), 9999,
                     MEGASOLI30TEMP_ERR_CHAN);  // 1초 순간 지중 30Cm온도

      kma_update_sensor_err(B8_SOIL_TEMPERATURE_30CM, sensor_err);

      // 추가 2017. 03. 22 지중 온도 추가 END //
      MegaErrorCheck(pSystem, &pAws->mSoilTemp50cm.sReal,
                     TempCalcExt(SOLITEMP50CM_CHN, &sensor_err), 9999,
                     MEGASOLI10TEMP_ERR_CHAN);  // 1초 순간 지중 50Cm온도

      kma_update_sensor_err(B9_SOIL_TEMPERATURE_50CM, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mSoilTemp1_0m.sReal,
                     TempCalcExt(SOLITEMP1_0M_CHN, &sensor_err), 9999,
                     MEGASOLI20TEMP_ERR_CHAN);  // 1초 순간 지중 1~0m온도

      kma_update_sensor_err(B10_SOIL_TEMPERATURE_100CM, sensor_err);

      MegaErrorCheck(pSystem, &pAws->mSoilTemp1_5m.sReal,
                     TempCalcExt(SOLITEMP1_5M_CHN, &sensor_err), 9999,
                     MEGASOLI30TEMP_ERR_CHAN);  // 1초 순간 지중 1_5m온도
                                                // 추가 2017. 03. 22 지중 온도 추가 END //

      kma_update_sensor_err(B11_SOIL_TEMPERATURE_150CM, sensor_err);

      // Off Delay 적용 함
      if (is_raining(&sensor_err))  // 강우 감지
      {
        kma_update_sensor_err(A8_RAIN_PRESENT, sensor_err);

        pAws->mRainDetect.sReal = 0x000a;
        pSystem->m_shOffDelayRemain = pConfig->m_usRainDtOffDelay;
        pSystem->m_cOffDelayFlag = 1;
      }

      pAws->mSunshine.sReal = SunshineCalc(&sensor_err);  // CSD3 기준 0-1V 신호로 발생됨

      kma_update_sensor_err(B2_SUNSHINE_DURATION, sensor_err);

      // // 센서 불량 처리
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

      schedule_process(&ct);
      
      update_old_kma_real();
  }
}

const osThreadAttr_t dualportTask_attributes = {
    .name = "DUALPORT_TASK",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityNormal1,
};

extern void aws_data_task(void *arg) ;


void dualportTask_init(void)
{
  AwsMinMaxInit();

  osThreadNew(DUALPORT_TASK, NULL, &dualportTask_attributes);
  //osThreadNew(aws_data_task, NULL, &dualportTask_attributes);
}