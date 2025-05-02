#include <math.h>
#include <string.h>

#include "schedule.h"
#include "utile_time.h"
#include "config_nvm.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "old_aws_define.h"

#include "app_sensor.h"
#include "aws_data.h"
#include "task_logging.h"
#include "app_dataLogging.h"

#define D2R 3.14159265 / 180.0
#define R2D 180.0 / 3.14159265

#define MAXWDSPEED 100.0

#define MIN1_PROC 0
#define MIN10_PROC 1
#define HOUR_PROC 2

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
#pragma location = "SRAM_section"
SYSTEM_CONFIG_AWS Config;
#pragma location = "SRAM_section"
AWS_DATA_STRUCT mMinAwsLog[60];  // 1분 Logging할 자료를 1시간 분량 저장

 static DATE_TIME_BUF OldDate;


 

 void DircTouvConv(uint16_t sDirc, uint16_t sSpeed, float *dir_u, float *dir_v);
 uint16_t UVToDirc(float u_tmp, float v_tmp);
 void SecProcess(void);
 void Sec10Process(void);
 void MinProcess(DATE_TIME_BUF *pDate);
 void Min10Process(void);
 void HourProcess(DATE_TIME_BUF *pDate);
 void DayProcess(void);
 void MonthProcess(void);
 float UVToSpeed(float u_tmp, float v_tmp);

