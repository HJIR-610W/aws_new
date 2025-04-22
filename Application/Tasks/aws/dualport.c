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

measure_data_t *g_p_raw = NULL;



bool is_raining(void)
{

  sensor_data_t *p_sensor = g_p_raw->data;

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
uint16_t  WindSpeedCalc(void)
{
  uint16_t  sRet;
  uint16_t speed_x10; //tenth
  uint8_t err;
  sensor_data_t *p_sensor = g_p_raw->data;

  if (p_sensor[A3_WIND_SPEED].err)
  {
    speed_x10 = 9999;
  }
  else
  {
    speed_x10 = (uint16_t)(p_sensor[A3_WIND_SPEED].data.f * 10);
  }

  return speed_x10;
}

uint16_t  WindDirecCalc(void)
{
  uint16_t sRet;

  return sRet;
}

//(측정값 + 100)*10
uint16_t TempCalc(void)
 {
  sensor_data_t *p_sensor = g_p_raw->data;

  if (p_sensor[A1_TEMPERATURE].err)
  {
    return 9999;
  }
  
  return (uint16_t)((p_sensor[A1_TEMPERATURE].data.i +100)*10);

}

//측정값 *10
uint16_t  BarometricCalc(void)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  if (p_sensor[A7_PRESSURE].err)
  {
    return 9999;
  }

  return (uint16_t)(p_sensor[A7_PRESSURE].data.i * 10);
}

uint16_t HumidityCalc(void)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  if (p_sensor[A10_RELATIVE_HUMIDITY].err)
  {
    return 9999;
  }

  return (uint16_t)(p_sensor[A10_RELATIVE_HUMIDITY].data.i * 10);
}

uint16_t  SolarRadCalc(void)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  if (p_sensor[B1_SOLAR_RADIATION].err)
  {
    return 9999;
  }

  return (uint16_t)(p_sensor[B1_SOLAR_RADIATION].data.i * 10);
}

uint16_t SnowCalc(void)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  if (p_sensor[A9_SNOW_DEPTH].err)
  {
    return 9999;
  }

  return (uint16_t)(p_sensor[A9_SNOW_DEPTH].data.i);
}

uint16_t  TempCalcExt(uint8_t ch)
{
  uint16_t sRet;
  sensor_data_t *p_sensor = g_p_raw->data;

  switch (ch)
  {
    case SOLITEMP5CM_CHN:
      if (p_sensor[B5_SOIL_TEMPERATURE_5CM].err)
      {
        return 9999;
      }
      sRet = p_sensor[B5_SOIL_TEMPERATURE_5CM].data.i * 10;
      break;
    case SOLITEMP10CM_CHN:
      if (p_sensor[B6_SOIL_TEMPERATURE_10CM].err)
      {
        return 9999;
      }
      sRet = p_sensor[B6_SOIL_TEMPERATURE_10CM].data.i * 10;
      break;
    case SOLITEMP20CM_CHN:
      if (p_sensor[B7_SOIL_TEMPERATURE_20CM].err)
      {
        return 9999;
      }
      sRet = p_sensor[B7_SOIL_TEMPERATURE_20CM].data.i * 10;
      break;
    case SOLITEMP30CM_CHN:
      if (p_sensor[B8_SOIL_TEMPERATURE_30CM].err)
      {
        return 9999;
      }
      sRet = p_sensor[B8_SOIL_TEMPERATURE_30CM].data.i * 10;
      break;
    case SOLITEMP50CM_CHN:
      if (p_sensor[B9_SOIL_TEMPERATURE_50CM].err)
      {
        return 9999;
      }
      sRet = p_sensor[B9_SOIL_TEMPERATURE_50CM].data.i * 10;
      break;
    case SOLITEMP1_0M_CHN:
      if (p_sensor[B10_SOIL_TEMPERATURE_100CM].err)
      {
        return 9999;
      }
      sRet = p_sensor[B10_SOIL_TEMPERATURE_100CM].data.i * 10;
      break;
    case SOLITEMP1_5M_CHN:
      if (p_sensor[B11_SOIL_TEMPERATURE_150CM].err)
      {
        return 9999;
      }
      sRet = p_sensor[B11_SOIL_TEMPERATURE_150CM].data.i * 10;
      break;
      break;
  }

  return sRet;
}

