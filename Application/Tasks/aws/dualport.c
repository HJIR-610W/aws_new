#include <stdbool.h>

#include "cmsis_os2.h"

#include  "old_aws_define.h"
#include "schedule.h"

#include "utile_time.h"

#include "app_sensor.h"
#include "task_measure.h"
#include "schedule.h"
#include "user_heap.h"

measure_data_t *g_p_raw;


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

uint16_t TempCalc(void)
 {
  sensor_data_t *p_sensor = g_p_raw->data;

  if (p_sensor[A1_TEMPERATURE].err)
  {
    return 9999;
  }
  
  return (uint16_t)(p_sensor[A1_TEMPERATURE].data.i *10);

}

uint16_t  BarometricCalc(void)
{
  uint16_t sRet;

  return sRet;
}

uint16_t HumidityCalc(void)
{
  uint16_t sRet;

  return sRet;
}

uint16_t  SolarRadCalc(void)
{

  uint16_t sRet;

  return sRet;
}

uint16_t SnowCalc(void)
{
  uint16_t sRet;

  return sRet;
}

uint16_t  TempCalcExt(uint8_t ch)
{
  uint16_t sRet;

  return sRet;
}

uint8_t SunshineCalc(void)
{
  uint8_t sRet;

  return sRet;
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

  g_p_raw = aws_malloc(sizeof(measure_data_t));

  while (1)
  {
    if(is_measurement(g_p_raw)==false)
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

   // pSystem->windDirSrc = sDirec;
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

    // ==============================================================================
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
        // 센서 불량 처리 끝

        schedule_process(&ct);
  }
}

const osThreadAttr_t dualportTask_attributes = {
    .name = "DUALPORT_TASK",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityNormal1,
};

void dualportTask_init(void)
{
  AwsMinMaxInit();

  osThreadNew(DUALPORT_TASK, NULL, &dualportTask_attributes);
}