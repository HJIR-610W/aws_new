#include <math.h>
#include <string.h>

#include "schedule.h"
#include "util_time.h"
#include "config_nvm.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "old_aws_define.h"
#include "FreeRTOS.h" // pvPortMalloc, vPortFree 사용 시 필요
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
#include "util_crc16_ccitt.h"
#define D2R 3.14159265 / 180.0
#define R2D 180.0 / 3.14159265

#define MAXWDSPEED 100.0


#pragma location = "SRAM_section"
AWS_DATA_STRUCT mRealAws;   // 실시간 자료
#pragma location = "SRAM_section"
AWS_DATA_STRUCT mMinAws;    // 1분 자료 이값은 실시간 변경하면 안된다. 분이 바뀔때만 갱신되어야함
#pragma location = "SRAM_section"
AWS_DATA_STRUCT m10MinAws;  // 10분 자료
#pragma location = "SRAM_section"
AWS_DATA_STRUCT mHourAws;   // 1시간 자료
#pragma location = "SRAM_section"
AWS_DATA_STRUCT mDayAws; // 일간 자료 

void SecProcess(void);

void MinProcess(DATE_TIME_BUF *pDate);
void Min10Process(void);
void HourProcess(DATE_TIME_BUF *pDate);
void DayProcess(void);
void MonthProcess(void);
float UVToSpeed(float u_tmp, float v_tmp);
void save_min_data(DATE_TIME_BUF *p_time);
void update_kma_data(eAWS_DATA_MIN_t min, DATE_TIME_BUF *p_time);

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
}

void SecProcess(void)
{

  AWS_DATA_STRUCT *pAws;

  pAws = &mRealAws;

    //온도 1분 평균 최대 최소
  calculate_data_avg(eAVG_TEMPERATURE, g_avg_1min, pAws->mTemperature.sReal);
  calculate_data_min_max(eAVG_TEMPERATURE, g_1min_min_max, pAws->mTemperature.sReal);
  //기압
  calculate_data_avg(eAVG_PRESSURE, g_avg_1min, pAws->mBarometric.sReal);
  calculate_data_min_max(eAVG_PRESSURE, g_1min_min_max, pAws->mBarometric.sReal);
  //습도
  calculate_data_avg(eAVG_RELATIVE_HUMIDITY, g_avg_1min, pAws->mHumidity.sReal);
  calculate_data_min_max(eAVG_RELATIVE_HUMIDITY, g_1min_min_max, pAws->mHumidity.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_5CM, g_avg_1min, pAws->mSoilTemp5cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_5CM, g_1min_min_max, pAws->mSoilTemp5cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_10CM, g_avg_1min, pAws->mSoilTemp10cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_10CM, g_1min_min_max, pAws->mSoilTemp10cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_20CM, g_avg_1min, pAws->mSoilTemp20cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_20CM, g_1min_min_max, pAws->mSoilTemp20cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_30CM, g_avg_1min, pAws->mSoilTemp30cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_30CM, g_1min_min_max, pAws->mSoilTemp30cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_50CM, g_avg_1min, pAws->mSoilTemp50cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_50CM, g_1min_min_max, pAws->mSoilTemp50cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_100CM, g_avg_1min, pAws->mSoilTemp1_0m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_100CM, g_1min_min_max, pAws->mSoilTemp1_0m.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_150CM, g_avg_1min, pAws->mSoilTemp1_5m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_150CM, g_1min_min_max, pAws->mSoilTemp1_5m.sReal);

  //우량
  if(g_rainfall.current)
  {
    uint16_t rain = g_rainfall.current ;
    g_rainfall.current = 0;
    g_rainfall.today   += rain;
    g_rainfall.min     += rain;
    g_rainfall.ten_min += rain;
    g_rainfall.hourly  += rain;
    g_rainfall.monthly += rain;
    g_rainfall.yearly  += rain;

  }
  
  mRealAws.mRainFall.sReal      = g_rainfall.today;  
  mRealAws.mRainFall.sHourRain  = g_rainfall.hourly; 
  mRealAws.mRainFall.sMonthRain = g_rainfall.monthly;  
  mRealAws.mRainFall.sYearRain  = g_rainfall.yearly;

  m10MinAws.mRainFall.sReal = g_rainfall.today;
  mHourAws.mRainFall.sReal = g_rainfall.today;

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
    g_solar_radiation.min_acc += mRealAws.mSolarRad.sReal;
    g_solar_radiation.hourly  += mRealAws.mSolarRad.sReal;
    g_solar_radiation.today   += mRealAws.mSolarRad.sReal;
  }

}




#define MIN_LIMIT 10000
#define MAX_LIMIT 0
/*
1분이 됬을때 처리 내용
온도, 습도, 기압 일조, 일사 10분 누적, 및 1분 최소 최고 처리
일조 아루 총 누적에 처리
강수량 1분
*/

