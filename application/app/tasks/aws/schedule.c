#include <math.h>
#include <string.h>

#include "schedule.h"
#include "util_time.h"
#include "config_nvm.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "old_aws_define.h"

#include "app_sensor.h"
#include "aws_data.h"
#include "task_logging.h"
#include "app_dataLogging.h"
#include "util_memory.h"
#include "kma2.h"
#include "wind_data.h"
#include "system_err.h"
#include "dev_io.h"
#include "aws_default_data.h"
#define D2R 3.14159265 / 180.0
#define R2D 180.0 / 3.14159265

#define MAXWDSPEED 100.0


#pragma location = "SRAM_section"
AWS_DATA_STRUCT mRealAws;   // 실시간 자료
#pragma location = "SRAM_section"
AWS_DATA_STRUCT mMinAws;    // 1분 자료
#pragma location = "SRAM_section"
AWS_DATA_STRUCT m10MinAws;  // 10분 자료
#pragma location = "SRAM_section"
AWS_DATA_STRUCT mHourAws;   // 1시간 자료
#pragma location = "SRAM_section"
SYSTEM_INFO_AWS Sysinfo;
 

 void DircTouvConv(uint16_t sDirc, uint16_t sSpeed, float *dir_u, float *dir_v);
 uint16_t UVToDirc(float u_tmp, float v_tmp);
 void SecProcess(void);

 void MinProcess(DATE_TIME_BUF *pDate);
 void Min10Process(void);
 void HourProcess(DATE_TIME_BUF *pDate);
 void DayProcess(void);
 void MonthProcess(void);
 float UVToSpeed(float u_tmp, float v_tmp);

 /*
 lowlevel init 호출전에 SystemInit_ExtMemCtl 여기에서 FSMC 초기화를 해서
 초기화된 섹션,초기화되지 않은 섹션을 처리해줘야하는데
 FSMC 초기화에 문제가 있어. 일단
 main에서 FSMC 초기화한 다음 수동으로 FSMC영역에 배치된 변수를 0으로 초기화
 */
 void manual_bss_init(void)
 {
   memset(&mRealAws, 0, sizeof(mRealAws));
   memset(&mMinAws, 0, sizeof(mMinAws));
   memset(&m10MinAws, 0, sizeof(m10MinAws));
   memset(&mHourAws, 0, sizeof(mHourAws));
   memset(&Sysinfo, 0, sizeof(Sysinfo));
 }

void update_kma_data(eAWS_DATA_MIN_t min);


void AwsMinMaxInit(void)
{
  SYSTEM_INFO_AWS *pSystem;

  pSystem = &Sysinfo;

  mRealAws.mWind.mDirection.sMax = 0; //일 최대 풍향
  mRealAws.mWind.mSpeed.sMax = 0;    // 일 최대 풍속

  mRealAws.mTemperature.sMin = 9999;  // 일일 최대 최소 값 초기화
  mRealAws.mTemperature.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mTempBuf[i].sMin = 9999;
    pSystem->mTempBuf[i].sMax = 0;
  }


  mRealAws.mBarometric.sMin = 9999;
  mRealAws.mBarometric.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mBaroBuf[i].sMin = 9999;
    pSystem->mBaroBuf[i].sMax = 0;
  }

  mRealAws.mHumidity.sMin = 9999;
  mRealAws.mHumidity.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mHumidBuf[i].sMin = 9999;
    pSystem->mHumidBuf[i].sMax = 0;
  }


  //지중온도 5cm
  mRealAws.mSoilTemp5cm.sMin = 9999; //일 최대 최소 값 초기화 
  mRealAws.mSoilTemp5cm.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mSoil5Buf[i].sMin = 9999;
    pSystem->mSoil5Buf[i].sMax = 0;
  }

  // 지중온도 10cm
  mRealAws.mSoilTemp10cm.sMin = 9999;  // 일 최대 최소 값 초기화
  mRealAws.mSoilTemp10cm.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mSoil10Buf[i].sMin = 9999;
    pSystem->mSoil10Buf[i].sMax = 0;
  }
  // 지중온도 20cm
  mRealAws.mSoilTemp20cm.sMin = 9999;  // 일 최대 최소 값 초기화
  mRealAws.mSoilTemp20cm.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mSoil20Buf[i].sMin = 9999;
    pSystem->mSoil20Buf[i].sMax = 0;
  }

  // 지중온도 30cm
  mRealAws.mSoilTemp30cm.sMin = 9999;  // 일 최대 최소 값 초기화
  mRealAws.mSoilTemp30cm.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mSoil30Buf[i].sMin = 9999;
    pSystem->mSoil30Buf[i].sMax = 0;
  }

  // 지중온도 50cm
  mRealAws.mSoilTemp50cm.sMin = 9999;  // 일 최대 최소 값 초기화
  mRealAws.mSoilTemp50cm.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mSoil50Buf[i].sMin = 9999;
    pSystem->mSoil50Buf[i].sMax = 0;
  }

  // 지중온도 100cm
  mRealAws.mSoilTemp1_0m.sMin = 9999;  // 일 최대 최소 값 초기화
  mRealAws.mSoilTemp1_0m.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mSoil100Buf[i].sMin = 9999;
    pSystem->mSoil100Buf[i].sMax = 0;
  }

    // 지중온도 1.5m
  mRealAws.mSoilTemp1_5m.sMin = 9999;  // 일 최대 최소 값 초기화
  mRealAws.mSoilTemp1_5m.sMax = 0;

  for (int i = MIN1_PROC; i <= HOUR_PROC; i++)
  {
    pSystem->mSoil150Buf[i].sMin = 9999;
    pSystem->mSoil150Buf[i].sMax = 0;
  }


  mRealAws.mRainDetect.sReal = 0;
}

void AwsMinMaxProc(uint16_t sSour, uint16_t *psDestMin, uint16_t *psDestMax)
{
  if (sSour > *psDestMax)
    *psDestMax = sSour;
  if (sSour < *psDestMin)
    *psDestMin = sSour;
}