uint8_t SunshineCalc(void)
{
  sensor_data_t *p_sensor = g_p_raw->data;
  if(p_sensor[B2_SUNSHINE_DURATION].data.b)
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
uint16_t get_rain_mm(void)
{
  sensor_data_t *p_sensor = g_p_raw->data;

  return (uint16_t)p_sensor[A6_RAINFALL_DOT5_1MM].data.i;
}


void update_old_kma_real(void)
{

  g_kma_inst_ex.temperature.data =  mRealAws.mTemperature.sReal;
  g_kma_inst_ex.temperature.max = mRealAws.mTemperature.sMax;
  g_kma_inst_ex.temperature.min = mRealAws.mTemperature.sMin;

  g_kma_inst_ex.relative_humidity.data = mRealAws.mHumidity.sReal;
  g_kma_inst_ex.relative_humidity.max  = mRealAws.mHumidity.sMin;
  g_kma_inst_ex.relative_humidity.min  = mRealAws.mHumidity.sMin;

  g_kma_inst_ex.wind_speed_avg.data     = mRealAws.mWind.mSpeed.sReal;
  g_kma_inst_ex.wind_speed_avg.max = mRealAws.mWind.mSpeed.sMax;


  g_kma_inst_ex.wind_direction_avg.data = mRealAws.mWind.mDirection.sReal;
  g_kma_inst_ex.wind_direction_avg.max = mRealAws.mWind.mDirection.sMax;

  g_kma_inst_ex.wind_speed_instant.data = mRealAws.mWind.mSpeed.sMax;
  g_kma_inst_ex.wind_direction_instant.data = mRealAws.mWind.mDirection.sMax;

  g_kma_inst_ex.sunshine_duration.data = mRealAws.mSunshine.sReal;
  g_kma_inst_ex.sunshine_duration.max = mRealAws.mSunshine.sMax;

  
  g_kma_inst_ex.solar_radiation.data = mRealAws.mSolarRad.sReal;
  g_kma_inst_ex.solar_radiation.max = mRealAws.mSolarRad.sMax;//일간


  g_kma_inst_ex.precipitation_presence.data = mRealAws.mRainDetect.sReal;//우량 감지

  g_kma_inst_ex.snowfall.data = mRealAws.mSnowFall.sReal;

  g_kma_inst_ex.pressure.max = mRealAws.mBarometric.sMax;
  g_kma_inst_ex.pressure.min = mRealAws.mBarometric.sMin;
  g_kma_inst_ex.pressure.data = mRealAws.mBarometric.sReal;

  g_kma_inst_ex.soil_temperature_5cm.data = mRealAws.mSoilTemp5cm.sReal;

  g_kma_inst_ex.soil_temperature_10cm.data = mRealAws.mSoilTemp10cm.sReal;
  g_kma_inst_ex.soil_temperature_20cm.data = mRealAws.mSoilTemp20cm.sReal;
  g_kma_inst_ex.soil_temperature_30cm.data = mRealAws.mSoilTemp30cm.sReal;
  g_kma_inst_ex.soil_temperature_50cm.data = mRealAws.mSoilTemp50cm.sReal;
  g_kma_inst_ex.soil_temperature_1m.data = mRealAws.mSoilTemp1_0m.sReal;
  g_kma_inst_ex.soil_temperature_1_5m.data = mRealAws.mSoilTemp1_5m.sReal;

}

void DUALPORT_TASK(void *arg)
{
  uint16_t sTriger=0;
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
  int i, j;
  int nWindCnt12 = 0;
  int nWindCnt40 = 0;


  pAws = &mRealAws;
  pSystem = &Sysinfo;
  pConfig = &Config;

  g_p_raw = aws_malloc(sizeof(measure_data_t));//250ms 마다 측정한 데이터 

  while (1)
  {
    if(is_measurement(g_p_raw)==false)//데이터가 있는지 확인,250ms마다 업데이트 됨
    {
      continue;
    }

    ct = Date_Time;

    pSystem->mRain.sDayCount += get_rain_mm();

    sSpeed = sSpeedOld;
    sDirec = sDirecOld;
    
    MegaErrorCheck(pSystem, &sSpeed, WindSpeedCalc(), 9999, MEGASPEED_ERR_CHAN);
    MegaErrorCheck(pSystem, &sDirec, WindDirecCalc(), 9999, MEGADIREC_ERR_CHAN);
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

    MegaErrorCheck(pSystem, &pAws->mTemperature.sReal, TempCalc(), 9999,
                   MEGATEMP_ERR_CHAN);  // 1초 순간 온도

    MegaErrorCheck(pSystem, &pAws->mBarometric.sReal, BarometricCalc(), 19999,
                   MEGABARO_ERR_CHAN);  // 1초 순간 기압

    MegaErrorCheck(pSystem, &pAws->mHumidity.sReal, HumidityCalc(), 9999,
                   MEGAHUMID_ERR_CHAN);  // 1초 순간 습도
    MegaErrorCheck(pSystem, &pAws->mSolarRad.sReal, SolarRadCalc(), 9999,
                   MEGASOL_ERR_CHAN);  // 일사
    MegaErrorCheck(pSystem, &pAws->mSnowFall.sReal, SnowCalc(), 9999,
                   MEGASNOW_ERR_CHAN);  // 현재 적설량

    // 추가 2017. 03. 22 지중 온도 추가  //
    MegaErrorCheck(pSystem, &pAws->mSoilTemp5cm.sReal, TempCalcExt(SOLITEMP5CM_CHN), 9999,
                   MEGASOLI5TEMP_ERR_CHAN);  // 1초 순간 지중 5Cm온도
    MegaErrorCheck(pSystem, &pAws->mSoilTemp10cm.sReal, TempCalcExt(SOLITEMP10CM_CHN), 9999,
                   MEGASOLI10TEMP_ERR_CHAN);  // 1초 순간 지중 10Cm온도
    MegaErrorCheck(pSystem, &pAws->mSoilTemp20cm.sReal, TempCalcExt(SOLITEMP20CM_CHN), 9999,
                   MEGASOLI20TEMP_ERR_CHAN);  // 1초 순간 지중 20Cm온도
    MegaErrorCheck(pSystem, &pAws->mSoilTemp30cm.sReal, TempCalcExt(SOLITEMP30CM_CHN), 9999,
                   MEGASOLI30TEMP_ERR_CHAN);  // 1초 순간 지중 30Cm온도
    // 추가 2017. 03. 22 지중 온도 추가 END //
    MegaErrorCheck(pSystem, &pAws->mSoilTemp50cm.sReal, TempCalcExt(SOLITEMP50CM_CHN), 9999,
                   MEGASOLI10TEMP_ERR_CHAN);  // 1초 순간 지중 50Cm온도
    MegaErrorCheck(pSystem, &pAws->mSoilTemp1_0m.sReal, TempCalcExt(SOLITEMP1_0M_CHN), 9999,
                   MEGASOLI20TEMP_ERR_CHAN);  // 1초 순간 지중 1~0m온도
    MegaErrorCheck(pSystem, &pAws->mSoilTemp1_5m.sReal, TempCalcExt(SOLITEMP1_5M_CHN), 9999,
                   MEGASOLI30TEMP_ERR_CHAN);  // 1초 순간 지중 1_5m온도
                                              // 추가 2017. 03. 22 지중 온도 추가 END //

    // Off Delay 적용 함
    if (is_raining())  // 강우 감지
    {
      pAws->mRainDetect.sReal = 0x000a;
      pSystem->m_shOffDelayRemain = pConfig->m_usRainDtOffDelay;
      pSystem->m_cOffDelayFlag = 1;
    }

    pAws->mSunshine.sReal =  SunshineCalc();  // CSD3 기준 0-1V 신호로 발생됨


    // // 센서 불량 처리
    pAws->mStatus.sReal =0;// pDp640to710->sLoggerDiStatus;  // m_main.h 참조

#ifdef RAIN_DETECT_HALL
#if 0 
        if (pDp640to710->sDiStatus & RAINFAIL_HALLDIBIT)
          pAws->mStatus.sMin |= RAINFALLFAIL_BIT;
        else
          pAws->mStatus.sMin &= ~(RAINFALLFAIL_BIT);
          #endif
#else
        if (pDp640to710->sDiStatus & RAINFAIL_DIBIT)
          pAws->mStatus.sMin |= RAINFALLFAIL_BIT;
        else
          pAws->mStatus.sMin &= ~(RAINFALLFAIL_BIT);
#endif

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

        pAws->mStatus.sMin &= ~(RAINDETECTFAIL_BIT);
        pAws->mStatus.sMin &= ~(FANFAIL_BIT);

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