void update_old_kma_1min(void);

 void AwsMinMaxInit(void)
 {
   SYSTEM_INFO_AWS *pSystem;

   pSystem = &Sysinfo;

   mRealAws.mWind.mDirection.sMax = mRealAws.mWind.mDirection.sReal;
   mRealAws.mWind.mSpeed.sMax = mRealAws.mWind.mSpeed.sReal;

   mRealAws.mTemperature.sMin = mRealAws.mTemperature.sReal;  // 일일 최대 최소 값 초기화
   mRealAws.mTemperature.sMax = mRealAws.mTemperature.sReal;
   mMinAws.mTemperature.sReal = mRealAws.mTemperature.sReal;
   mMinAws.mTemperature.sMin = mRealAws.mTemperature.sReal;  // 1분 최대 최소 값 초기화
   mMinAws.mTemperature.sMax = mRealAws.mTemperature.sReal;
   m10MinAws.mTemperature.sReal = mRealAws.mTemperature.sReal;
   m10MinAws.mTemperature.sMin = mRealAws.mTemperature.sReal;  // 10분 최대 최소 값 초기화
   m10MinAws.mTemperature.sMax = mRealAws.mTemperature.sReal;
   mHourAws.mTemperature.sReal = mRealAws.mTemperature.sReal;
   mHourAws.mTemperature.sMin = mRealAws.mTemperature.sReal;  // 1시간 최대 최소 값 초기화
   mHourAws.mTemperature.sMax = mRealAws.mTemperature.sReal;

   pSystem->mTempBuf[MIN1_PROC].sMin = mRealAws.mTemperature.sReal;
   pSystem->mTempBuf[MIN1_PROC].sMax = mRealAws.mTemperature.sReal;
   pSystem->mTempBuf[MIN10_PROC].sMin = mRealAws.mTemperature.sReal;
   pSystem->mTempBuf[MIN10_PROC].sMax = mRealAws.mTemperature.sReal;
   pSystem->mTempBuf[HOUR_PROC].sMin = mRealAws.mTemperature.sReal;
   pSystem->mTempBuf[HOUR_PROC].sMax = mRealAws.mTemperature.sReal;

   mRealAws.mBarometric.sMin = mRealAws.mBarometric.sReal;
   mRealAws.mBarometric.sMax = mRealAws.mBarometric.sReal;
   mMinAws.mBarometric.sReal = mRealAws.mBarometric.sReal;
   mMinAws.mBarometric.sMin = mRealAws.mBarometric.sReal;
   mMinAws.mBarometric.sMax = mRealAws.mBarometric.sReal;
   m10MinAws.mBarometric.sReal = mRealAws.mBarometric.sReal;
   m10MinAws.mBarometric.sMin = mRealAws.mBarometric.sReal;
   m10MinAws.mBarometric.sMax = mRealAws.mBarometric.sReal;
   mHourAws.mBarometric.sReal = mRealAws.mBarometric.sReal;
   mHourAws.mBarometric.sMin = mRealAws.mBarometric.sReal;
   mHourAws.mBarometric.sMax = mRealAws.mBarometric.sReal;
   pSystem->mBaroBuf[MIN1_PROC].sMin = mRealAws.mBarometric.sReal;
   pSystem->mBaroBuf[MIN1_PROC].sMax = mRealAws.mBarometric.sReal;
   pSystem->mBaroBuf[MIN10_PROC].sMin = mRealAws.mBarometric.sReal;
   pSystem->mBaroBuf[MIN10_PROC].sMax = mRealAws.mBarometric.sReal;
   pSystem->mBaroBuf[HOUR_PROC].sMin = mRealAws.mBarometric.sReal;
   pSystem->mBaroBuf[HOUR_PROC].sMax = mRealAws.mBarometric.sReal;

   mRealAws.mHumidity.sMin = mRealAws.mHumidity.sReal;
   mRealAws.mHumidity.sMax = mRealAws.mHumidity.sReal;
   mMinAws.mHumidity.sReal = mRealAws.mHumidity.sReal;
   mMinAws.mHumidity.sMin = mRealAws.mHumidity.sReal;
   mMinAws.mHumidity.sMax = mRealAws.mHumidity.sReal;
   m10MinAws.mHumidity.sReal = mRealAws.mHumidity.sReal;
   m10MinAws.mHumidity.sMin = mRealAws.mHumidity.sReal;
   m10MinAws.mHumidity.sMax = mRealAws.mHumidity.sReal;
   mHourAws.mHumidity.sReal = mRealAws.mHumidity.sReal;
   mHourAws.mHumidity.sMin = mRealAws.mHumidity.sReal;
   mHourAws.mHumidity.sMax = mRealAws.mHumidity.sReal;
   pSystem->mHumidBuf[MIN1_PROC].sMin = mRealAws.mHumidity.sReal;
   pSystem->mHumidBuf[MIN1_PROC].sMax = mRealAws.mHumidity.sReal;
   pSystem->mHumidBuf[MIN10_PROC].sMin = mRealAws.mHumidity.sReal;
   pSystem->mHumidBuf[MIN10_PROC].sMax = mRealAws.mHumidity.sReal;
   pSystem->mHumidBuf[HOUR_PROC].sMin = mRealAws.mHumidity.sReal;
   pSystem->mHumidBuf[HOUR_PROC].sMax = mRealAws.mHumidity.sReal;

   pSystem->shSnowFallOld = mRealAws.mSnowFall.sReal;  // 현재 적설을 옮긴다.

   mRealAws.mSoilTemp5cm.sMin = mRealAws.mSoilTemp5cm.sReal;  // 일일 최대 최소 값 초기화
   mRealAws.mSoilTemp5cm.sMax = mRealAws.mSoilTemp5cm.sReal;
   mMinAws.mSoilTemp5cm.sReal = mRealAws.mSoilTemp5cm.sReal;
   mMinAws.mSoilTemp5cm.sMin = mRealAws.mSoilTemp5cm.sReal;  // 1분 최대 최소 값 초기화
   mMinAws.mSoilTemp5cm.sMax = mRealAws.mSoilTemp5cm.sReal;
   m10MinAws.mSoilTemp5cm.sReal = mRealAws.mSoilTemp5cm.sReal;
   m10MinAws.mSoilTemp5cm.sMin = mRealAws.mSoilTemp5cm.sReal;  // 10분 최대 최소 값 초기화
   m10MinAws.mSoilTemp5cm.sMax = mRealAws.mSoilTemp5cm.sReal;
   mHourAws.mSoilTemp5cm.sReal = mRealAws.mSoilTemp5cm.sReal;
   mHourAws.mSoilTemp5cm.sMin = mRealAws.mSoilTemp5cm.sReal;  // 1시간 최대 최소 값 초기화
   mHourAws.mSoilTemp5cm.sMax = mRealAws.mSoilTemp5cm.sReal;

   mRealAws.mSoilTemp10cm.sMin = mRealAws.mSoilTemp10cm.sReal;  // 일일 최대 최소 값 초기화
   mRealAws.mSoilTemp10cm.sMax = mRealAws.mSoilTemp10cm.sReal;
   mMinAws.mSoilTemp10cm.sReal = mRealAws.mSoilTemp10cm.sReal;
   mMinAws.mSoilTemp10cm.sMin = mRealAws.mSoilTemp10cm.sReal;  // 1분 최대 최소 값 초기화
   mMinAws.mSoilTemp10cm.sMax = mRealAws.mSoilTemp10cm.sReal;
   m10MinAws.mSoilTemp10cm.sReal = mRealAws.mSoilTemp10cm.sReal;
   m10MinAws.mSoilTemp10cm.sMin = mRealAws.mSoilTemp10cm.sReal;  // 10분 최대 최소 값 초기화
   m10MinAws.mSoilTemp10cm.sMax = mRealAws.mSoilTemp10cm.sReal;
   mHourAws.mSoilTemp10cm.sReal = mRealAws.mSoilTemp10cm.sReal;
   mHourAws.mSoilTemp10cm.sMin = mRealAws.mSoilTemp10cm.sReal;  // 1시간 최대 최소 값 초기화
   mHourAws.mSoilTemp10cm.sMax = mRealAws.mSoilTemp10cm.sReal;

   mRealAws.mSoilTemp20cm.sMin = mRealAws.mSoilTemp20cm.sReal;  // 일일 최대 최소 값 초기화
   mRealAws.mSoilTemp20cm.sMax = mRealAws.mSoilTemp20cm.sReal;
   mMinAws.mSoilTemp20cm.sReal = mRealAws.mSoilTemp20cm.sReal;
   mMinAws.mSoilTemp20cm.sMin = mRealAws.mSoilTemp20cm.sReal;  // 1분 최대 최소 값 초기화
   mMinAws.mSoilTemp20cm.sMax = mRealAws.mSoilTemp20cm.sReal;
   m10MinAws.mSoilTemp20cm.sReal = mRealAws.mSoilTemp20cm.sReal;
   m10MinAws.mSoilTemp20cm.sMin = mRealAws.mSoilTemp20cm.sReal;  // 10분 최대 최소 값 초기화
   m10MinAws.mSoilTemp20cm.sMax = mRealAws.mSoilTemp20cm.sReal;
   mHourAws.mSoilTemp20cm.sReal = mRealAws.mSoilTemp20cm.sReal;
   mHourAws.mSoilTemp20cm.sMin = mRealAws.mSoilTemp20cm.sReal;  // 1시간 최대 최소 값 초기화
   mHourAws.mSoilTemp20cm.sMax = mRealAws.mSoilTemp20cm.sReal;

   mRealAws.mSoilTemp30cm.sMin = mRealAws.mSoilTemp30cm.sReal;  // 일일 최대 최소 값 초기화
   mRealAws.mSoilTemp30cm.sMax = mRealAws.mSoilTemp30cm.sReal;
   mMinAws.mSoilTemp30cm.sReal = mRealAws.mSoilTemp30cm.sReal;
   mMinAws.mSoilTemp30cm.sMin = mRealAws.mSoilTemp30cm.sReal;  // 1분 최대 최소 값 초기화
   mMinAws.mSoilTemp30cm.sMax = mRealAws.mSoilTemp30cm.sReal;
   m10MinAws.mSoilTemp30cm.sReal = mRealAws.mSoilTemp30cm.sReal;
   m10MinAws.mSoilTemp30cm.sMin = mRealAws.mSoilTemp30cm.sReal;  // 10분 최대 최소 값 초기화
   m10MinAws.mSoilTemp30cm.sMax = mRealAws.mSoilTemp30cm.sReal;
   mHourAws.mSoilTemp30cm.sReal = mRealAws.mSoilTemp30cm.sReal;
   mHourAws.mSoilTemp30cm.sMin = mRealAws.mSoilTemp30cm.sReal;  // 1시간 최대 최소 값 초기화
   mHourAws.mSoilTemp30cm.sMax = mRealAws.mSoilTemp30cm.sReal;

   mRealAws.mSoilTemp50cm.sMin = mRealAws.mSoilTemp50cm.sReal;  // 일일 최대 최소 값 초기화
   mRealAws.mSoilTemp50cm.sMax = mRealAws.mSoilTemp50cm.sReal;
   mMinAws.mSoilTemp50cm.sReal = mRealAws.mSoilTemp50cm.sReal;
   mMinAws.mSoilTemp50cm.sMin = mRealAws.mSoilTemp50cm.sReal;  // 1분 최대 최소 값 초기화
   mMinAws.mSoilTemp50cm.sMax = mRealAws.mSoilTemp50cm.sReal;
   m10MinAws.mSoilTemp50cm.sReal = mRealAws.mSoilTemp50cm.sReal;
   m10MinAws.mSoilTemp50cm.sMin = mRealAws.mSoilTemp50cm.sReal;  // 10분 최대 최소 값 초기화
   m10MinAws.mSoilTemp50cm.sMax = mRealAws.mSoilTemp50cm.sReal;
   mHourAws.mSoilTemp50cm.sReal = mRealAws.mSoilTemp50cm.sReal;
   mHourAws.mSoilTemp50cm.sMin = mRealAws.mSoilTemp50cm.sReal;  // 1시간 최대 최소 값 초기화
   mHourAws.mSoilTemp50cm.sMax = mRealAws.mSoilTemp50cm.sReal;

   mRealAws.mSoilTemp1_0m.sMin = mRealAws.mSoilTemp1_0m.sReal;  // 일일 최대 최소 값 초기화
   mRealAws.mSoilTemp1_0m.sMax = mRealAws.mSoilTemp1_0m.sReal;
   mMinAws.mSoilTemp1_0m.sReal = mRealAws.mSoilTemp1_0m.sReal;
   mMinAws.mSoilTemp1_0m.sMin = mRealAws.mSoilTemp1_0m.sReal;  // 1분 최대 최소 값 초기화
   mMinAws.mSoilTemp1_0m.sMax = mRealAws.mSoilTemp1_0m.sReal;
   m10MinAws.mSoilTemp1_0m.sReal = mRealAws.mSoilTemp1_0m.sReal;
   m10MinAws.mSoilTemp1_0m.sMin = mRealAws.mSoilTemp1_0m.sReal;  // 10분 최대 최소 값 초기화
   m10MinAws.mSoilTemp1_0m.sMax = mRealAws.mSoilTemp1_0m.sReal;
   mHourAws.mSoilTemp1_0m.sReal = mRealAws.mSoilTemp1_0m.sReal;
   mHourAws.mSoilTemp1_0m.sMin = mRealAws.mSoilTemp1_0m.sReal;  // 1시간 최대 최소 값 초기화
   mHourAws.mSoilTemp1_0m.sMax = mRealAws.mSoilTemp1_0m.sReal;

   mRealAws.mSoilTemp1_5m.sMin = mRealAws.mSoilTemp1_5m.sReal;  // 일일 최대 최소 값 초기화
   mRealAws.mSoilTemp1_5m.sMax = mRealAws.mSoilTemp1_5m.sReal;
   mMinAws.mSoilTemp1_5m.sReal = mRealAws.mSoilTemp1_5m.sReal;
   mMinAws.mSoilTemp1_5m.sMin = mRealAws.mSoilTemp1_5m.sReal;  // 1분 최대 최소 값 초기화
   mMinAws.mSoilTemp1_5m.sMax = mRealAws.mSoilTemp1_5m.sReal;
   m10MinAws.mSoilTemp1_5m.sReal = mRealAws.mSoilTemp1_5m.sReal;
   m10MinAws.mSoilTemp1_5m.sMin = mRealAws.mSoilTemp1_5m.sReal;  // 10분 최대 최소 값 초기화
   m10MinAws.mSoilTemp1_5m.sMax = mRealAws.mSoilTemp1_5m.sReal;
   mHourAws.mSoilTemp1_5m.sReal = mRealAws.mSoilTemp1_5m.sReal;
   mHourAws.mSoilTemp1_5m.sMin = mRealAws.mSoilTemp1_5m.sReal;  // 1시간 최대 최소 값 초기화
   mHourAws.mSoilTemp1_5m.sMax = mRealAws.mSoilTemp1_5m.sReal;

   pSystem->m_cOffDelayFlag = 0;  // Rain Detecter 의 현재를  Off상태로 만든다
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
  uint16_t sAvgSpeed;
  uint16_t sAvgDirection;
  uint16_t sRain;
  uint32_t nSpeedTot, i;
  SYSTEM_CONFIG_AWS *pConfig;
  SYSTEM_INFO_AWS *pSystem;
  uint64_t windSum=0;
  float wind_sum_u = 0;
  float wind_sum_v = 0;

  pConfig = &Config;
  pSystem = &Sysinfo;

  // 1분 누적을 구하기 위한 합
  pSystem->mTempBuf[MIN1_PROC].lTot += mRealAws.mTemperature.sReal;
  pSystem->mTempBuf[MIN1_PROC].sAddCnt++;
  pSystem->mBaroBuf[MIN1_PROC].lTot += mRealAws.mBarometric.sReal;
  pSystem->mBaroBuf[MIN1_PROC].sAddCnt++;
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
  AwsMinMaxProc(mRealAws.mSoilTemp5cm.sReal, &mRealAws.mSoilTemp5cm.sMin,
                &mRealAws.mSoilTemp5cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp5cm.sReal,
                &pSystem->mSoil5Buf[MIN1_PROC].sMin,
                &pSystem->mSoil5Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp5cm.sReal,
                &pSystem->mSoil5Buf[MIN10_PROC].sMin,
                &pSystem->mSoil5Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp5cm.sReal,
                &pSystem->mSoil5Buf[HOUR_PROC].sMin,
                &pSystem->mSoil5Buf[HOUR_PROC].sMax);

  // 지중온도 10 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp10cm.sReal, &mRealAws.mSoilTemp10cm.sMin,
                &mRealAws.mSoilTemp10cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp10cm.sReal,
                &pSystem->mSoil10Buf[MIN1_PROC].sMin,
                &pSystem->mSoil10Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp10cm.sReal,
                &pSystem->mSoil10Buf[MIN10_PROC].sMin,
                &pSystem->mSoil10Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp10cm.sReal,
                &pSystem->mSoil10Buf[HOUR_PROC].sMin,
                &pSystem->mSoil10Buf[HOUR_PROC].sMax);

  // 지중온도 20  최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp20cm.sReal, &mRealAws.mSoilTemp20cm.sMin,
                &mRealAws.mSoilTemp20cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp20cm.sReal,
                &pSystem->mSoil20Buf[MIN1_PROC].sMin,
                &pSystem->mSoil20Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp20cm.sReal,
                &pSystem->mSoil20Buf[MIN10_PROC].sMin,
                &pSystem->mSoil20Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp20cm.sReal,
                &pSystem->mSoil20Buf[HOUR_PROC].sMin,
                &pSystem->mSoil20Buf[HOUR_PROC].sMax);

  // 지중온도 30 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp30cm.sReal, &mRealAws.mSoilTemp30cm.sMin,
                &mRealAws.mSoilTemp30cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp30cm.sReal,
                &pSystem->mSoil30Buf[MIN1_PROC].sMin,
                &pSystem->mSoil30Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp30cm.sReal,
                &pSystem->mSoil30Buf[MIN10_PROC].sMin,
                &pSystem->mSoil30Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp30cm.sReal,
                &pSystem->mSoil30Buf[HOUR_PROC].sMin,
                &pSystem->mSoil30Buf[HOUR_PROC].sMax);

  // 지중온도 50 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp50cm.sReal, &mRealAws.mSoilTemp50cm.sMin,
                &mRealAws.mSoilTemp50cm.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp50cm.sReal,
                &pSystem->mSoil50Buf[MIN1_PROC].sMin,
                &pSystem->mSoil50Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp50cm.sReal,
                &pSystem->mSoil50Buf[MIN10_PROC].sMin,
                &pSystem->mSoil50Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp50cm.sReal,
                &pSystem->mSoil50Buf[HOUR_PROC].sMin,
                &pSystem->mSoil50Buf[HOUR_PROC].sMax);

  // 지중온도 1_0 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp1_0m.sReal, &mRealAws.mSoilTemp1_0m.sMin,
                &mRealAws.mSoilTemp1_0m.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp1_0m.sReal,
                &pSystem->mSoil100Buf[MIN1_PROC].sMin,
                &pSystem->mSoil100Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp1_0m.sReal,
                &pSystem->mSoil100Buf[MIN10_PROC].sMin,
                &pSystem->mSoil100Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp1_0m.sReal,
                &pSystem->mSoil100Buf[HOUR_PROC].sMin,
                &pSystem->mSoil100Buf[HOUR_PROC].sMax);

  // 지중온도 1_5 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mSoilTemp1_5m.sReal, &mRealAws.mSoilTemp1_5m.sMin,
                &mRealAws.mSoilTemp1_5m.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mSoilTemp1_5m.sReal,
                &pSystem->mSoil150Buf[MIN1_PROC].sMin,
                &pSystem->mSoil150Buf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp1_5m.sReal,
                &pSystem->mSoil150Buf[MIN10_PROC].sMin,
                &pSystem->mSoil150Buf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mSoilTemp1_5m.sReal,
                &pSystem->mSoil150Buf[HOUR_PROC].sMin,
                &pSystem->mSoil150Buf[HOUR_PROC].sMax);

  // 2017 . 04 . 03 추가 끝

  // 온도 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mTemperature.sReal, &mRealAws.mTemperature.sMin,
                &mRealAws.mTemperature.sMax);  // 일간 최고 최소 온도
  AwsMinMaxProc(mRealAws.mTemperature.sReal, &pSystem->mTempBuf[MIN1_PROC].sMin,
                &pSystem->mTempBuf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mTemperature.sReal,
                &pSystem->mTempBuf[MIN10_PROC].sMin,
                &pSystem->mTempBuf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mTemperature.sReal, &pSystem->mTempBuf[HOUR_PROC].sMin,
                &pSystem->mTempBuf[HOUR_PROC].sMax);

  // 기압 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mBarometric.sReal, &mRealAws.mBarometric.sMin,
                &mRealAws.mBarometric.sMax);  // 일간 최고 최소 기압
  AwsMinMaxProc(mRealAws.mBarometric.sReal, &pSystem->mBaroBuf[MIN1_PROC].sMin,
                &pSystem->mBaroBuf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mBarometric.sReal, &pSystem->mBaroBuf[MIN10_PROC].sMin,
                &pSystem->mBaroBuf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mBarometric.sReal, &pSystem->mBaroBuf[HOUR_PROC].sMin,
                &pSystem->mBaroBuf[HOUR_PROC].sMax);

  // 습도 최소 최대 구하기
  AwsMinMaxProc(mRealAws.mHumidity.sReal, &mRealAws.mHumidity.sMin,
                &mRealAws.mHumidity.sMax);  // 일간 최고 최소 습도
  AwsMinMaxProc(mRealAws.mHumidity.sReal, &pSystem->mHumidBuf[MIN1_PROC].sMin,
                &pSystem->mHumidBuf[MIN1_PROC].sMax);
  AwsMinMaxProc(mRealAws.mHumidity.sReal, &pSystem->mHumidBuf[MIN10_PROC].sMin,
                &pSystem->mHumidBuf[MIN10_PROC].sMax);
  AwsMinMaxProc(mRealAws.mHumidity.sReal, &pSystem->mHumidBuf[HOUR_PROC].sMin,
                &pSystem->mHumidBuf[HOUR_PROC].sMax);

  // 강우량처리
 // if (pConfig->cRainSensorType == 1)
   // sRain = (pSystem->mRain.sDayCount - pSystem->mRain.sDayCountOld) *
    //        5;  // 0.5 mm 수수구를 사용
 // else
   // sRain = (pSystem->mRain.sDayCount - pSystem->mRain.sDayCountOld) *
     //       10;  // 1 mm 수수구를 사용
  if (sRain != 0)
  {
    // 강우량 값이 변경 되었음
    pSystem->mRain.sDayCountOld = pSystem->mRain.sDayCount;

    pSystem->mRain.sMinRain += sRain;  // 1분 강수량
    //        m1MinAws->mRainFall.sReal   += sRain; // 1분 강수량

    pSystem->mRain.s10MinRain += sRain;  // 10분 강수량
    //        m10MinAws->mRainFall.sReal  += sRain; // 10분 강수량

    pSystem->mRain.sHourRain += sRain;  // 1시간강수량
    //        mHourAws->mRainFall.sReal   += sRain; // 1시간강수량

    pSystem->mRain.sDayRain += sRain;     // 일간강수량
    pSystem->mNVram.nMonthRain += sRain;  // 월간강수량
  /*  RtccSmsWriteLong(
        MONTHRAIN_DS1306,
        pSystem->mNVram
            .nMonthRain);  // 시스템 초기화시 저장된 값을 가지고 온다.
    pSystem->mNVram.nYearRain += sRain;  // 연간강수량
    RtccSmsWriteLong(YEARRAIN_DS1306, pSystem->mNVram.nYearRain);
    */
  }
  mRealAws.mRainFall.sReal =
      pSystem->mRain.sDayRain;  // 일간강수량(초단위로 바뀌는 값)
  // 2010. 08. 28. 수정 : 1일 강수량으로 만들기위함
  mMinAws.mRainFall.sReal = pSystem->mRain.sDayRain;
  m10MinAws.mRainFall.sReal = pSystem->mRain.sDayRain;
  mHourAws.mRainFall.sReal = pSystem->mRain.sDayRain;

  mRealAws.mRainFall.sMax = pSystem->mRain.sHourRain;  // 1시간 강수량
  mRealAws.mRainFall.sMin =
      (uint16_t)pSystem->mNVram.nMonthRain;  // 월간강수량( " )
  mRealAws.mRainFall.sSpec =
      (uint16_t)pSystem->mNVram.nYearRain;  // 연간강수량( " )

  // 강우 감지 처리 (초 단위로 처리)
  mMinAws.mRainDetect.sReal = mRealAws.mRainDetect.sReal;
  m10MinAws.mRainDetect.sReal = mRealAws.mRainDetect.sReal;
  mHourAws.mRainDetect.sReal = mRealAws.mRainDetect.sReal;

  // 1분 자료를 10분 자료에 복사
  //    memcpy((char *)&(m10MinAws.mRainFall), (char *)&(mMinAws.mRainFall),
  //    sizeof(SENSOR_RIXS_BUF));
  // 1분 자료를 1시간 자료에 복사
  //    memcpy((char *)&(mHourAws.mRainFall), (char *)&(mMinAws.mRainFall),
  //    sizeof(SENSOR_RIXS_BUF));

  // 풍향 풍속 처리 & 3초 이동 평균 처리