void MinProcess(DATE_TIME_BUF *pDate)
{
  float wind_speed_avg;
  float wind_direction_avg;

  AWS_DATA_STRUCT *pAws;


  pAws = &mMinAws;

  pAws->mDate.cMonth = pDate->Month;  // 월일 시분만 기록
  pAws->mDate.cDay = pDate->Day;
  pAws->mDate.cHour = pDate->Hour;
  pAws->mDate.cMin = pDate->Min;

  //온도 1분자료 업데이트 
  pAws->mTemperature.sReal = read_data_average(eAVG_TEMPERATURE, g_avg_1min);
  pAws->mTemperature.sMin = read_data_min(eAVG_TEMPERATURE, g_1min_min_max, MIN_LIMIT);
  pAws->mTemperature.sMax = read_data_max(eAVG_TEMPERATURE, g_1min_min_max, 0);

  //습도
  pAws->mHumidity.sReal = read_data_average(eAVG_RELATIVE_HUMIDITY, g_avg_1min);
  pAws->mHumidity.sMin = read_data_min(eAVG_RELATIVE_HUMIDITY, g_1min_min_max, MIN_LIMIT);
  pAws->mHumidity.sMax = read_data_max(eAVG_RELATIVE_HUMIDITY, g_1min_min_max, MAX_LIMIT);

  //기압
  pAws->mBarometric.sReal = read_data_average(eAVG_PRESSURE, g_avg_1min);
  pAws->mBarometric.sMin = read_data_min(eAVG_PRESSURE, g_1min_min_max, MIN_LIMIT);
  pAws->mBarometric.sMax = read_data_max(eAVG_PRESSURE, g_1min_min_max, MAX_LIMIT);

  // 지면온도
  pAws->mGndTemp.sReal = read_data_average(eAVG_GROUND_TEMPERATURE, g_avg_1min);
  pAws->mGndTemp.sMin = read_data_min(eAVG_GROUND_TEMPERATURE, g_1min_min_max, MIN_LIMIT);
  pAws->mGndTemp.sMax = read_data_max(eAVG_GROUND_TEMPERATURE, g_1min_min_max, MAX_LIMIT);

  // 초상온도
  pAws->mGrassTemp.sReal = read_data_average(eAVG_SURFACE_TEMPERATURE, g_avg_1min);
  pAws->mGrassTemp.sMin = read_data_min(eAVG_SURFACE_TEMPERATURE, g_1min_min_max, MIN_LIMIT);
  pAws->mGrassTemp.sMax = read_data_max(eAVG_SURFACE_TEMPERATURE, g_1min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp5cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_5CM, g_avg_1min);
  pAws->mSoilTemp5cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_5CM, g_1min_min_max, MIN_LIMIT);
  pAws->mSoilTemp5cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_5CM, g_1min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp10cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_10CM, g_avg_1min);
  pAws->mSoilTemp10cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_10CM, g_1min_min_max, MIN_LIMIT);
  pAws->mSoilTemp10cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_10CM, g_1min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp20cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_20CM, g_avg_1min);
  pAws->mSoilTemp20cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_20CM, g_1min_min_max, MIN_LIMIT);
  pAws->mSoilTemp20cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_20CM, g_1min_min_max, MAX_LIMIT);
  // 지중온도
  pAws->mSoilTemp30cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_30CM, g_avg_1min);
  pAws->mSoilTemp30cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_30CM, g_1min_min_max, MIN_LIMIT);
  pAws->mSoilTemp30cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_30CM, g_1min_min_max, MAX_LIMIT);
  // 지중온도
  pAws->mSoilTemp50cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_50CM, g_avg_1min);
  pAws->mSoilTemp50cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_50CM, g_1min_min_max, MIN_LIMIT);
  pAws->mSoilTemp50cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_50CM, g_1min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp1_0m.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_100CM, g_avg_1min);
  pAws->mSoilTemp1_0m.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_100CM, g_1min_min_max, MIN_LIMIT);
  pAws->mSoilTemp1_0m.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_100CM, g_1min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp1_5m.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_150CM, g_avg_1min);
  pAws->mSoilTemp1_5m.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_150CM, g_1min_min_max, MIN_LIMIT);
  pAws->mSoilTemp1_5m.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_150CM, g_1min_min_max, MAX_LIMIT);

  // 풍향 풍속
  calculate_wind_avg_1min(&wind_speed_avg, &wind_direction_avg);
  pAws->mWind.mDirection.sReal = (uint16_t)(wind_direction_avg * 10);
  pAws->mWind.mSpeed.sReal = (uint16_t)(wind_speed_avg * 10);
  wind_avg_1min_init();
  pAws->mWind.mSpeed.sMax = read_wind_speed_max(eWIND_MAX_1MIN);
  pAws->mWind.mDirection.sMax = read_wind_direction_max(eWIND_MAX_1MIN);
  //적설
  pAws->mSnowFall.sReal = mRealAws.mSnowFall.sReal;
  //일조 1분 누적
  pAws->mSunshine.sReal = g_sunshine.min;
  //일조
  pAws->mSunshine.sMax = g_sunshine.today;

  // 일사 1분   누적값  KJ/m2
  pAws->mSolarRad.sReal = g_solar_radiation.min_acc / 1000; 
  mRealAws.mSolarRad.sMax = g_solar_radiation.today;


  // 강수량
  pAws->rain_1min = g_rainfall.min ;
  pAws->mRainFall.sReal = g_rainfall.today;
  pAws->mRainFall.sHourRain = g_rainfall.hourly;
  pAws->mRainFall.sMonthRain = g_rainfall.monthly;
  pAws->mRainFall.sYearRain = g_rainfall.yearly;

  g_solar_radiation.sunshine_r_1min = g_solar_radiation.min_acc;


  for (int i = 0; i < 8; i++)
  {
    pAws->kma3_sensor_status[i] = mRealAws.kma3_sensor_status[i];
  }

  pAws->mStatus = mRealAws.mStatus;

  
  os_save_aws_data(pDate, pAws, sizeof(AWS_DATA_STRUCT), LOGGING_AWS, 1);

  // 온도
  calculate_data_avg(eAVG_TEMPERATURE, g_avg_10min, pAws->mTemperature.sReal);
  calculate_data_min_max(eAVG_TEMPERATURE, g_10min_min_max, pAws->mTemperature.sReal);
  //습도
  calculate_data_avg(eAVG_RELATIVE_HUMIDITY, g_avg_10min, pAws->mHumidity.sReal);
  calculate_data_min_max(eAVG_RELATIVE_HUMIDITY, g_10min_min_max, pAws->mHumidity.sReal);
  // 기압
  calculate_data_avg(eAVG_PRESSURE, g_avg_10min, pAws->mBarometric.sReal);
  calculate_data_min_max(eAVG_PRESSURE, g_10min_min_max, pAws->mBarometric.sReal);

  // 지면온도
  calculate_data_avg(eAVG_GROUND_TEMPERATURE, g_avg_10min, pAws->mGndTemp.sReal);
  calculate_data_min_max(eAVG_GROUND_TEMPERATURE, g_10min_min_max, pAws->mGndTemp.sReal);

  // 초상온도
  calculate_data_avg(eAVG_SURFACE_TEMPERATURE, g_avg_10min, pAws->mGrassTemp.sReal);
  calculate_data_min_max(eAVG_SURFACE_TEMPERATURE, g_10min_min_max, pAws->mGrassTemp.sReal);

  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_5CM, g_avg_10min, pAws->mSoilTemp5cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_5CM, g_10min_min_max, pAws->mSoilTemp5cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_10CM, g_avg_10min, pAws->mSoilTemp10cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_10CM, g_10min_min_max, pAws->mSoilTemp10cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_20CM, g_avg_10min, pAws->mSoilTemp20cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_20CM, g_10min_min_max, pAws->mSoilTemp20cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_30CM, g_avg_10min, pAws->mSoilTemp30cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_30CM, g_10min_min_max, pAws->mSoilTemp30cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_50CM, g_avg_10min, pAws->mSoilTemp50cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_50CM, g_10min_min_max, pAws->mSoilTemp50cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_100CM, g_avg_10min, pAws->mSoilTemp1_0m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_100CM, g_10min_min_max, pAws->mSoilTemp1_0m.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_150CM, g_avg_10min, pAws->mSoilTemp1_5m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_150CM, g_10min_min_max, pAws->mSoilTemp1_5m.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_300CM, g_avg_10min, pAws->mSoilTemp3_0m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_300CM, g_10min_min_max, pAws->mSoilTemp3_0m.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_500CM, g_avg_10min, pAws->mSoilTemp5_0m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_500CM, g_10min_min_max, pAws->mSoilTemp5_0m.sReal);

  g_sunshine.ten_min += g_sunshine.min;
  // 단위변환 W/M2 -> MJ/M2
  g_solar_radiation.ten_min += g_solar_radiation.sunshine_r_1min / 1000000;
  g_solar_radiation.min_acc = 0;
  g_sunshine.min = 0;
  g_rainfall.min = 0;
}