void SecProcess(void)
{

  SYSTEM_INFO_AWS *pSystem;

  pSystem = &Sysinfo;

  update_sensor_avg(eAVG_TEMPERATURE, g_sensor_avg_1min, mRealAws.mTemperature.sReal);
  calculate_sensor_min_max(eAVG_TEMPERATURE, g_1min_min_max, mRealAws.mTemperature.sReal);

      // 온도: 1분 누적을 구하기 위한 합
  pSystem->mTempBuf[MIN1_PROC].lTot += mRealAws.mTemperature.sReal;
  pSystem->mTempBuf[MIN1_PROC].sAddCnt++;

  //기압
  pSystem->mBaroBuf[MIN1_PROC].lTot += mRealAws.mBarometric.sReal;
  pSystem->mBaroBuf[MIN1_PROC].sAddCnt++;
  //습도
  pSystem->mHumidBuf[MIN1_PROC].lTot += mRealAws.mHumidity.sReal;
  pSystem->mHumidBuf[MIN1_PROC].sAddCnt++;

  // 2017.04.03 추가
  pSystem->mSoil5Buf[MIN1_PROC].lTot += mRealAws.mSoilTemp5cm.sReal;
  pSystem->mSoil5Buf[MIN1_PROC].sAddCnt++;

  pSystem->mSoil10Buf[MIN1_PROC].lTot += mRealAws.mSoilTemp10cm.sReal;
  pSystem->mSoil10Buf[MIN1_PROC].sAddCnt++;

  pSystem->mSoil20Buf[MIN1_PROC].lTot += mRealAws.mSoilTemp20cm.sReal;
  pSystem->mSoil20Buf[MIN1_PROC].sAddCnt++;

  pSystem->mSoil30Buf[MIN1_PROC].lTot += mRealAws.mSoilTemp30cm.sReal;
  pSystem->mSoil30Buf[MIN1_PROC].sAddCnt++;

  pSystem->mSoil50Buf[MIN1_PROC].lTot += mRealAws.mSoilTemp50cm.sReal;
  pSystem->mSoil50Buf[MIN1_PROC].sAddCnt++;

  pSystem->mSoil100Buf[MIN1_PROC].lTot += mRealAws.mSoilTemp1_0m.sReal;
  pSystem->mSoil100Buf[MIN1_PROC].sAddCnt++;

  pSystem->mSoil150Buf[MIN1_PROC].lTot += mRealAws.mSoilTemp1_5m.sReal;
  pSystem->mSoil150Buf[MIN1_PROC].sAddCnt++;

  // 지중 온도 5  최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp5cm.sReal, &mRealAws.mSoilTemp5cm.sMin,&mRealAws.mSoilTemp5cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp5cm.sReal,&pSystem->mSoil5Buf[MIN1_PROC].sMin,&pSystem->mSoil5Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp5cm.sReal,&pSystem->mSoil5Buf[MIN10_PROC].sMin,&pSystem->mSoil5Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp5cm.sReal,&pSystem->mSoil5Buf[HOUR_PROC].sMin,&pSystem->mSoil5Buf[HOUR_PROC].sMax);

  // 지중온도 10 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp10cm.sReal, &mRealAws.mSoilTemp10cm.sMin, &mRealAws.mSoilTemp10cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp10cm.sReal, &pSystem->mSoil10Buf[MIN1_PROC].sMin,&pSystem->mSoil10Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp10cm.sReal, &pSystem->mSoil10Buf[MIN10_PROC].sMin, &pSystem->mSoil10Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp10cm.sReal, &pSystem->mSoil10Buf[HOUR_PROC].sMin, &pSystem->mSoil10Buf[HOUR_PROC].sMax);

  // 지중온도 20  최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp20cm.sReal, &mRealAws.mSoilTemp20cm.sMin,&mRealAws.mSoilTemp20cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp20cm.sReal,&pSystem->mSoil20Buf[MIN1_PROC].sMin,&pSystem->mSoil20Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp20cm.sReal,&pSystem->mSoil20Buf[MIN10_PROC].sMin,&pSystem->mSoil20Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp20cm.sReal,&pSystem->mSoil20Buf[HOUR_PROC].sMin,&pSystem->mSoil20Buf[HOUR_PROC].sMax);

  // 지중온도 30 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp30cm.sReal, &mRealAws.mSoilTemp30cm.sMin,&mRealAws.mSoilTemp30cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp30cm.sReal,&pSystem->mSoil30Buf[MIN1_PROC].sMin,&pSystem->mSoil30Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp30cm.sReal,&pSystem->mSoil30Buf[MIN10_PROC].sMin,&pSystem->mSoil30Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp30cm.sReal,&pSystem->mSoil30Buf[HOUR_PROC].sMin,&pSystem->mSoil30Buf[HOUR_PROC].sMax);

  // 지중온도 50 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp50cm.sReal, &mRealAws.mSoilTemp50cm.sMin,&mRealAws.mSoilTemp50cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp50cm.sReal, &pSystem->mSoil50Buf[MIN1_PROC].sMin,&pSystem->mSoil50Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp50cm.sReal, &pSystem->mSoil50Buf[MIN10_PROC].sMin,&pSystem->mSoil50Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp50cm.sReal, &pSystem->mSoil50Buf[HOUR_PROC].sMin, &pSystem->mSoil50Buf[HOUR_PROC].sMax);

  // 지중온도 1_0 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp1_0m.sReal, &mRealAws.mSoilTemp1_0m.sMin,&mRealAws.mSoilTemp1_0m.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp1_0m.sReal,&pSystem->mSoil100Buf[MIN1_PROC].sMin,&pSystem->mSoil100Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp1_0m.sReal,&pSystem->mSoil100Buf[MIN10_PROC].sMin,&pSystem->mSoil100Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp1_0m.sReal,&pSystem->mSoil100Buf[HOUR_PROC].sMin,&pSystem->mSoil100Buf[HOUR_PROC].sMax);

  // 지중온도 1_5 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp1_5m.sReal, &mRealAws.mSoilTemp1_5m.sMin,&mRealAws.mSoilTemp1_5m.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp1_5m.sReal,&pSystem->mSoil150Buf[MIN1_PROC].sMin,&pSystem->mSoil150Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp1_5m.sReal,&pSystem->mSoil150Buf[MIN10_PROC].sMin,&pSystem->mSoil150Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp1_5m.sReal,&pSystem->mSoil150Buf[HOUR_PROC].sMin,&pSystem->mSoil150Buf[HOUR_PROC].sMax);

  // 2017 . 04 . 03 추가 끝
  // 온도 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mTemperature.sReal, &mRealAws.mTemperature.sMin,&mRealAws.mTemperature.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mTemperature.sReal, &pSystem->mTempBuf[MIN1_PROC].sMin,&pSystem->mTempBuf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mTemperature.sReal,&pSystem->mTempBuf[MIN10_PROC].sMin,&pSystem->mTempBuf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mTemperature.sReal, &pSystem->mTempBuf[HOUR_PROC].sMin,&pSystem->mTempBuf[HOUR_PROC].sMax);

  // 기압 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mBarometric.sReal, &mRealAws.mBarometric.sMin,&mRealAws.mBarometric.sMax);  // 일간 최고 최소 기압
  AwsMinMaxProc(mRealAws.mBarometric.sReal, &pSystem->mBaroBuf[MIN1_PROC].sMin,&pSystem->mBaroBuf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mBarometric.sReal, &pSystem->mBaroBuf[MIN10_PROC].sMin,&pSystem->mBaroBuf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mBarometric.sReal, &pSystem->mBaroBuf[HOUR_PROC].sMin,&pSystem->mBaroBuf[HOUR_PROC].sMax);

  // 습도 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mHumidity.sReal, &mRealAws.mHumidity.sMin,&mRealAws.mHumidity.sMax);  // 일간 최고 최소 습도
  AwsMinMaxProc(mRealAws.mHumidity.sReal, &pSystem->mHumidBuf[MIN1_PROC].sMin,&pSystem->mHumidBuf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mHumidity.sReal, &pSystem->mHumidBuf[MIN10_PROC].sMin,&pSystem->mHumidBuf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mHumidity.sReal, &pSystem->mHumidBuf[HOUR_PROC].sMin,&pSystem->mHumidBuf[HOUR_PROC].sMax);


  //우량
  if(pSystem->mRain.rain)
  {
    float rain = (float)pSystem->mRain.rain/10.0f;
    pSystem->mRain.rain = 0;

    g_rainfall.today   += rain;
    g_rainfall.min     += rain;
    g_rainfall.ten_min += rain;
    g_rainfall.hourly  += rain;
    g_rainfall.monthly += rain;
    g_rainfall.yearly  += rain;

  }
  
  mRealAws.mRainFall.sReal      = (uint16_t)(g_rainfall.today*10.0f);  
  mRealAws.mRainFall.sHourRain  = (uint16_t)(g_rainfall.hourly*10.0f); 
  mRealAws.mRainFall.sMonthRain = (uint16_t)(g_rainfall.monthly*10.0f);  
  mRealAws.mRainFall.sYearRain  = (uint16_t)(g_rainfall.yearly*10.0f);

  m10MinAws.mRainFall.sReal = (uint16_t)(g_rainfall.today*10.0f);
  mHourAws.mRainFall.sReal = (uint16_t)(g_rainfall.today*10.0f);

  // 강우 감지 처리 (초 단위로 처리)
  mMinAws.mRainDetect.sReal = mRealAws.mRainDetect.sReal;
  m10MinAws.mRainDetect.sReal = mRealAws.mRainDetect.sReal;
  mHourAws.mRainDetect.sReal = mRealAws.mRainDetect.sReal;



  // 일조 
  if (mRealAws.mSunshine.sReal)
  {
    g_sunshine.min++;
    g_sunshine.today++;
    g_sunshine.monthly++;
    g_sunshine.yearly++;
    g_sunshine.hourly++;
  }

  if (mRealAws.mSolarRad.sReal != 9999)
  {  
    uint32_t sunshine_r = 0;
    sunshine_r = g_solar_radiation.sunshine_r_1min_acc+mRealAws.mSolarRad.sReal;
    g_solar_radiation.sunshine_r_1min_acc = sunshine_r;
    g_solar_radiation.hourly = g_solar_radiation.hourly + mRealAws.mSolarRad.sReal;
  }

}