#define WIND_INSTANCT_CNT 12
  windSum = 0;
  
  for (i = 0; i < WIND_INSTANCT_CNT; i++)
  {
    sAvgSpeed = pSystem->mRealWind.sAvg3Speed[i];
    sAvgDirection = pSystem->mRealWind.sAvg3Direction[i];

    windSum += sAvgSpeed;

    if (sAvgSpeed)
    {
      DircTouvConv(sAvgDirection, sAvgSpeed, &wind_sum_u, &wind_sum_v);
    }
  }

  //pSystem->windSpeed = windSum / WIND_INSTANCT_CNT;// real 값 표시 

  {
    // 12샘플링한 자료를 평균해서 순간 풍향,풍속 산출출
    float wind_avg_u = wind_sum_u / WIND_INSTANCT_CNT;
    float wind_avg_v = wind_sum_v / WIND_INSTANCT_CNT;
    uint16_t wind_speed = 0;
    uint16_t wind_deg = 0;
    int aws_speed;
    // 주의:wind_avg 값 자체가 aws 단위이다.
    aws_speed = (int)(UVToSpeed(wind_avg_u, wind_avg_v));

    wind_speed = (uint16_t)aws_speed;
    if (wind_speed == 0)
    {
      wind_deg = 0;
    }
    else
    {
      wind_deg = UVToDirc(wind_avg_u, wind_avg_v);
    }
    mRealAws.mWind.mSpeed.sReal = wind_speed;
    mRealAws.mWind.mDirection.sReal = wind_deg;

    if (wind_speed >= mRealAws.mWind.mSpeed.sMax)  // 일간 최대 풍속
    {
      mRealAws.mWind.mSpeed.sMax = wind_speed;
      mRealAws.mWind.mDirection.sMax = wind_deg;  // 풍속이 최대일때의 풍향향
    }

    if (wind_speed >= pSystem->mWind[0].sGustSpeedMax)  // 1분 최대 풍속
    {
      pSystem->mWind[0].sGustSpeedMax = wind_speed;
      pSystem->mWind[0].sGustDircMax = wind_deg;
    }

    if (wind_speed >= pSystem->mWind[1].sGustSpeedMax)  // 10분 최대 풍속
    {
      pSystem->mWind[1].sGustSpeedMax = wind_speed;
      pSystem->mWind[1].sGustDircMax = wind_deg;
    }

    if (wind_speed >= pSystem->mWind[2].sGustSpeedMax)  // 1시간 최대 풍속
    {
      pSystem->mWind[2].sGustSpeedMax = wind_speed;
      pSystem->mWind[2].sGustDircMax = wind_deg;
    }
  }
  // 3초 이동 평균(1초 Sample)
  //    mRealAws.mWind.mDirection.sReal   = sAvgDirection; // 1초 풍향
  //    Dualport.c에서 처리함(0.25초 간격으로)
  // 일사 일조 처리
  if (mRealAws.mSunshine.sReal)
  {
    mRealAws.mSunshine.sMax += 1;  // 리얼값 누적  2017.03.22

    pSystem->mSun[MIN1_PROC].nSunshineTot += 1;  // 1분 누적 일조
    pSystem->mNVram.nMonthSunshine += 1;         // 월간 누적 일조량
   // RtccSmsWriteLong(MONTHSUNSHINE_DS1306, pSystem->mNVram.nMonthSunshine);
    pSystem->mNVram.nYearSunshine += 1;  // 연간 누적 일조량
   // RtccSmsWriteLong(YEARSUNSHINE_DS1306, pSystem->mNVram.nYearSunshine);
  }

  if (mRealAws.mSolarRad.sReal != 9999)  // 에러값이 아니면 누적일사를 구한다.
    pSystem->mSun[MIN1_PROC].nSolarTot +=
        mRealAws.mSolarRad.sReal;  // 1분 누적 일사

  if (pSystem->m_cOffDelayFlag)  // 강우 감지 루틴
  {
    if (pConfig->m_usRainDtOffDelay >= 1)
    {
      if (--pSystem->m_shOffDelayRemain <= 0)
      {
        pSystem->m_cOffDelayFlag = 0;
        mRealAws.mRainDetect.sReal = 0;
      }
    }
    else
    {
      pSystem->m_cOffDelayFlag = 0;
      mRealAws.mRainDetect.sReal = 0;
      pSystem->m_shOffDelayRemain = 0;
    }
  }
}