void Min10Process(void)
// 10분이 됬을때 처리 내용
// 온도, 습도, 기압 일조, 일사 1시간 누적, 및 10분 최소 최고 처리
{

  AWS_DATA_STRUCT *pAws;


  pAws = &m10MinAws;

  // 온도 1분자료 업데이트
  pAws->mTemperature.sReal = read_data_average(eAVG_TEMPERATURE, g_avg_10min);
  pAws->mTemperature.sMin = read_data_min(eAVG_TEMPERATURE, g_10min_min_max, MIN_LIMIT);
  pAws->mTemperature.sMax = read_data_max(eAVG_TEMPERATURE, g_10min_min_max, MAX_LIMIT);

  // 습도
  pAws->mHumidity.sReal = read_data_average(eAVG_RELATIVE_HUMIDITY, g_avg_10min);
  pAws->mHumidity.sMin = read_data_min(eAVG_RELATIVE_HUMIDITY, g_10min_min_max, MIN_LIMIT);
  pAws->mHumidity.sMax = read_data_max(eAVG_RELATIVE_HUMIDITY, g_10min_min_max, MAX_LIMIT);

  // 기압
  pAws->mBarometric.sReal = read_data_average(eAVG_PRESSURE, g_avg_10min);
  pAws->mBarometric.sMin = read_data_min(eAVG_PRESSURE, g_10min_min_max, MIN_LIMIT);
  pAws->mBarometric.sMax = read_data_max(eAVG_PRESSURE, g_10min_min_max, MAX_LIMIT);

  // 지면온도
  pAws->mGndTemp.sReal = read_data_average(eAVG_GROUND_TEMPERATURE, g_avg_10min);
  pAws->mGndTemp.sMin = read_data_min(eAVG_GROUND_TEMPERATURE, g_10min_min_max, MIN_LIMIT);
  pAws->mGndTemp.sMax = read_data_max(eAVG_GROUND_TEMPERATURE, g_10min_min_max, MAX_LIMIT);

  // 초상온도
  pAws->mGrassTemp.sReal = read_data_average(eAVG_SURFACE_TEMPERATURE, g_avg_10min);
  pAws->mGrassTemp.sMin = read_data_min(eAVG_SURFACE_TEMPERATURE, g_10min_min_max, MIN_LIMIT);
  pAws->mGrassTemp.sMax = read_data_max(eAVG_SURFACE_TEMPERATURE, g_10min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp5cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_5CM, g_avg_10min);
  pAws->mSoilTemp5cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_5CM, g_10min_min_max, MIN_LIMIT);
  pAws->mSoilTemp5cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_5CM, g_10min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp10cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_10CM, g_avg_10min);
  pAws->mSoilTemp10cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_10CM, g_10min_min_max, MIN_LIMIT);
  pAws->mSoilTemp10cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_10CM, g_10min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp20cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_20CM, g_avg_10min);
  pAws->mSoilTemp20cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_20CM, g_10min_min_max, MIN_LIMIT);
  pAws->mSoilTemp20cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_20CM, g_10min_min_max, MAX_LIMIT);
  // 지중온도
  pAws->mSoilTemp30cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_30CM, g_avg_10min);
  pAws->mSoilTemp30cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_30CM, g_10min_min_max, MIN_LIMIT);
  pAws->mSoilTemp30cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_30CM, g_10min_min_max, MAX_LIMIT);
  // 지중온도
  pAws->mSoilTemp50cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_50CM, g_avg_10min);
  pAws->mSoilTemp50cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_50CM, g_10min_min_max, MIN_LIMIT);
  pAws->mSoilTemp50cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_50CM, g_10min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp1_0m.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_100CM, g_avg_10min);
  pAws->mSoilTemp1_0m.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_100CM, g_10min_min_max, MIN_LIMIT);
  pAws->mSoilTemp1_0m.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_100CM, g_10min_min_max, MAX_LIMIT);

  // 지중온도
  pAws->mSoilTemp1_5m.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_150CM, g_avg_10min);
  pAws->mSoilTemp1_5m.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_150CM, g_10min_min_max, MIN_LIMIT);
  pAws->mSoilTemp1_5m.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_150CM, g_10min_min_max, MAX_LIMIT);

  // 풍향
  pAws->mWind.mDirection.sMax = read_wind_direction_max(eWIND_MAX_10MIN);
  // 풍속
  pAws->mWind.mSpeed.sMax = read_wind_speed_max(eWIND_MAX_10MIN);
  //적설
  pAws->mSnowFall.sReal = mRealAws.mSnowFall.sReal;
  //일조
  pAws->mSunshine.sReal = g_sunshine.ten_min;
  //일사
  pAws->mSolarRad.sReal = g_solar_radiation.ten_min;
  pAws->mSolarRad.sMax = g_solar_radiation.today;


  g_sunshine.hourly += g_sunshine.ten_min;
  g_solar_radiation.hourly += g_solar_radiation.ten_min;


  // 강수량 처리
  pAws->mRainFall.sReal = g_rainfall.ten_min;
  pAws->mRainFall.sHourRain = g_rainfall.hourly;

  //한시간 자료을 위한 연산
  // 온도
  calculate_data_avg(eAVG_TEMPERATURE, g_avg_hour, pAws->mTemperature.sReal);
  calculate_data_min_max(eAVG_TEMPERATURE, g_hour_min_max, pAws->mTemperature.sReal);
  // 습도
  calculate_data_avg(eAVG_RELATIVE_HUMIDITY, g_avg_hour, pAws->mHumidity.sReal);
  calculate_data_min_max(eAVG_RELATIVE_HUMIDITY, g_hour_min_max, pAws->mHumidity.sReal);
  // 기압
  calculate_data_avg(eAVG_PRESSURE, g_avg_hour, pAws->mBarometric.sReal);
  calculate_data_min_max(eAVG_PRESSURE, g_hour_min_max, pAws->mBarometric.sReal);

  // 지면온도
  calculate_data_avg(eAVG_GROUND_TEMPERATURE, g_avg_hour, pAws->mGndTemp.sReal);
  calculate_data_min_max(eAVG_GROUND_TEMPERATURE, g_hour_min_max, pAws->mGndTemp.sReal);

  // 초상온도
  calculate_data_avg(eAVG_SURFACE_TEMPERATURE, g_avg_hour, pAws->mGrassTemp.sReal);
  calculate_data_min_max(eAVG_SURFACE_TEMPERATURE, g_hour_min_max, pAws->mGrassTemp.sReal);

  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_5CM, g_avg_hour, pAws->mSoilTemp5cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_5CM, g_hour_min_max, pAws->mSoilTemp5cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_10CM, g_avg_hour, pAws->mSoilTemp10cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_10CM, g_hour_min_max, pAws->mSoilTemp10cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_20CM, g_avg_hour, pAws->mSoilTemp20cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_20CM, g_hour_min_max, pAws->mSoilTemp20cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_30CM, g_avg_hour, pAws->mSoilTemp30cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_30CM, g_hour_min_max, pAws->mSoilTemp30cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_50CM, g_avg_hour, pAws->mSoilTemp50cm.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_50CM, g_hour_min_max, pAws->mSoilTemp50cm.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_100CM, g_avg_hour, pAws->mSoilTemp1_0m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_100CM, g_hour_min_max, pAws->mSoilTemp1_0m.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_150CM, g_avg_hour, pAws->mSoilTemp1_5m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_150CM, g_hour_min_max, pAws->mSoilTemp1_5m.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_300CM, g_avg_hour, pAws->mSoilTemp3_0m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_300CM, g_hour_min_max, pAws->mSoilTemp3_0m.sReal);
  // 지중온도
  calculate_data_avg(eAVG_SOIL_TEMPERATURE_500CM, g_avg_hour, pAws->mSoilTemp5_0m.sReal);
  calculate_data_min_max(eAVG_SOIL_TEMPERATURE_500CM, g_hour_min_max, pAws->mSoilTemp5_0m.sReal);


  g_solar_radiation.ten_min = 0;
  g_sunshine.ten_min = 0;
  g_rainfall.ten_min = 0;
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
  AWS_DATA_STRUCT *pAws;


  pAws = &mHourAws;

  // 온도
  pAws->mTemperature.sReal = read_data_average(eAVG_TEMPERATURE, g_avg_hour);
  pAws->mTemperature.sMin = read_data_min(eAVG_TEMPERATURE, g_hour_min_max, MIN_LIMIT);
  pAws->mTemperature.sMax = read_data_max(eAVG_TEMPERATURE, g_hour_min_max, MAX_LIMIT);

  // 기압
  pAws->mBarometric.sReal = read_data_average(eAVG_PRESSURE, g_avg_hour);
  pAws->mBarometric.sMin = read_data_min(eAVG_PRESSURE, g_hour_min_max, MIN_LIMIT);
  pAws->mBarometric.sMax = read_data_max(eAVG_PRESSURE, g_hour_min_max, MAX_LIMIT);

  // 습도
  pAws->mHumidity.sReal = read_data_average(eAVG_RELATIVE_HUMIDITY, g_avg_hour);
  pAws->mHumidity.sMin = read_data_min(eAVG_RELATIVE_HUMIDITY, g_hour_min_max, MIN_LIMIT);
  pAws->mHumidity.sMax = read_data_max(eAVG_RELATIVE_HUMIDITY, g_hour_min_max, MAX_LIMIT);

  // 풍속
  pAws->mWind.mSpeed.sMax = read_wind_speed_max(eWIND_MAX_HOUR);
  // 풍향
  pAws->mWind.mDirection.sMax = read_wind_direction_max(eWIND_MAX_HOUR);


  // 일사
  pAws->mSolarRad.sReal = g_solar_radiation.hourly;
  pAws->mSolarRad.sMax = g_solar_radiation.today;
  // 일조
  pAws->mSunshine.sReal = g_sunshine.hourly;
 //지면온도
  pAws->mGndTemp.sReal = read_data_average(eAVG_GROUND_TEMPERATURE, g_avg_hour);
  pAws->mGndTemp.sMin = read_data_min(eAVG_GROUND_TEMPERATURE, g_hour_min_max, MIN_LIMIT);
  pAws->mGndTemp.sMax = read_data_max(eAVG_GROUND_TEMPERATURE, g_hour_min_max, MAX_LIMIT);
 //초상 온도
  pAws->mGrassTemp.sReal = read_data_average(eAVG_SURFACE_TEMPERATURE, g_avg_hour);
  pAws->mGrassTemp.sMin = read_data_min(eAVG_SURFACE_TEMPERATURE, g_hour_min_max, MIN_LIMIT);
  pAws->mGrassTemp.sMax = read_data_max(eAVG_SURFACE_TEMPERATURE, g_hour_min_max, MAX_LIMIT);

  // 지중온도 처리 2017.04.03
  pAws->mSoilTemp5cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_5CM, g_avg_hour);
  pAws->mSoilTemp5cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_5CM, g_hour_min_max, MIN_LIMIT);
  pAws->mSoilTemp5cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_5CM, g_hour_min_max, MAX_LIMIT);

  pAws->mSoilTemp10cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_10CM, g_avg_hour);
  pAws->mSoilTemp10cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_10CM, g_hour_min_max, MIN_LIMIT);
  pAws->mSoilTemp10cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_10CM, g_hour_min_max, MAX_LIMIT);

  pAws->mSoilTemp20cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_20CM, g_avg_hour);
  pAws->mSoilTemp20cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_20CM, g_hour_min_max, MIN_LIMIT);
  pAws->mSoilTemp20cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_20CM, g_hour_min_max, MAX_LIMIT);

  pAws->mSoilTemp30cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_30CM, g_avg_hour);
  pAws->mSoilTemp30cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_30CM, g_hour_min_max, MIN_LIMIT);
  pAws->mSoilTemp30cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_30CM, g_hour_min_max, MAX_LIMIT);

  pAws->mSoilTemp50cm.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_50CM, g_avg_hour);
  pAws->mSoilTemp50cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_50CM, g_hour_min_max, MIN_LIMIT);
  pAws->mSoilTemp50cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_50CM, g_hour_min_max, MAX_LIMIT);

  pAws->mSoilTemp1_0m.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_100CM, g_avg_hour);
  pAws->mSoilTemp1_0m.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_100CM, g_hour_min_max, MIN_LIMIT);
  pAws->mSoilTemp1_0m.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_100CM, g_hour_min_max, MAX_LIMIT);

  pAws->mSoilTemp1_5m.sReal = read_data_average(eAVG_SOIL_TEMPERATURE_150CM, g_avg_hour);
  pAws->mSoilTemp1_5m.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_150CM, g_hour_min_max, MIN_LIMIT);
  pAws->mSoilTemp1_5m.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_150CM, g_hour_min_max, MAX_LIMIT);

  pAws->mRainFall.sHourRain = g_rainfall.hourly;

  g_rainfall.hourly = 0;
  g_sunshine.hourly = 0;
  g_solar_radiation.hourly = 0;
}