void AwsMinMaxTotSave(SENSOR_RIX_BUF *pSensor, SENSORPROC_BUF *pSensorTmp,
                      uint16_t sInitValue)
// pSensor    : 평균 최소 최대 값 들어갈 위치
// pSensorTmp : 임시로 연산을 위한 필드
// sInitValue : 최고 최소 값 초기화
// sAddCnt    : 평균 값을 구하기 위한 누적 횟수
{
  if (pSensorTmp->sAddCnt)
    pSensor->sReal = (uint16_t)(pSensorTmp->lTot / pSensorTmp->sAddCnt);
  else
    pSensor->sReal = 0;

  pSensorTmp->lTot = 0;
  pSensorTmp->sAddCnt = 0;
  pSensor->sMin = pSensorTmp->sMin;
  pSensor->sMax = pSensorTmp->sMax;
  pSensorTmp->sMax = sInitValue;
  pSensorTmp->sMin = sInitValue;
}

int WindMinMaxAvgSave(SENSOR_WIND_BUF *pSensor, SENSORWIND_BUF *pWindTmp,
                      SENSOR_WIND_BUF *pInit)
// *pSensor     : 평균 최소 최대 값 들어갈 위치
// *pWindTmp    : 임시로 연산을 위한 필드
// sInitValue   : 돌풍 초기화값
// sAddCnt      : 평균 값을 구하기 위한 누적 횟수
{
  float avg_u;
  float avg_v;
  int awv_wind_speed;

  avg_u = pWindTmp->uTot / (float)pWindTmp->sAddCnt;
  avg_v = pWindTmp->vTot / (float)pWindTmp->sAddCnt;

  // 샘플링된게 있으면 평균
  if (pWindTmp->sAddCnt)
  {
    // avg 자체가 awv단위
    awv_wind_speed = (int)UVToSpeed(avg_u, avg_v);

    pSensor->mSpeed.sReal = (uint16_t)awv_wind_speed;
  }
  else
  {
    pSensor->mSpeed.sReal = 0;
  }

  // 풍속이 0이면 풍향도 0으로 처리
  if (pSensor->mSpeed.sReal == 0)
  {
    pSensor->mDirection.sReal = 0;
  }
  else
  {
    pSensor->mDirection.sReal =
        (uint16_t)UVToDirc(pWindTmp->uTot / (float)pWindTmp->sAddCnt,
                         pWindTmp->vTot / (float)pWindTmp->sAddCnt);
  }
  pWindTmp->uTot = pWindTmp->vTot = 0.0;
  pWindTmp->lSpeedTot = 0;
  pWindTmp->sAddCnt = 0;
  pSensor->mDirection.sMax = pWindTmp->sGustDircMax;
  pSensor->mSpeed.sMax = pWindTmp->sGustSpeedMax;
  pWindTmp->sGustDircMax = 0;   // pInit->mDirection.sReal;
  pWindTmp->sGustSpeedMax = 0;  // pInit->mSpeed.sReal;

  return 0;
}