void Sec10Process(void)
{
  SYSTEM_INFO_AWS *pSystem;
  float u, v;
  int i;
  uint32_t nSpeedTot;
  uint32_t sAcnt = 0;

  pSystem = &Sysinfo;

  // 풍향 풍속 처리
  u = v = 0.0;
  nSpeedTot = 0;

  for (i = 0; i < 40; i++)
  {
    if (pSystem->mRealWind.sWrFlag[i])
    {
      DircTouvConv(pSystem->mRealWind.sAvg10Direction[i],
                   pSystem->mRealWind.sAvg10Speed[i], &u, &v);
      nSpeedTot += pSystem->mRealWind.sAvg10Speed[i];
      sAcnt++;
    }
  }
  if (sAcnt)
  {
    pSystem->mWind[MIN1_PROC].uTot +=
        (u / (float)sAcnt);  // 10초 평균을 구한후 합산한다
    pSystem->mWind[MIN1_PROC].vTot += (v / (float)sAcnt);  //       "

    pSystem->mWind[MIN1_PROC].lSpeedTot += (nSpeedTot / sAcnt);  //       "
    pSystem->mWind[MIN1_PROC].sAddCnt++;
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
    awv_wind_speed = UVToSpeed(avg_u, avg_v);

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

void MinProcess(DATE_TIME_BUF *pDate)
// 1분이 됬을때 처리 내용
// 온도, 습도, 기압 일조, 일사 10분 누적, 및 1분 최소 최고 처리
// 일조 아루 총 누적에 처리
// 강수량 1분
{
  uint32_t nIdx;
  SYSTEM_INFO_AWS *pSystem;
  AWS_DATA_STRUCT *pAws;
  short snow;

  pSystem = &Sysinfo;
  pAws = &mMinAws;

  pAws->mDate.cMonth = pDate->Month;  // 월일 시분만 기록
  pAws->mDate.cDay = pDate->Day;
  pAws->mDate.cHour = pDate->Hour;
  pAws->mDate.cMin = pDate->Min;

  // 온도
  AwsMinMaxTotSave(&pAws->mTemperature, &pSystem->mTempBuf[MIN1_PROC],
                   mRealAws.mTemperature.sReal);
  pSystem->mTempBuf[MIN10_PROC].lTot +=
      pAws->mTemperature.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mTempBuf[MIN10_PROC].sAddCnt++;

  // 기압
  AwsMinMaxTotSave(&pAws->mBarometric, &pSystem->mBaroBuf[MIN1_PROC],
                   mRealAws.mBarometric.sReal);
  pSystem->mBaroBuf[MIN10_PROC].lTot += pAws->mBarometric.sReal;
  pSystem->mBaroBuf[MIN10_PROC].sAddCnt++;

  // 습도
  AwsMinMaxTotSave(&pAws->mHumidity, &pSystem->mHumidBuf[MIN1_PROC],
                   mRealAws.mHumidity.sReal);
  pSystem->mHumidBuf[MIN10_PROC].lTot += pAws->mHumidity.sReal;
  pSystem->mHumidBuf[MIN10_PROC].sAddCnt++;

  pSystem->mSun[MIN10_PROC].nSunshineTot +=
      pSystem->mSun[MIN1_PROC].nSunshineTot;
  pSystem->mSun[MIN10_PROC].nSolarTot +=
      pSystem->mSun[MIN1_PROC].nSolarTot / 1000000;  // 단위변환 W/M2 -> MJ/M2

  // 풍향 풍속
  WindMinMaxAvgSave(&pAws->mWind, &pSystem->mWind[MIN1_PROC], &mRealAws.mWind);
  DircTouvConv(pAws->mWind.mDirection.sReal, pAws->mWind.mSpeed.sReal,
               &pSystem->mWind[MIN10_PROC].uTot,
               &pSystem->mWind[MIN10_PROC].vTot);
  pSystem->mWind[MIN10_PROC].lSpeedTot += pAws->mWind.mSpeed.sReal;  // 1분 "
  pSystem->mWind[MIN10_PROC].sAddCnt++;

  // 일사 일조
  pAws->mSunshine.sReal =
      pSystem->mSun[MIN1_PROC].nSunshineTot;      // 일조 1분 누적값
  pAws->mSunshine.sMax += pAws->mSunshine.sReal;  // 하루 총 일조
  pSystem->mSun[MIN1_PROC].nSunshineTot = 0;

  pAws->mSolarRad.sReal =
      pSystem->mSun[MIN1_PROC].nSolarTot / 1000;  // 일사 1분   누적값  KJ/m2
  pSystem->mSun[MIN1_PROC].nSolarTot = 0;
  pSystem->mSun[MIN10_PROC].nSolarTot += pAws->mSolarRad.sReal;
  pAws->mSolarRad.sMax += pAws->mSolarRad.sReal;  // 하루 총 일사
  mRealAws.mSolarRad.sMax = pAws->mSolarRad.sMax;
  m10MinAws.mSolarRad.sMax = pAws->mSolarRad.sMax;
  mHourAws.mSolarRad.sMax = pAws->mSolarRad.sMax;

  // 지중  온도 2017.04.03
  AwsMinMaxTotSave(&pAws->mSoilTemp5cm, &pSystem->mSoil5Buf[MIN1_PROC],
                   mRealAws.mSoilTemp5cm.sReal);
  pSystem->mSoil5Buf[MIN10_PROC].lTot +=
      pAws->mSoilTemp5cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil5Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp10cm, &pSystem->mSoil10Buf[MIN1_PROC],
                   mRealAws.mSoilTemp10cm.sReal);
  pSystem->mSoil10Buf[MIN10_PROC].lTot +=
      pAws->mSoilTemp10cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil10Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp20cm, &pSystem->mSoil20Buf[MIN1_PROC],
                   mRealAws.mSoilTemp20cm.sReal);
  pSystem->mSoil20Buf[MIN10_PROC].lTot +=
      pAws->mSoilTemp20cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil20Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp30cm, &pSystem->mSoil30Buf[MIN1_PROC],
                   mRealAws.mSoilTemp30cm.sReal);
  pSystem->mSoil30Buf[MIN10_PROC].lTot +=
      pAws->mSoilTemp30cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil30Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp50cm, &pSystem->mSoil50Buf[MIN1_PROC],
                   mRealAws.mSoilTemp50cm.sReal);
  pSystem->mSoil50Buf[MIN10_PROC].lTot +=
      pAws->mSoilTemp50cm.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil50Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp1_0m, &pSystem->mSoil100Buf[MIN1_PROC],
                   mRealAws.mSoilTemp1_0m.sReal);
  pSystem->mSoil100Buf[MIN10_PROC].lTot +=
      pAws->mSoilTemp1_0m.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil100Buf[MIN10_PROC].sAddCnt++;

  AwsMinMaxTotSave(&pAws->mSoilTemp1_5m, &pSystem->mSoil150Buf[MIN1_PROC],
                   mRealAws.mSoilTemp1_5m.sReal);
  pSystem->mSoil150Buf[MIN10_PROC].lTot +=
      pAws->mSoilTemp1_5m.sReal;  // 1분 평균을 구한 값을 10분 누적에 더한다
  pSystem->mSoil150Buf[MIN10_PROC].sAddCnt++;
  // 지중온도 처리 끝

  // 강수량 처리
  // 2010. 08. 28. 수정
  //    pAws->mRainFall.sReal   = pSystem->mRain.sMinRain; // 1분 강수량
  pAws->mRainFall.sMax = pSystem->mRain.sHourRain;
  pAws->mRainFall.sMin = (uint16_t)pSystem->mNVram.nMonthRain;  // 월간강수량( " )
  pAws->mRainFall.sSpec = (uint16_t)pSystem->mNVram.nYearRain;  // 연간강수량( " )
  pSystem->mRain.sMinRain = 0;                                // 1분 강수량

  pAws->mSnowFall.sReal = mRealAws.mSnowFall.sReal;
  nIdx = (pDate->Min + 59) % 60;

 // memcpy((char *)&mMinAwsLog[nIdx], (char *)pAws, sizeof(AWS_DATA_STRUCT));

  os_write_sensorData(pDate, pAws, sizeof(AWS_DATA_STRUCT), LOGGING_AWS,1);
}