void DayProcess(void)
{
  AWS_DATA_STRUCT *pAws;

  pAws = &mDayAws;

  // 온도
  pAws->mTemperature.sReal = mRealAws.mTemperature.sReal;
  pAws->mTemperature.sMin = read_data_min(eAVG_TEMPERATURE, g_day_min_max, MIN_LIMIT);
  pAws->mTemperature.sMax = read_data_max(eAVG_TEMPERATURE, g_day_min_max, 10000);

  // 기압
  pAws->mBarometric.sReal = mRealAws.mBarometric.sReal;
  pAws->mBarometric.sMin = read_data_min(eAVG_PRESSURE, g_day_min_max, MIN_LIMIT);
  pAws->mBarometric.sMax = read_data_max(eAVG_PRESSURE, g_day_min_max, 10000);

  // 습도
  pAws->mHumidity.sReal = mRealAws.mHumidity.sReal;
  pAws->mHumidity.sMin = read_data_min(eAVG_RELATIVE_HUMIDITY, g_day_min_max, MIN_LIMIT);
  pAws->mHumidity.sMax = read_data_max(eAVG_RELATIVE_HUMIDITY, g_day_min_max, 10000);

  pAws->mWind.mSpeed.sReal = mRealAws.mWind.mSpeed.sReal;
  pAws->mWind.mDirection.sReal = mRealAws.mWind.mDirection.sReal;

  pAws->mWind.mDirection.sMax = read_wind_direction_max(eWIND_MAX_DAY);
  pAws->mWind.mSpeed.sMax = read_wind_speed_max(eWIND_MAX_DAY);

  // 일사
  pAws->mSolarRad.sReal = g_solar_radiation.today;
  pAws->mSolarRad.sMax = g_solar_radiation.today;
  // 일조
  pAws->mSunshine.sReal = g_sunshine.today;
 //적설
  pAws->mSnowFall.sReal = mRealAws.mSnowFall.sReal;
  
  //우량
  pAws->mRainFall.sReal = g_rainfall.today;

  // 지면온도
  pAws->mGndTemp.sReal = mRealAws.mGndTemp.sReal;
  pAws->mGndTemp.sMin = read_data_min(eAVG_GROUND_TEMPERATURE, g_day_min_max, MIN_LIMIT);
  pAws->mGndTemp.sMax = read_data_max(eAVG_GROUND_TEMPERATURE, g_day_min_max, MAX_LIMIT);
  // 초상 온도
  pAws->mGrassTemp.sReal = mRealAws.mGrassTemp.sReal;
  pAws->mGrassTemp.sMin = read_data_min(eAVG_SURFACE_TEMPERATURE, g_day_min_max, MIN_LIMIT);
  pAws->mGrassTemp.sMax = read_data_max(eAVG_SURFACE_TEMPERATURE, g_day_min_max, MAX_LIMIT);

  // 지중온도 처리 2017.04.03
  pAws->mSoilTemp5cm.sReal = mRealAws.mSoilTemp5cm.sReal;
  pAws->mSoilTemp5cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_5CM, g_day_min_max, MIN_LIMIT);
  pAws->mSoilTemp5cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_5CM, g_day_min_max, MAX_LIMIT);

  pAws->mSoilTemp10cm.sReal = mRealAws.mSoilTemp10cm.sReal;
  pAws->mSoilTemp10cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_10CM, g_day_min_max, MIN_LIMIT);
  pAws->mSoilTemp10cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_10CM, g_day_min_max, MAX_LIMIT);

  pAws->mSoilTemp20cm.sReal = mRealAws.mSoilTemp20cm.sReal;
  pAws->mSoilTemp20cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_20CM, g_day_min_max, MIN_LIMIT);
  pAws->mSoilTemp20cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_20CM, g_day_min_max, MAX_LIMIT);

  pAws->mSoilTemp30cm.sReal = mRealAws.mSoilTemp30cm.sReal;
  pAws->mSoilTemp30cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_30CM, g_day_min_max, MIN_LIMIT);
  pAws->mSoilTemp30cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_30CM, g_day_min_max, MAX_LIMIT);

  pAws->mSoilTemp50cm.sReal = mRealAws.mSoilTemp50cm.sReal;
  pAws->mSoilTemp50cm.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_50CM, g_day_min_max, MIN_LIMIT);
  pAws->mSoilTemp50cm.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_50CM, g_day_min_max, MAX_LIMIT);

  pAws->mSoilTemp1_0m.sReal = mRealAws.mSoilTemp1_0m.sReal;
  pAws->mSoilTemp1_0m.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_100CM, g_day_min_max, MIN_LIMIT);
  pAws->mSoilTemp1_0m.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_100CM, g_day_min_max, MAX_LIMIT);

  pAws->mSoilTemp1_5m.sReal = mRealAws.mSoilTemp1_5m.sReal;
  pAws->mSoilTemp1_5m.sMin = read_data_min(eAVG_SOIL_TEMPERATURE_150CM, g_day_min_max, MIN_LIMIT);
  pAws->mSoilTemp1_5m.sMax = read_data_max(eAVG_SOIL_TEMPERATURE_150CM, g_day_min_max, MAX_LIMIT);

  g_rainfall.yesterday = g_rainfall.today;
  g_rainfall.today = 0;
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
    update_kma_data(eAWS_DATA_1MIN,pDate);
    save_min_data(pDate);

    if (pDate->Min % 10 == 0)
    { 
      Min10Process();
      update_kma_data(eAWS_DATA_10MIN, pDate);
    }
    pOldDate->Min = pDate->Min;
  }

  if (pDate->Hour != pOldDate->Hour)
  { 
    HourProcess(pDate);
    update_kma_data(eAWS_DATA_HOUR, pDate);
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

void set_active_sensor(uint8_t *p_status,eSENSOR_TYPE_t sensor_number,bool active)
{
  int quot;
  int rem;

  quot = sensor_number / 8;
  rem = sensor_number % 8;

  if(active)
  {
    p_status[quot] |= 1 << rem;
  }
}

void save_min_data(DATE_TIME_BUF *p_time)
{
  kma_data_ex_t *p_kma_data;
  aws_logging_data_t *p_logging;

  p_kma_data = get_kma_data(eAWS_DATA_1MIN);

  p_logging = pvPortMalloc(sizeof(aws_logging_data_t));

  if(p_logging)
  {
    memset(p_logging,0,sizeof(aws_logging_data_t));
    p_logging->time = p_kma_data->time;
    

    p_logging->temperature = p_kma_data->temperature.data;
    set_active_sensor(p_logging->active, A1_TEMPERATURE,p_kma_data->temperature.enable);
    
    p_logging->wind_direction_avg = p_kma_data->wind_direction_avg.data;
    set_active_sensor(p_logging->active, A2_WIND_DIRECTION,p_kma_data->wind_direction_avg.enable);
    
    p_logging->wind_speed_avg = p_kma_data->wind_speed_avg.data;
    set_active_sensor(p_logging->active, A3_WIND_SPEED,p_kma_data->wind_speed_avg.enable);
    
    p_logging->wind_direction_instant = p_kma_data->wind_direction_instant.data;
    set_active_sensor(p_logging->active, A2_WIND_DIRECTION,p_kma_data->wind_direction_instant.enable);
    
    p_logging->wind_speed_instant = p_kma_data->wind_speed_instant.data;
    set_active_sensor(p_logging->active, A3_WIND_SPEED,p_kma_data->wind_speed_instant.enable);
    
    p_logging->precipitation = p_kma_data->precipitation.data;
    set_active_sensor(p_logging->active, A6_RAINFALL_DOT5_1MM,p_kma_data->precipitation.enable);
    
    p_logging->pressure = p_kma_data->pressure.data;
    set_active_sensor(p_logging->active, A7_PRESSURE,p_kma_data->pressure.enable);
    
    p_logging->precipitation_presence = p_kma_data->precipitation_presence.data;
    set_active_sensor(p_logging->active, A8_RAIN_PRESENT,p_kma_data->precipitation_presence.enable);
    
    p_logging->snowfall = p_kma_data->snowfall.data;
    set_active_sensor(p_logging->active, A9_SNOW_DEPTH,p_kma_data->snowfall.enable);
    
    p_logging->relative_humidity = p_kma_data->relative_humidity.data;
    set_active_sensor(p_logging->active, A10_RELATIVE_HUMIDITY,p_kma_data->relative_humidity.enable);
    
    p_logging->precipitation_fine = p_kma_data->precipitation_fine.data;
    set_active_sensor(p_logging->active, A11_RAINFALL_DOT1MM,p_kma_data->precipitation_fine.enable);
    
    p_logging->solar_radiation = p_kma_data->solar_radiation.data;
    set_active_sensor(p_logging->active, B1_SOLAR_RADIATION,p_kma_data->solar_radiation.enable);
    
    p_logging->sunshine_duration = p_kma_data->sunshine_duration.data;
    set_active_sensor(p_logging->active, B2_SUNSHINE_DURATION,p_kma_data->sunshine_duration.enable);
    
    p_logging->surface_temperature = p_kma_data->surface_temperature.data;
    set_active_sensor(p_logging->active, B3_GROUND_TEMPERATURE,p_kma_data->surface_temperature.enable);
    
    p_logging->grass_temperature = p_kma_data->grass_temperature.data;
    set_active_sensor(p_logging->active, B4_SURFACE_TEMPERATURE,p_kma_data->grass_temperature.enable);
    
    p_logging->soil_temperature_5cm = p_kma_data->soil_temperature_5cm.data;
    set_active_sensor(p_logging->active, B5_SOIL_TEMPERATURE_5CM,p_kma_data->soil_temperature_5cm.enable);
    
    p_logging->soil_temperature_10cm = p_kma_data->soil_temperature_10cm.data;
    set_active_sensor(p_logging->active, B6_SOIL_TEMPERATURE_10CM,p_kma_data->soil_temperature_10cm.enable);
    
    p_logging->soil_temperature_20cm = p_kma_data->soil_temperature_20cm.data;
    set_active_sensor(p_logging->active, B7_SOIL_TEMPERATURE_20CM,p_kma_data->soil_temperature_20cm.enable);
    
    p_logging->soil_temperature_30cm = p_kma_data->soil_temperature_30cm.data;
    set_active_sensor(p_logging->active, B8_SOIL_TEMPERATURE_30CM,p_kma_data->soil_temperature_30cm.enable);
    
    p_logging->soil_temperature_50cm = p_kma_data->soil_temperature_50cm.data;
    set_active_sensor(p_logging->active, B9_SOIL_TEMPERATURE_50CM,p_kma_data->soil_temperature_50cm.enable);
    
    p_logging->soil_temperature_1m = p_kma_data->soil_temperature_1m.data;
    set_active_sensor(p_logging->active, B10_SOIL_TEMPERATURE_100CM,p_kma_data->soil_temperature_1m.enable);
    
    p_logging->soil_temperature_1_5m = p_kma_data->soil_temperature_1_5m.data;
    set_active_sensor(p_logging->active, B11_SOIL_TEMPERATURE_150CM,p_kma_data->soil_temperature_1_5m.enable);
    
    p_logging->soil_temperature_3m = p_kma_data->soil_temperature_3m.data;
    set_active_sensor(p_logging->active, B12_SOIL_TEMPERATURE_300CM,p_kma_data->soil_temperature_3m.enable);
    
    p_logging->soil_temperature_5m = p_kma_data->soil_temperature_5m.data;
    set_active_sensor(p_logging->active, B13_SOIL_TEMPERATURE_500CM,p_kma_data->soil_temperature_5m.enable);
    
    p_logging->cloud_height_1st = p_kma_data->cloud_height_1st.data;
    set_active_sensor(p_logging->active, C1_CLOUD_BASE1,p_kma_data->cloud_height_1st.enable);
    
    p_logging->cloud_height_2nd = p_kma_data->cloud_height_2nd.data;
    set_active_sensor(p_logging->active, C2_CLOUD_BASE2,p_kma_data->cloud_height_2nd.enable);
    
    p_logging->cloud_height_3rd = p_kma_data->cloud_height_3rd.data;
    set_active_sensor(p_logging->active, C3_CLOUD_BASE3,p_kma_data->cloud_height_3rd.enable);
    
    p_logging->cloud_amount = p_kma_data->cloud_amount.data;
    set_active_sensor(p_logging->active, C4_CLOUD_COVER,p_kma_data->cloud_amount.enable);
    
    p_logging->visibility = p_kma_data->visibility.data;
    set_active_sensor(p_logging->active, C5_VISIBILITY,p_kma_data->visibility.enable);
    
    p_logging->pm10_concentration = p_kma_data->pm10_concentration.data;
    set_active_sensor(p_logging->active, C6_PM10,p_kma_data->pm10_concentration.enable);
    
    p_logging->pm25_concentration = p_kma_data->pm25_concentration.data;
    set_active_sensor(p_logging->active, C7_PM2DOT5,p_kma_data->pm25_concentration.enable);
    
    p_logging->net_radiation = p_kma_data->net_radiation.data;
    set_active_sensor(p_logging->active, C8_NET_RADIATION,p_kma_data->net_radiation.enable);
    
    p_logging->total_radiation = p_kma_data->total_radiation.data;
    set_active_sensor(p_logging->active, C9_TOTAL_RADIATION,p_kma_data->total_radiation.enable);
    
    p_logging->reflected_radiation = p_kma_data->reflected_radiation.data;
    set_active_sensor(p_logging->active, C10_REFLECTED_RADIATION,p_kma_data->reflected_radiation.enable);
    
    p_logging->direct_radiation = p_kma_data->direct_radiation.data;
    set_active_sensor(p_logging->active, C11_DIRECT_SOLAR,p_kma_data->direct_radiation.enable);
    
    p_logging->current_weather = p_kma_data->current_weather.data;
    set_active_sensor(p_logging->active, C12_CURRENT_WEATHER,p_kma_data->current_weather.enable);
    
    p_logging->soil_moisture_10cm = p_kma_data->soil_moisture_10cm.data;
    set_active_sensor(p_logging->active, N1_SOIL_MOISTURE_10CM,p_kma_data->soil_moisture_10cm.enable);
    
    p_logging->soil_moisture_20cm = p_kma_data->soil_moisture_20cm.data;
    set_active_sensor(p_logging->active, N2_SOIL_MOISTURE_20CM,p_kma_data->soil_moisture_20cm.enable);
    
    p_logging->soil_moisture_30cm = p_kma_data->soil_moisture_30cm.data;
    set_active_sensor(p_logging->active, N3_SOIL_MOISTURE_30CM,p_kma_data->soil_moisture_30cm.enable);
    
    p_logging->soil_moisture_50cm = p_kma_data->soil_moisture_50cm.data;
    set_active_sensor(p_logging->active, N4_SOIL_MOISTURE_50CM,p_kma_data->soil_moisture_50cm.enable);
    
    p_logging->illuminance = p_kma_data->illuminance.data;
    set_active_sensor(p_logging->active, N5_ILLUMINANCE,p_kma_data->illuminance.enable);
    
    p_logging->wind_speed_1_5m = p_kma_data->wind_speed_1_5m.data;
    set_active_sensor(p_logging->active, N6_WIND_VELOCITY_150CM,p_kma_data->wind_speed_1_5m.enable);
    
    p_logging->wind_speed_4m = p_kma_data->wind_speed_4m.data;
    set_active_sensor(p_logging->active, N7_WIND_VELOCITY_400CM,p_kma_data->wind_speed_4m.enable);
    
    p_logging->instant_wind_speed_1_5m = p_kma_data->instant_wind_speed_1_5m.data;
    set_active_sensor(p_logging->active, N8_INSTANT_VELOCITY_150CM,p_kma_data->instant_wind_speed_1_5m.enable);
    
    p_logging->instant_wind_speed_4m = p_kma_data->instant_wind_speed_4m.data;
    set_active_sensor(p_logging->active, N9_INSTANT_VELOCITY_400CM,p_kma_data->instant_wind_speed_4m.enable);
    
    p_logging->temperature_0_5m = p_kma_data->temperature_0_5m.data;
    set_active_sensor(p_logging->active, N10_AIR_TEMPERATURE_50CM,p_kma_data->temperature_0_5m.enable);
    
    p_logging->temperature_4m = p_kma_data->temperature_4m.data;
    set_active_sensor(p_logging->active, N11_AIR_TEMPERATURE_400CM,p_kma_data->temperature_4m.enable);
    
    p_logging->humidity_0_5m = p_kma_data->humidity_0_5m.data;
    set_active_sensor(p_logging->active, N12_HUMIDITY_50CM,p_kma_data->humidity_0_5m.enable);
    
    p_logging->humidity_4m = p_kma_data->humidity_4m.data;
    set_active_sensor(p_logging->active, N13_HUMIDITY_400CM,p_kma_data->humidity_4m.enable);
    
    p_logging->tacometer = p_kma_data->tacometer.data;
    set_active_sensor(p_logging->active, I1_TACHOMETER,p_kma_data->tacometer.enable);

    for(int i = 0 ; i< 8; i++)
    {
      p_logging->X_sensorStatus[i] = p_kma_data->X_sensorStatus[i];
    }
    p_logging->Y_volateStatus = p_kma_data->Y_volateStatus;

    int count = sizeof(aws_logging_data_t) - (uint32_t)(&((aws_logging_data_t *)0)->time);

    p_logging->crc = crc16_ccitt_table((uint8_t *)&p_logging->time,count);

    os_save_aws_data(p_time, p_logging, sizeof(aws_logging_data_t), LOGGING_AWS_NEW, 1);

    vPortFree(p_logging);
  }


}

void update_kma_data(eAWS_DATA_MIN_t min,DATE_TIME_BUF *p_time)
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
    case eAWS_DATA_DAY:
      pAws = &mDayAws;
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
  p_kma_data->wind_direction_instant.data = pAws->mWind.mDirection.sMax;

  // 풍속
  p_kma_data->wind_speed_avg.data = pAws->mWind.mSpeed.sReal;
  p_kma_data->wind_speed_avg.max = pAws->mWind.mSpeed.sMax;
  p_kma_data->wind_speed_instant.data = pAws->mWind.mSpeed.sMax;


  // 일조
  p_kma_data->sunshine_duration.data = g_sunshine.today;

  // 일사 // mSolarRad.sReal kw/m2 단위인데 전송시에는 mj/m2 *100 한값이 전송되어야함
  // 따라서 여기서 10으로 한번더 나누어 준다 .즉 data는 최종 전송되는 데이터 포맷이다.
  //에너지(J) = 전력(W)*시간(s)
  p_kma_data->solar_radiation.data = g_solar_radiation.sunshine_r_1min/ 10000;
  p_kma_data->solar_radiation.day_accu = pAws->mSolarRad.sMax; // 일간

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


  p_kma_data->precipitation.data  = pAws->mRainFall.sReal;
  p_kma_data->precipitation.hour  = pAws->mRainFall.sHourRain;    // 시간당 강수량량
  p_kma_data->precipitation.month = pAws->mRainFall.sMonthRain;  // 월간 강수량
  p_kma_data->precipitation.year  = pAws->mRainFall.sYearRain;  // 연간 강수량

  p_kma_data->precipitation_presence.data = pAws->mRainDetect.sReal;  // 우량 감지

  p_kma_data->snowfall.data = pAws->mSnowFall.sReal;

  kma_data_ex_t *p_kma_avg = get_kma_data(eAWS_DATA_REAL);

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