/*
1분이 됬을때 처리 내용
온도, 습도, 기압 일조, 일사 10분 누적, 및 1분 최소 최고 처리
일조 아루 총 누적에 처리
강수량 1분
*/
void MinProcess(DATE_TIME_BUF *pDate)
{
  SYSTEM_INFO_AWS *pSystem;
  AWS_DATA_STRUCT *pAws;
  float wind_speed_avg;
  float wind_direction_avg;
  int32_t wind_speed_max;
  int32_t wind_direction_max;
  int32_t average;
  int32_t min,max;

  pSystem = &Sysinfo;
  pAws = &mMinAws;

  pAws->mDate.cMonth = pDate->Month;  // 월일 시분만 기록
  pAws->mDate.cDay = pDate->Day;
  pAws->mDate.cHour = pDate->Hour;
  pAws->mDate.cMin = pDate->Min;

  // 온도
  AwsMinMaxTotSave(&pAws->mTemperature, &pSystem->mTempBuf[MIN1_PROC],mRealAws.mTemperature.sReal);

  average = read_sensor_avg(eAVG_TEMPERATURE,g_sensor_avg_1min);
  sensor_avg_init(eAVG_TEMPERATURE,g_sensor_avg_1min);

  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mTempBuf[MIN10_PROC].lTot += pAws->mTemperature.sReal;
  pSystem->mTempBuf[MIN10_PROC].sAddCnt++;

  // 기압
  AwsMinMaxTotSave(&pAws->mBarometric, &pSystem->mBaroBuf[MIN1_PROC],mRealAws.mBarometric.sReal);
  pSystem->mBaroBuf[MIN10_PROC].lTot += pAws->mBarometric.sReal;
  pSystem->mBaroBuf[MIN10_PROC].sAddCnt++;

  // 습도
  AwsMinMaxTotSave(&pAws->mHumidity, &pSystem->mHumidBuf[MIN1_PROC], mRealAws.mHumidity.sReal);
  pSystem->mHumidBuf[MIN10_PROC].lTot += pAws->mHumidity.sReal;
  pSystem->mHumidBuf[MIN10_PROC].sAddCnt++;

  pSystem->mSun[MIN10_PROC].nSunshineTot +=  g_sunshine.min;
  // 단위변환 W/M2 -> MJ/M2
  pSystem->mSun[MIN10_PROC].nSolarTot += g_solar_radiation.sunshine_r_1min/ 1000000;  

  // 풍향 풍속

  read_wind_max(eWIND_MAX_1MIN,&wind_speed_max,&wind_direction_max);
  wind_max_init(eWIND_MAX_1MIN);
  pAws->mWind.mSpeed.sMax = wind_speed_max;
  pAws->mWind.mDirection.sMax = wind_direction_max;

  calculate_wind_avg_1min(&wind_speed_avg, &wind_direction_avg);
  wind_avg_1min_init();
  pAws->mWind.mDirection.sReal = (uint16_t)(wind_direction_avg*10);
  pAws->mWind.mSpeed.sReal = (uint16_t)(wind_speed_avg * 10);



  // 일사 일조
  // 일조 1분 누적값
  pAws->mSunshine.sReal = g_sunshine.min;
  g_sunshine.min = 0;
  pAws->mSunshine.sMax = g_sunshine.today;

  pAws->mSolarRad.sReal = g_solar_radiation.sunshine_r_1min_acc / 1000; // 일사 1분   누적값  KJ/m2

  pSystem->mSun[MIN10_PROC].nSolarTot += pAws->mSolarRad.sReal;
  pAws->mSolarRad.sMax += pAws->mSolarRad.sReal;  // 하루 총 일사
  mRealAws.mSolarRad.sMax = pAws->mSolarRad.sMax;
  m10MinAws.mSolarRad.sMax = pAws->mSolarRad.sMax;
  mHourAws.mSolarRad.sMax = pAws->mSolarRad.sMax;

  // 지중  온도 2017.04.03
  AwsMinMaxTotSave(&pAws->mSoilTemp5cm, &pSystem->mSoil5Buf[MIN1_PROC], mRealAws.mSoilTemp5cm.sReal);
  pSystem->mSoil5Buf[MIN10_PROC].lTot += pAws->mSoilTemp5cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil5Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp10cm, &pSystem->mSoil10Buf[MIN1_PROC], mRealAws.mSoilTemp10cm.sReal);
  pSystem->mSoil10Buf[MIN10_PROC].lTot += pAws->mSoilTemp10cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil10Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp20cm, &pSystem->mSoil20Buf[MIN1_PROC],mRealAws.mSoilTemp20cm.sReal);
  pSystem->mSoil20Buf[MIN10_PROC].lTot += pAws->mSoilTemp20cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil20Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp30cm, &pSystem->mSoil30Buf[MIN1_PROC], mRealAws.mSoilTemp30cm.sReal);
  pSystem->mSoil30Buf[MIN10_PROC].lTot += pAws->mSoilTemp30cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil30Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp50cm, &pSystem->mSoil50Buf[MIN1_PROC],  mRealAws.mSoilTemp50cm.sReal);
  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil50Buf[MIN10_PROC].lTot += pAws->mSoilTemp50cm.sReal;  
  pSystem->mSoil50Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp1_0m, &pSystem->mSoil100Buf[MIN1_PROC], mRealAws.mSoilTemp1_0m.sReal);
  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil100Buf[MIN10_PROC].lTot += pAws->mSoilTemp1_0m.sReal; 
  pSystem->mSoil100Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp1_5m, &pSystem->mSoil150Buf[MIN1_PROC], mRealAws.mSoilTemp1_5m.sReal);

  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil150Buf[MIN10_PROC].lTot  +=  pAws->mSoilTemp1_5m.sReal; 
  pSystem->mSoil150Buf[MIN10_PROC].sAddCnt++;
  // 지중온도 처리 끝

  // 강수량 처리
  // 2010. 08. 28. 수정
  pAws->rain_1min = (uint16_t)(g_rainfall.min*10.0f);
  pAws->mRainFall.sReal = (uint16_t)(g_rainfall.today * 10.0f);
  pAws->mRainFall.sHourRain  = (uint16_t )(g_rainfall.hourly*10.0f);
  pAws->mRainFall.sMonthRain = (uint16_t )(g_rainfall.monthly*10.0f);
  pAws->mRainFall.sYearRain  = (uint16_t )(g_rainfall.yearly*10.0f);


  g_rainfall.min = 0;

  g_solar_radiation.sunshine_r_1min = g_solar_radiation.sunshine_r_1min_acc;
  g_solar_radiation.sunshine_r_1min_acc = 0;

  pAws->mSnowFall.sReal = mRealAws.mSnowFall.sReal;

  kma_data_ex_t *p_kma_avg = get_kma_data(eAWS_DATA_AVG);
  
  for(int i = 0 ; i< 8;i++)
  {
    pAws->kma3_sensor_status[i] = p_kma_avg->X_sensorStatus[i];
  }
  
  os_save_aws_data(pDate, pAws, sizeof(AWS_DATA_STRUCT), LOGGING_AWS, 1);
}