void Min10Process(void)
// 10분이 됬을때 처리 내용
// 온도, 습도, 기압 일조, 일사 1시간 누적, 및 10분 최소 최고 처리
{
  SYSTEM_INFO_AWS *pSystem;
  AWS_DATA_STRUCT *pAws;
  short shSnow;

  pSystem = &Sysinfo;
  pAws = &m10MinAws;

  // 온도
  AwsMinMaxTotSave(&pAws->mTemperature, &pSystem->mTempBuf[MIN10_PROC],
                   mRealAws.mTemperature.sReal);
  pSystem->mTempBuf[HOUR_PROC].lTot += pAws->mTemperature.sReal;
  pSystem->mTempBuf[HOUR_PROC].sAddCnt++;

  // 기압
  AwsMinMaxTotSave(&pAws->mBarometric, &pSystem->mBaroBuf[MIN10_PROC],
                   mRealAws.mBarometric.sReal);
  pSystem->mBaroBuf[HOUR_PROC].lTot += pAws->mBarometric.sReal;
  pSystem->mBaroBuf[HOUR_PROC].sAddCnt++;

  // 습도
  AwsMinMaxTotSave(&pAws->mHumidity, &pSystem->mHumidBuf[MIN10_PROC],
                   mRealAws.mHumidity.sReal);
  pSystem->mHumidBuf[HOUR_PROC].lTot += pAws->mHumidity.sReal;
  pSystem->mHumidBuf[HOUR_PROC].sAddCnt++;

  // 풍향 풍속
  WindMinMaxAvgSave(&pAws->mWind, &pSystem->mWind[MIN10_PROC],
                    &mRealAws.mWind);  // 10분
  DircTouvConv(pAws->mWind.mDirection.sReal, pAws->mWind.mSpeed.sReal,
               &pSystem->mWind[HOUR_PROC].uTot,
               &pSystem->mWind[HOUR_PROC].vTot);
  pSystem->mWind[HOUR_PROC].lSpeedTot += pAws->mWind.mSpeed.sReal;  // 1시간 "
  pSystem->mWind[HOUR_PROC].sAddCnt++;

  // 일사 일조
  pSystem->mSun[HOUR_PROC].nSolarTot += pSystem->mSun[MIN10_PROC].nSolarTot;
  pSystem->mSun[HOUR_PROC].nSunshineTot +=
      pSystem->mSun[MIN10_PROC].nSunshineTot;

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
  pAws->mRainFall.sReal = pSystem->mRain.s10MinRain;  // 10분 강수량
  pAws->mRainFall.sMax = pSystem->mRain.sHourRain;
  pSystem->mRain.s10MinRain = 0;  // 10분 강수량

#if 0 
// 2010. 11. 30. 수정 적설량 처리
	shSnow = mRealAws.mSnowFall.sReal - pSystem->shSnowFallOld;											// 실 적설에서 예전적설(10분전)을 뺀다
	if(shSnow >= 0)																							// +인 경우는 눈이 온것임
	{
		m10MinAws.mSnowFall.sReal = shSnow;	
	}
	else
	{
		m10MinAws.mSnowFall.sReal = 0;	
	}	
	pSystem->shSnowFallOld 	= mRealAws.mSnowFall.sReal;														// 현재 적설위치를 옮겨 놓는다.
#else
  m10MinAws.mSnowFall.sReal = mRealAws.mSnowFall.sReal;
#endif
}