void Min10Process(void)
// 10분이 됬을때 처리 내용
// 온도, 습도, 기압 일조, 일사 1시간 누적, 및 10분 최소 최고 처리
{
  SYSTEM_INFO_AWS *pSystem;
  AWS_DATA_STRUCT *pAws;
  int32_t wind_speed_max;
  int32_t wind_direction_max;

  pSystem = &Sysinfo;
  pAws = &m10MinAws;

  // 온도
  AwsMinMaxTotSave(&pAws->mTemperature, &pSystem->mTempBuf[MIN10_PROC],mRealAws.mTemperature.sReal);
  pSystem->mTempBuf[HOUR_PROC].lTot += pAws->mTemperature.sReal;
  pSystem->mTempBuf[HOUR_PROC].sAddCnt++;

  // 기압
  AwsMinMaxTotSave(&pAws->mBarometric, &pSystem->mBaroBuf[MIN10_PROC],mRealAws.mBarometric.sReal);
  pSystem->mBaroBuf[HOUR_PROC].lTot += pAws->mBarometric.sReal;
  pSystem->mBaroBuf[HOUR_PROC].sAddCnt++;

  // 습도
  AwsMinMaxTotSave(&pAws->mHumidity, &pSystem->mHumidBuf[MIN10_PROC], mRealAws.mHumidity.sReal);
  pSystem->mHumidBuf[HOUR_PROC].lTot += pAws->mHumidity.sReal;
  pSystem->mHumidBuf[HOUR_PROC].sAddCnt++;

  // 풍향 풍속

  read_wind_max(eWIND_MAX_10MIN,&wind_speed_max,&wind_direction_max);
  pAws->mWind.mSpeed.sMax = wind_speed_max;
  pAws->mWind.mDirection.sMax = wind_direction_max;
  wind_max_init(eWIND_MAX_10MIN);


  // 일사 일조
  pSystem->mSun[HOUR_PROC].nSolarTot += pSystem->mSun[MIN10_PROC].nSolarTot;
  pSystem->mSun[HOUR_PROC].nSunshineTot += pSystem->mSun[MIN10_PROC].nSunshineTot;

  pAws->mSunshine.sReal = pSystem->mSun[MIN10_PROC].nSunshineTot;
  pSystem->mSun[MIN10_PROC].nSunshineTot = 0;

  pAws->mSolarRad.sReal = pSystem->mSun[MIN10_PROC].nSolarTot;
  pSystem->mSun[MIN10_PROC].nSolarTot = 0;
  pSystem->mSun[HOUR_PROC].nSolarTot += pAws->mSolarRad.sReal;

  // 지중 온도 처리 추가 17.04.03
  AwsMinMaxTotSave(&pAws->mSoilTemp5cm, &pSystem->mSoil5Buf[MIN10_PROC],
                   mRealAws.mSoilTemp5cm.sReal);
  pSystem->mSoil5Buf[HOUR_PROC].lTot += pAws->mSoilTemp5cm.sReal;
  pSystem->mSoil5Buf[HOUR_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp10cm, &pSystem->mSoil10Buf[MIN10_PROC],
                   mRealAws.mSoilTemp10cm.sReal);
  pSystem->mSoil10Buf[HOUR_PROC].lTot += pAws->mSoilTemp10cm.sReal;
  pSystem->mSoil10Buf[HOUR_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp20cm, &pSystem->mSoil20Buf[MIN10_PROC],
                   mRealAws.mSoilTemp20cm.sReal);
  pSystem->mSoil20Buf[HOUR_PROC].lTot += pAws->mSoilTemp20cm.sReal;
  pSystem->mSoil20Buf[HOUR_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp30cm, &pSystem->mSoil30Buf[MIN10_PROC],
                   mRealAws.mSoilTemp30cm.sReal);
  pSystem->mSoil30Buf[HOUR_PROC].lTot += pAws->mSoilTemp30cm.sReal;
  pSystem->mSoil30Buf[HOUR_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp50cm, &pSystem->mSoil50Buf[MIN10_PROC],
                   mRealAws.mSoilTemp50cm.sReal);
  pSystem->mSoil50Buf[HOUR_PROC].lTot += pAws->mSoilTemp50cm.sReal;
  pSystem->mSoil50Buf[HOUR_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp1_0m, &pSystem->mSoil100Buf[MIN10_PROC],
                   mRealAws.mSoilTemp1_0m.sReal);
  pSystem->mSoil100Buf[HOUR_PROC].lTot += pAws->mSoilTemp1_0m.sReal;
  pSystem->mSoil100Buf[HOUR_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp1_5m, &pSystem->mSoil150Buf[MIN10_PROC],
                   mRealAws.mSoilTemp1_5m.sReal);
  pSystem->mSoil150Buf[HOUR_PROC].lTot += pAws->mSoilTemp1_5m.sReal;
  pSystem->mSoil150Buf[HOUR_PROC].sAddCnt++;

  // 지중 온도 처리 끝

  // 강수량 처리
  pAws->mRainFall.sReal = (uint16_t)(g_rainfall.ten_min*10.0f);
  pAws->mRainFall.sHourRain = (uint16_t)(g_rainfall.hourly*10.0f);


  g_rainfall.ten_min = 0;

 m10MinAws.mSnowFall.sReal = mRealAws.mSnowFall.sReal;

}


/*
한시간 자료 처리 항목
온도
기압
습도
풍향
풍속
일사
일조
지중온도
*/
void HourProcess(DATE_TIME_BUF *pDate)
{
  SYSTEM_INFO_AWS *pSystem;
  AWS_DATA_STRUCT *pAws;
  int32_t wind_speed_max;
  int32_t wind_direction_max;

  pSystem = &Sysinfo;
  pAws = &mHourAws;

  // 온도
  AwsMinMaxTotSave(&pAws->mTemperature, &pSystem->mTempBuf[HOUR_PROC],mRealAws.mTemperature.sReal);
  // 기압
  AwsMinMaxTotSave(&pAws->mBarometric, &pSystem->mBaroBuf[HOUR_PROC],mRealAws.mBarometric.sReal);
  // 습도
  AwsMinMaxTotSave(&pAws->mHumidity, &pSystem->mHumidBuf[HOUR_PROC],mRealAws.mHumidity.sReal);
  // 풍향 풍속
  read_wind_max(eWIND_MAX_HOUR,&wind_speed_max,&wind_direction_max);
  pAws->mWind.mSpeed.sMax = wind_speed_max;
  pAws->mWind.mDirection.sMax = wind_direction_max;
  wind_max_init(eWIND_MAX_HOUR);

  // 일사 일조
  pAws->mSunshine.sReal = g_sunshine.hourly;
  pAws->mSolarRad.sReal = g_solar_radiation.hourly;

  // 지중온도 처리 2017.04.03
  AwsMinMaxTotSave(&pAws->mSoilTemp5cm, &pSystem->mSoil5Buf[HOUR_PROC],mRealAws.mSoilTemp5cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp10cm, &pSystem->mSoil10Buf[HOUR_PROC],mRealAws.mSoilTemp10cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp20cm, &pSystem->mSoil20Buf[HOUR_PROC],mRealAws.mSoilTemp20cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp30cm, &pSystem->mSoil30Buf[HOUR_PROC],mRealAws.mSoilTemp30cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp50cm, &pSystem->mSoil50Buf[HOUR_PROC],mRealAws.mSoilTemp50cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp1_0m, &pSystem->mSoil100Buf[HOUR_PROC],mRealAws.mSoilTemp1_0m.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp1_5m, &pSystem->mSoil150Buf[HOUR_PROC],mRealAws.mSoilTemp1_5m.sReal);
  // 지중 온도 처리 끝

  g_rainfall.hourly = 0;
  g_sunshine.hourly = 0;
  g_solar_radiation.hourly = 0;
}

void DayProcess(void)
{
  int32_t wind_speed_max;
  int32_t wind_direction_max;
  kma_data_ex_t *p_kma;

  p_kma = get_kma_data(eAWS_DATA_HOUR);

  read_wind_max(eWIND_MAX_DAY, &wind_speed_max, &wind_direction_max);
  p_kma->wind_direction_instant.data= wind_direction_max;
  p_kma->wind_speed_instant.data = wind_speed_max;
  wind_max_init(eWIND_MAX_DAY);

  mRealAws.mWind.mDirection.sMax = 0; //일 최대 풍향 초기화
  mRealAws.mWind.mSpeed.sMax = 0;    //일 최대 풍속 초기화

  mRealAws.mTemperature.sMin = mRealAws.mTemperature.sReal;//일 최저 기온 초기화
  mRealAws.mTemperature.sMax = 0;

  mRealAws.mBarometric.sMin = 9999; //일 최저 기압 초기화
  mRealAws.mBarometric.sMax = 0;

  mRealAws.mHumidity.sMin = 9999;  // 일 최저 습도 초기화
  mRealAws.mHumidity.sMax = 0;

  mRealAws.mSoilTemp5cm.sMin  = 9999;  // 일 최저 지중 온도 5cm
  mRealAws.mSoilTemp5cm.sMax = 0;

  mRealAws.mSoilTemp10cm.sMin = 9999;  // 일 최저 지중 온도 10cm
  mRealAws.mSoilTemp10cm.sMax = 0;

  mRealAws.mSoilTemp20cm.sMin = 9999;  // 일 최저 지중 온도 20cm
  mRealAws.mSoilTemp20cm.sMax = 0;

  mRealAws.mSoilTemp30cm.sMin = 9999;  // 일 최저 지중 온도 30cm
  mRealAws.mSoilTemp30cm.sMax = 0;

  mRealAws.mSoilTemp50cm.sMin = 9999;  // 일 최저 지중 온도 50cm
  mRealAws.mSoilTemp50cm.sMax = 0;

  mRealAws.mSoilTemp1_0m.sMin = 9999;  // 일 최저 지중 온도 1m
  mRealAws.mSoilTemp1_0m.sMax = 0;

  mRealAws.mSoilTemp1_5m.sMin = 9999;  // 일 최저 지중 온도 1.5cm
  mRealAws.mSoilTemp1_5m.sMax = 0;
  
  mRealAws.mSunshine.sMax = 0;  // 하루 총 일조
  mMinAws.mSunshine.sMax = 0;   // 하루 총 일조
  mMinAws.mSolarRad.sMax = 0;   // 하루 총 일사


  g_rainfall.yesterday = g_rainfall.today;

  g_rainfall.min = 0;
  g_rainfall.ten_min = 0;
  g_rainfall.hourly = 0;
  g_rainfall.today = 0;
  g_rainfall.monthly = 0;
  g_rainfall.yearly = 0;



  g_sunshine.today = 0;
}