void HourProcess(DATE_TIME_BUF *pDate)
{
  SYSTEM_INFO_AWS *pSystem;
  AWS_DATA_STRUCT *pAws;

  pSystem = &Sysinfo;
  pAws = &mHourAws;

  // 온도
  AwsMinMaxTotSave(&pAws->mTemperature, &pSystem->mTempBuf[HOUR_PROC],
                   mRealAws.mTemperature.sReal);
  // 기압
  AwsMinMaxTotSave(&pAws->mBarometric, &pSystem->mBaroBuf[HOUR_PROC],
                   mRealAws.mBarometric.sReal);
  // 습도
  AwsMinMaxTotSave(&pAws->mHumidity, &pSystem->mHumidBuf[HOUR_PROC],
                   mRealAws.mHumidity.sReal);
  // 풍향 풍속
  WindMinMaxAvgSave(&pAws->mWind, &pSystem->mWind[HOUR_PROC], &mRealAws.mWind);
  // 일사 일조
  pAws->mSunshine.sReal = pSystem->mSun[HOUR_PROC].nSunshineTot;
  pSystem->mSun[HOUR_PROC].nSunshineTot = 0;
  pAws->mSolarRad.sReal = pSystem->mSun[HOUR_PROC].nSolarTot;
  pSystem->mSun[HOUR_PROC].nSolarTot = 0;

  // 지중온도 처리 2017.04.03

  AwsMinMaxTotSave(&pAws->mSoilTemp5cm, &pSystem->mSoil5Buf[HOUR_PROC],
                   mRealAws.mSoilTemp5cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp10cm, &pSystem->mSoil10Buf[HOUR_PROC],
                   mRealAws.mSoilTemp10cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp20cm, &pSystem->mSoil20Buf[HOUR_PROC],
                   mRealAws.mSoilTemp20cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp30cm, &pSystem->mSoil30Buf[HOUR_PROC],
                   mRealAws.mSoilTemp30cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp50cm, &pSystem->mSoil50Buf[HOUR_PROC],
                   mRealAws.mSoilTemp50cm.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp1_0m, &pSystem->mSoil100Buf[HOUR_PROC],
                   mRealAws.mSoilTemp1_0m.sReal);
  AwsMinMaxTotSave(&pAws->mSoilTemp1_5m, &pSystem->mSoil150Buf[HOUR_PROC],
                   mRealAws.mSoilTemp1_5m.sReal);
  // 지중 온도 처리 끝

  // 강수량 처리
  // 2010. 08. 28. 수정
  //    pAws->mRainFall.sReal    = pSystem->mRain.sHourRain; // 1시간 강수량
  pSystem->mRain.sHourRain = 0;  // 1시간 강수량

  // File로 Save한다
  //HourLogWrite(pDate, &mMinAwsLog[0]);
}