void MonthProcess(void)
{
  g_rainfall.monthly = 0;
  g_sunshine.monthly = 0;
}

void YearProcess(void)
{
  g_rainfall.yearly = 0;
  g_sunshine.yearly = 0;
}

// 풍향,풍속으로 바람벡터 성분 분해
// :TODO 풍향 45도이면 u,v값이 같아야 하는데 다르게 계산됨, UVToDirc 이거 쌍으로 사용해야함
void DircTouvConv(uint16_t sDirc, uint16_t sSpeed, float *dir_u, float *dir_v)
{
  float fAngle;

  fAngle = (float)sDirc / 10;  // 3599를 359

  /* maker 측 제공 함수 사용 */
  /* Maker측 제공 함수는 2.2ms 정도 시간이 소요됨 */
  if (fAngle <= 90)
  {  // 1 상한 처리
    *dir_u += (float)sSpeed * (float)sin((double)fAngle * D2R);
    *dir_v += (float)sSpeed * (float)cos((double)fAngle * D2R);
  }
  else if (fAngle <= 180)
  {  // 2 상한 처리
    *dir_u += (float)sSpeed * (float)cos(((double)fAngle - 90.0) * D2R);
    *dir_v += (float)sSpeed * (float)sin(((double)fAngle - 90.0) * D2R) * -1.0;
  }
  else if (fAngle <= 270)
  {  // 3 상한 처리
    *dir_u += (float)sSpeed * (float)sin(((double)fAngle - 180.0) * D2R) * -1.0;
    *dir_v += (float)sSpeed * (float)cos(((double)fAngle - 180.0) * D2R) * -1.0;
  }
  else
  {  // 4 상한 처리
    *dir_u += (float)sSpeed * (float)cos(((double)fAngle - 270.0) * D2R) * -1.0;
    *dir_v += (float)sSpeed * (float)sin(((double)fAngle - 270.0) * D2R);
  }
/* maker 측 제공 함수 사용 끝 */

}
// AWS단위 측정값 *10
uint16_t UVToDirc(float u_tmp, float v_tmp)
{
  uint16_t sDirc;
  float f_arg;

  f_arg = fabs((v_tmp / MAXWDSPEED) / (u_tmp / MAXWDSPEED));
  if (v_tmp == 0 && u_tmp == 0)
  {
    sDirc = 0;
  }
  else if (v_tmp == 0 && u_tmp != 0)
  {
    if (u_tmp > 0)
      sDirc = 90 * 10;
    else
      sDirc = 270 * 10;
  }
  else if (v_tmp != 0 && u_tmp == 0)
  {
    if (v_tmp > 0)
      sDirc = 0;
    else
      sDirc = 180 * 10;
  }
  else if (v_tmp >= 0 && u_tmp >= 0)
  {
    sDirc = (uint16_t)((90.0 - (atan(f_arg) * R2D)) * 10.0);
  }
  else if (v_tmp < 0 && u_tmp >= 0)
  {
    sDirc = (uint16_t)(((atan(f_arg) * R2D) + 90.0) * 10.0);
  }
  else if (v_tmp < 0 && u_tmp < 0)
  {
    sDirc = (uint16_t)((270.0 - (atan(f_arg) * R2D)) * 10.0);
  }
  else if (v_tmp >= 0 && u_tmp < 0)
  {
    sDirc = (uint16_t)(((atan(f_arg) * R2D) + 270.0) * 10.0);
  }
  return (sDirc);
}

// 단위 m/s
float UVToSpeed(float u_tmp, float v_tmp)
{
  float wind_speed;

  // wind_speed = sqrtf(u_tmp * u_tmp + v_tmp * v_tmp);
  wind_speed = sqrt(u_tmp * u_tmp + v_tmp * v_tmp);
  return wind_speed;
}




void schedule_process(DATE_TIME_BUF *pDate, DATE_TIME_BUF *pOldDate)
{
  if (pDate->Sec != pOldDate->Sec)
  {  
    SecProcess();
    pOldDate->Sec = pDate->Sec;
  }

  if (pDate->Min != pOldDate->Min)
  { 
    MinProcess(pDate);
    update_kma_data(eAWS_DATA_1MIN);

    if (pDate->Min % 10 == 0)
    { 
      Min10Process();
      update_kma_data(eAWS_DATA_10MIN);
    }
    pOldDate->Min = pDate->Min;
  }

  if (pDate->Hour != pOldDate->Hour)
  { 
    HourProcess(pDate);
    update_kma_data(eAWS_DATA_HOUR);
    pOldDate->Hour = pDate->Hour;
  }

  if (pDate->Day != pOldDate->Day)
  {
    DayProcess();
    pOldDate->Day = pDate->Day;
  }

  if (pDate->Month != pOldDate->Month)
  {
    MonthProcess();
    pOldDate->Month = pDate->Month;
  }

  if (pDate->Year != pOldDate->Year)
  {
    YearProcess();
    pOldDate->Year = pDate->Year;
  }
}



SYSTEM_INFO_AWS *get_system_info_aws(void)
{
  return &Sysinfo;
}

void update_kma_data(eAWS_DATA_MIN_t min)
{
  kma_data_ex_t *p_kma_data;
  AWS_DATA_STRUCT *pAws;

  switch(min)
  {
    case eAWS_DATA_RAW:
      pAws = &mRealAws;
      break;
    case eAWS_DATA_1MIN:
      pAws = &mMinAws;
      break;
    case eAWS_DATA_10MIN:
      pAws = &m10MinAws;
      break;
    case eAWS_DATA_HOUR:
      pAws = &mHourAws;
      break;
  }

  p_kma_data = get_kma_data((eAWS_DATA_MIN_t)min);

  // 온도
  p_kma_data->temperature.data = pAws->mTemperature.sReal;
  p_kma_data->temperature.max = pAws->mTemperature.sMax;
  p_kma_data->temperature.min = pAws->mTemperature.sMin;

  // 기압
  p_kma_data->pressure.max = pAws->mBarometric.sMax;
  p_kma_data->pressure.min = pAws->mBarometric.sMin;
  p_kma_data->pressure.data = pAws->mBarometric.sReal;

  // 습도
  p_kma_data->relative_humidity.data = pAws->mHumidity.sReal;
  p_kma_data->relative_humidity.max = pAws->mHumidity.sMin;
  p_kma_data->relative_humidity.min = pAws->mHumidity.sMin;

  // 풍향
  p_kma_data->wind_direction_avg.data = pAws->mWind.mDirection.sReal;
  p_kma_data->wind_direction_avg.max = pAws->mWind.mDirection.sMax;

  // 풍속
  p_kma_data->wind_speed_avg.data = pAws->mWind.mSpeed.sReal;
  p_kma_data->wind_speed_avg.max = pAws->mWind.mSpeed.sMax;

  // 일조
  p_kma_data->sunshine_duration.data = g_sunshine.today;

  // 일사 // mSolarRad.sReal kw/m2 단위인데 전송시에는 mj/m2 *100 한값이 전송되어야함
  // 따라서 여기서 10으로 한번더 나누어 준다 .즉 data는 최종 전송되는 데이터 포맷이다.
  //에너지(J) = 전력(W)*시간(s)
  p_kma_data->solar_radiation.data = g_solar_radiation.sunshine_r_1min/ 10000; 
  p_kma_data->solar_radiation.max = pAws->mSolarRad.sMax;  // 일간

  // 지중 온도
  p_kma_data->soil_temperature_5cm.data = pAws->mSoilTemp5cm.sReal;
  p_kma_data->soil_temperature_5cm.max = pAws->mSoilTemp5cm.sMax;
  p_kma_data->soil_temperature_5cm.min = pAws->mSoilTemp5cm.sMin;

  p_kma_data->soil_temperature_10cm.data = pAws->mSoilTemp10cm.sReal;
  p_kma_data->soil_temperature_10cm.max = pAws->mSoilTemp10cm.sMax;
  p_kma_data->soil_temperature_10cm.min = pAws->mSoilTemp10cm.sMin;

  p_kma_data->soil_temperature_20cm.data = pAws->mSoilTemp20cm.sReal;
  p_kma_data->soil_temperature_20cm.max = pAws->mSoilTemp20cm.sMax;
  p_kma_data->soil_temperature_20cm.min = pAws->mSoilTemp20cm.sMin;

  p_kma_data->soil_temperature_30cm.data = pAws->mSoilTemp30cm.sReal;
  p_kma_data->soil_temperature_30cm.max = pAws->mSoilTemp30cm.sMax;
  p_kma_data->soil_temperature_30cm.min = pAws->mSoilTemp30cm.sMin;

  p_kma_data->soil_temperature_50cm.data = pAws->mSoilTemp50cm.sReal;
  p_kma_data->soil_temperature_50cm.max = pAws->mSoilTemp50cm.sMax;
  p_kma_data->soil_temperature_50cm.min = pAws->mSoilTemp50cm.sMin;

  p_kma_data->soil_temperature_1m.data = pAws->mSoilTemp1_0m.sReal;
  p_kma_data->soil_temperature_1m.max = pAws->mSoilTemp1_0m.sMax;
  p_kma_data->soil_temperature_1m.min = pAws->mSoilTemp1_0m.sMin;

  p_kma_data->soil_temperature_1_5m.data = pAws->mSoilTemp1_5m.sReal;
  p_kma_data->soil_temperature_1_5m.max = pAws->mSoilTemp1_5m.sMax;
  p_kma_data->soil_temperature_1_5m.min = pAws->mSoilTemp1_5m.sMin;

  p_kma_data->wind_speed_instant.data = pAws->mWind.mSpeed.sMax;
  p_kma_data->wind_direction_instant.data = pAws->mWind.mDirection.sMax;

  p_kma_data->precipitation.data  = pAws->mRainFall.sReal;
  p_kma_data->precipitation.hour  = pAws->mRainFall.sHourRain;    // 시간당 강수량량
  p_kma_data->precipitation.month = pAws->mRainFall.sMonthRain;  // 월간 강수량
  p_kma_data->precipitation.year  = pAws->mRainFall.sYearRain;  // 연간 강수량

  p_kma_data->precipitation_presence.data = pAws->mRainDetect.sReal;  // 우량 감지

  p_kma_data->snowfall.data = pAws->mSnowFall.sReal;

  kma_data_ex_t *p_kma_avg = get_kma_data(eAWS_DATA_AVG);

  for(int i = 0 ; i < 8 ;i++)
  {
    p_kma_data->X_sensorStatus[i] = p_kma_avg->X_sensorStatus[i];
  }
  p_kma_data->Y_volateStatus = p_kma_avg->Y_volateStatus;

  p_kma_data->updated = true;


  if(min == eAWS_DATA_1MIN)
  {

    
    p_kma_data->time = Date_Time;
    send_kma_data(eKMA_DATA_Q_1MIN, p_kma_data); // 실시간값을 공유자원 충돌없이 AI요청시 처리하기위한 목적
  }
}