void DayProcess(void)
{
  SYSTEM_INFO_AWS *pSystem;

  pSystem = &Sysinfo;
  // 전일 강수량 으로 기록
  // 일간 강우량 Clear
  // 일간 돌풍속 최대 값 Clear
  // 온도 최대 최소값 현재 값으로 기록
  // 습도   "
  // 기압   "
  // 지중온도 "			2017.03.30 추가
  // 일사   Clear 		2017.03.30 추가

  mRealAws.mWind.mDirection.sMax = 0;  // mRealAws.mWind.mDirection.sReal;
  mRealAws.mWind.mSpeed.sMax = 0;      // mRealAws.mWind.mSpeed.sReal;

  mRealAws.mTemperature.sMin = mRealAws.mTemperature.sReal;
  mRealAws.mBarometric.sMin = mRealAws.mBarometric.sReal;
  mRealAws.mHumidity.sMin = mRealAws.mHumidity.sReal;

  mRealAws.mSoilTemp5cm.sMin = mRealAws.mSoilTemp5cm.sReal;
  mRealAws.mSoilTemp10cm.sMin = mRealAws.mSoilTemp10cm.sReal;
  mRealAws.mSoilTemp20cm.sMin = mRealAws.mSoilTemp20cm.sReal;
  mRealAws.mSoilTemp30cm.sMin = mRealAws.mSoilTemp30cm.sReal;
  mRealAws.mSoilTemp50cm.sMin = mRealAws.mSoilTemp50cm.sReal;
  mRealAws.mSoilTemp1_0m.sMin = mRealAws.mSoilTemp1_0m.sReal;
  mRealAws.mSoilTemp1_5m.sMin = mRealAws.mSoilTemp1_5m.sReal;

  mRealAws.mTemperature.sMax = mRealAws.mTemperature.sReal;
  mRealAws.mBarometric.sMax = mRealAws.mBarometric.sReal;
  mRealAws.mHumidity.sMax = mRealAws.mHumidity.sReal;

  mRealAws.mSoilTemp5cm.sMax = mRealAws.mSoilTemp5cm.sReal;
  mRealAws.mSoilTemp10cm.sMax = mRealAws.mSoilTemp10cm.sReal;
  mRealAws.mSoilTemp20cm.sMax = mRealAws.mSoilTemp20cm.sReal;
  mRealAws.mSoilTemp30cm.sMax = mRealAws.mSoilTemp30cm.sReal;

  mRealAws.mSunshine.sMax = 0;  // 하루 총 일조
  mMinAws.mSunshine.sMax = 0;   // 하루 총 일조
  mMinAws.mSolarRad.sMax = 0;   // 하루 총 일사

  // 강수량 처리
  pSystem->mRain.sBefDayRain = pSystem->mRain.sDayRain;  // 전일강수량 <-
  pSystem->mRain.sDayRain = 0;
  pSystem->mRain.sDayCount=0;
  pSystem->mRain.sDayCountValue = 0;
  pSystem->mRain.sDayCountFlag = 1;
}

void MonthProcess(void)
{
  SYSTEM_INFO_AWS *pSystem;

  pSystem = &Sysinfo;

  g_config_nvm.monthRain = 0;
  WRITE_NVM(monthRain);

  g_config_nvm.monthSunshine = 0;
  WRITE_NVM(monthSunshine);

}

void DircTouvConv(uint16_t sDirc, uint16_t sSpeed, float *dir_u, float *dir_v)
{
  float fAngle;

  fAngle = (float)sDirc / 10;  // 3599를 359

#if 0
/* Memory Table에의한 연산속도는 250us정도 소요됨 */
    if(nAngle <= 90)
    {
        nTt = (90 - nAngle);
        *dir_u += (float)sSpeed * sin_tbl[90 - nTt];
        *dir_v += (float)sSpeed * sin_tbl[nTt];
    }
    else
    if(nAngle <= 180)
    {
        nTt = (nAngle - 90); 
        *dir_u += (float)sSpeed * sin_tbl[90 - nTt];
        *dir_v += (float)sSpeed * sin_tbl[nTt] * -1.0;
    }
    else
    if(nAngle <= 270)
    {

        nTt = (270 - nAngle); 
        *dir_u += (float)sSpeed * sin_tbl[90 - nTt] * -1.0;
        *dir_v += (float)sSpeed * sin_tbl[nTt] * -1.0;
    }        
    else
    {
        nTt = (nAngle - 270); 
        *dir_u += (float)sSpeed * sin_tbl[90 - nTt] * -1.0;
        *dir_v += (float)sSpeed * sin_tbl[nTt];
    }
#endif

#if 1
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
#endif
}
// AWS담위 측정값 *10
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



void scheduleProcess_init(void)
{
  memcpy((char *)&OldDate, (char *)&Date_Time, sizeof(Date_Time));
}




void schedule_process(DATE_TIME_BUF *pDate)
{
  DATE_TIME_BUF *pOldDate;
  SYSTEM_INFO_AWS *pSystem;



  pSystem = &Sysinfo;
  pOldDate = &OldDate;

  if (pDate->Sec != pOldDate->Sec)
  {  
    SecProcess();
    if (pDate->Sec % 10 == 0)
    { 
      Sec10Process(); // 매 10초 마다 처리
    }
    pOldDate->Sec = pDate->Sec;
  }


    if (pDate->Min != pOldDate->Min)
    { /* 분이 바귈때 처리						*/
      MinProcess(pDate);
      update_old_kma_1min();
      
      pOldDate->Min = pDate->Min;
      if (pDate->Min % 10 == 0)
      {  // 매 10분 마다 처리
        Min10Process();
      }
    }

    if (pDate->Hour != pOldDate->Hour)
    { /* 시간이 바뀔때 처리					*/
      HourProcess(pDate);
      pOldDate->Hour = pDate->Hour;
      // 매 시간 마다 SD CARD에 데이타를 기록
    }

    if (pDate->Day != pOldDate->Day)
    {
      DayProcess();
      pOldDate->Day = pDate->Day;
    }

    if (pDate->Month != pOldDate->Month)
    { /* 달이 바뀔때 처리						*/
      // 월간 강수량 기록
      // 금월 월간 강수량 삭제
      MonthProcess();
      pOldDate->Month = pDate->Month;
    }

    if (pDate->Year != pOldDate->Year)
    { 
      g_config_nvm.yearRain = 0;
      WRITE_NVM(yearRain);
      g_config_nvm.yearSunshine = 0;
      WRITE_NVM(yearSunshine);
      pOldDate->Year = pDate->Year;
    }

}

void update_old_kma_1min(void)
{
  //온도
  g_kma_1min_ex.temperature.data = mMinAws.mTemperature.sReal;
  g_kma_1min_ex.temperature.max = mMinAws.mTemperature.sMax;
  g_kma_1min_ex.temperature.min = mMinAws.mTemperature.sMin;

  //기압
  g_kma_1min_ex.pressure.max = mMinAws.mBarometric.sMax;
  g_kma_1min_ex.pressure.min = mMinAws.mBarometric.sMin;
  g_kma_1min_ex.pressure.data = mMinAws.mBarometric.sReal;

  //습도도
  g_kma_1min_ex.relative_humidity.data = mMinAws.mHumidity.sReal;
  g_kma_1min_ex.relative_humidity.max = mMinAws.mHumidity.sMin;
  g_kma_1min_ex.relative_humidity.min = mMinAws.mHumidity.sMin;

 //풍향
  g_kma_1min_ex.wind_direction_avg.data = mMinAws.mWind.mDirection.sReal;
  g_kma_1min_ex.wind_direction_avg.max = mMinAws.mWind.mDirection.sMax;

 //풍속
  g_kma_1min_ex.wind_speed_avg.data = mMinAws.mWind.mSpeed.sReal;
  g_kma_1min_ex.wind_speed_avg.max = mMinAws.mWind.mSpeed.sMax;

//일조
  g_kma_1min_ex.sunshine_duration.data = mMinAws.mSunshine.sReal;
  g_kma_1min_ex.sunshine_duration.max = mMinAws.mSunshine.sMax;//하루 총 일조

  //일사
  g_kma_1min_ex.solar_radiation.data = mMinAws.mSolarRad.sReal;
  g_kma_1min_ex.solar_radiation.max = mMinAws.mSolarRad.sMax;  // 일간

  //지중 온도
  g_kma_1min_ex.soil_temperature_5cm.data = mMinAws.mSoilTemp5cm.sReal;
  g_kma_1min_ex.soil_temperature_5cm.max = mMinAws.mSoilTemp5cm.sMax;
  g_kma_1min_ex.soil_temperature_5cm.min = mMinAws.mSoilTemp5cm.sMin;

  g_kma_1min_ex.soil_temperature_10cm.data = mMinAws.mSoilTemp10cm.sReal;
  g_kma_1min_ex.soil_temperature_10cm.max = mMinAws.mSoilTemp10cm.sMax;
  g_kma_1min_ex.soil_temperature_10cm.min = mMinAws.mSoilTemp10cm.sMin;

  g_kma_1min_ex.soil_temperature_20cm.data = mMinAws.mSoilTemp20cm.sReal;
  g_kma_1min_ex.soil_temperature_20cm.max = mMinAws.mSoilTemp20cm.sMax;
  g_kma_1min_ex.soil_temperature_20cm.min = mMinAws.mSoilTemp20cm.sMin;

  g_kma_1min_ex.soil_temperature_30cm.data = mMinAws.mSoilTemp30cm.sReal;
  g_kma_1min_ex.soil_temperature_30cm.max = mMinAws.mSoilTemp30cm.sMax;
  g_kma_1min_ex.soil_temperature_30cm.min = mMinAws.mSoilTemp30cm.sMin;

  g_kma_1min_ex.soil_temperature_50cm.data = mMinAws.mSoilTemp50cm.sReal;
  g_kma_1min_ex.soil_temperature_50cm.max = mMinAws.mSoilTemp50cm.sMax;
  g_kma_1min_ex.soil_temperature_50cm.min = mMinAws.mSoilTemp50cm.sMin;



  g_kma_1min_ex.soil_temperature_1m.data = mMinAws.mSoilTemp1_0m.sReal;
  g_kma_1min_ex.soil_temperature_1m.max = mMinAws.mSoilTemp1_0m.sMax;
  g_kma_1min_ex.soil_temperature_1m.min = mMinAws.mSoilTemp1_0m.sMin;

  g_kma_1min_ex.soil_temperature_1_5m.data = mMinAws.mSoilTemp1_5m.sReal;
  g_kma_1min_ex.soil_temperature_1_5m.max = mMinAws.mSoilTemp1_5m.sMax;
  g_kma_1min_ex.soil_temperature_1_5m.min = mMinAws.mSoilTemp1_5m.sMin;


  g_kma_1min_ex.wind_speed_instant.data = mMinAws.mWind.mSpeed.sMax;
  g_kma_1min_ex.wind_direction_instant.data = mMinAws.mWind.mDirection.sMax;


  g_kma_1min_ex.precipitation.data = mMinAws.mRainFall.sReal;
  g_kma_1min_ex.precipitation.max = mMinAws.mRainFall.sMax;//시간당 강수량량
  g_kma_1min_ex.precipitation.min = mMinAws.mRainFall.sMin;//월간 강수량
  g_kma_1min_ex.precipitation.spec = mMinAws.mRainFall.sSpec;//연간 강수량

  g_kma_1min_ex.precipitation_presence.data = mMinAws.mRainDetect.sReal;  // 우량 감지

  g_kma_1min_ex.snowfall.data = mMinAws.mSnowFall.sReal;


}

void update_old_kma_hour(void)
{
  // 온도
  g_kma_1min_ex.temperature.data = mMinAws.mTemperature.sReal;
  g_kma_1min_ex.temperature.max = mMinAws.mTemperature.sMax;
  g_kma_1min_ex.temperature.min = mMinAws.mTemperature.sMin;

  // 기압
  g_kma_1min_ex.pressure.max = mMinAws.mBarometric.sMax;
  g_kma_1min_ex.pressure.min = mMinAws.mBarometric.sMin;
  g_kma_1min_ex.pressure.data = mMinAws.mBarometric.sReal;

  // 습도
  g_kma_1min_ex.relative_humidity.data = mMinAws.mHumidity.sReal;
  g_kma_1min_ex.relative_humidity.max = mMinAws.mHumidity.sMin;
  g_kma_1min_ex.relative_humidity.min = mMinAws.mHumidity.sMin;

  // 풍향
  g_kma_1min_ex.wind_direction_avg.data = mMinAws.mWind.mDirection.sReal;

//순간 풍향
  g_kma_1min_ex.wind_direction_instant.max = mMinAws.mWind.mDirection.sMax;


  // 풍속
  g_kma_1min_ex.wind_speed_avg.data = mMinAws.mWind.mSpeed.sReal;

  g_kma_1min_ex.wind_speed_instant.max = mMinAws.mWind.mSpeed.sMax;



  // 일조
  g_kma_1min_ex.sunshine_duration.data = mMinAws.mSunshine.sReal;
  g_kma_1min_ex.sunshine_duration.max = mMinAws.mSunshine.sMax;  // 하루 총 일조

  // 일사
  g_kma_1min_ex.solar_radiation.data = mMinAws.mSolarRad.sReal;
  g_kma_1min_ex.solar_radiation.max = mMinAws.mSolarRad.sMax;  // 일간

  // 지중 온도
  g_kma_1min_ex.soil_temperature_5cm.data = mMinAws.mSoilTemp5cm.sReal;
  g_kma_1min_ex.soil_temperature_5cm.max = mMinAws.mSoilTemp5cm.sMax;
  g_kma_1min_ex.soil_temperature_5cm.min = mMinAws.mSoilTemp5cm.sMin;

  g_kma_1min_ex.soil_temperature_10cm.data = mMinAws.mSoilTemp10cm.sReal;
  g_kma_1min_ex.soil_temperature_10cm.max = mMinAws.mSoilTemp10cm.sMax;
  g_kma_1min_ex.soil_temperature_10cm.min = mMinAws.mSoilTemp10cm.sMin;

  g_kma_1min_ex.soil_temperature_20cm.data = mMinAws.mSoilTemp20cm.sReal;
  g_kma_1min_ex.soil_temperature_20cm.max = mMinAws.mSoilTemp20cm.sMax;
  g_kma_1min_ex.soil_temperature_20cm.min = mMinAws.mSoilTemp20cm.sMin;

  g_kma_1min_ex.soil_temperature_30cm.data = mMinAws.mSoilTemp30cm.sReal;
  g_kma_1min_ex.soil_temperature_30cm.max = mMinAws.mSoilTemp30cm.sMax;
  g_kma_1min_ex.soil_temperature_30cm.min = mMinAws.mSoilTemp30cm.sMin;

  g_kma_1min_ex.soil_temperature_50cm.data = mMinAws.mSoilTemp50cm.sReal;
  g_kma_1min_ex.soil_temperature_50cm.max = mMinAws.mSoilTemp50cm.sMax;
  g_kma_1min_ex.soil_temperature_50cm.min = mMinAws.mSoilTemp50cm.sMin;

  g_kma_1min_ex.soil_temperature_1m.data = mMinAws.mSoilTemp1_0m.sReal;
  g_kma_1min_ex.soil_temperature_1m.max = mMinAws.mSoilTemp1_0m.sMax;
  g_kma_1min_ex.soil_temperature_1m.min = mMinAws.mSoilTemp1_0m.sMin;

  g_kma_1min_ex.soil_temperature_1_5m.data = mMinAws.mSoilTemp1_5m.sReal;
  g_kma_1min_ex.soil_temperature_1_5m.max = mMinAws.mSoilTemp1_5m.sMax;
  g_kma_1min_ex.soil_temperature_1_5m.min = mMinAws.mSoilTemp1_5m.sMin;


}

SYSTEM_INFO_AWS *get_system_info_aws(void)
{
  return &Sysinfo;
}