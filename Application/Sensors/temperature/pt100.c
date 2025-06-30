#include "pt100.h"

#include <math.h>

#include "app_adc.h"
#include "driver_adc.h"
#include "driver_interface.h"
#include "pt100.h"
#include "util_memory.h"

#define PT100_CNT 2
#define MIN_TEMP -42  // 최소 온도
#define MAX_TEMP 62   // 최대 온도
#define TABLE_SIZE (MAX_TEMP - MIN_TEMP + 1)

// PT100 저항 테이블
const float pt100_table[TABLE_SIZE] = {83.48,  83.88,  // -°42C ~ --41°C
                                       84.27,  84.67,  85.06,  85.46,  85.85,
                                       86.25,  86.64,  87.04,  87.43,  87.83,  // -40°C ~ -31°C
                                       88.22,  88.62,  89.01,  89.40,  89.80,
                                       90.19,  90.59,  90.98,  91.37,  91.77,  // -30°C ~ -21°C
                                       92.16,  92.55,  92.95,  93.34,  93.73,
                                       94.12,  94.52,  94.91,  95.30,  95.69,  // -20°C ~ -11°C
                                       96.09,  96.48,  96.87,  97.26,  97.65,
                                       98.04,  98.44,  98.83,  99.22,  99.61,  // -10°C ~  -1°C
                                       100.00, 100.39, 100.78, 101.17, 101.56,
                                       101.95, 102.34, 102.73, 103.12, 103.51,  //   0°C ~   9°C
                                       103.90, 104.29, 104.68, 105.07, 105.46,
                                       105.85, 106.24, 106.63, 107.02, 107.40,  //  10°C ~  19°C
                                       107.79, 108.18, 108.57, 108.96, 109.35,
                                       109.73, 110.12, 110.51, 110.90, 111.29,  //  20°C ~  29°C
                                       111.67, 112.06, 112.45, 112.83, 113.22,
                                       113.61, 114.00, 114.38, 114.77, 115.15,  //  30°C ~  39°C
                                       115.54, 115.93, 116.31, 116.70, 117.08,
                                       117.47, 117.86, 118.24, 118.63, 119.01,  //  40°C ~  49°C
                                       119.40, 119.78, 120.17, 120.55, 120.94,
                                       121.32, 121.71, 122.09, 122.47, 122.86,  //  50°C ~  59°C
                                       123.24, 123.63, 124.01};                 // 60°C ~ 62°C

// 저항값을 기반으로 온도를 계산하는 함수 (선형 보간법 사용)
float pt100_resistance_to_temperature(float resistance)
{
  // 범위를 벗어난 경우

  if (bigger_float(resistance, pt100_table[TABLE_SIZE - 1]) ||
      less_float(resistance, pt100_table[0]))
  {
    return 9999.0;  // 오류 코드
  }

  for (int i = 0; i < TABLE_SIZE - 1; i++)
  {
    if (bigger_equal_float(resistance, pt100_table[i]) &&
        less_equal_float(resistance, pt100_table[i + 1]))
    {
      // 선형 보간법 적용
      double temp1 = MIN_TEMP + i;
      double temp2 = MIN_TEMP + i + 1;
      double res1 = pt100_table[i];
      double res2 = pt100_table[i + 1];

      return temp1 + (temp2 - temp1) * ((resistance - res1) / (res2 - res1));
    }
  }

  return 9999.0;  // 오류 코드
}

 const float kConstanctA = 1.2454e-3;


typedef struct pt100_cfg_s
{
  driver_t *adc_io;
  uint8_t channel;
}pt100_cfg_t;

#define NEW_AWS_METHOD

#define PT100_ADC_AVG_CNT 1
/**
 * @brief 온도 단위 도 12.56도
 */
float read_pt100_temperature(driver_t *driver,uint8_t *err)
{
#ifdef NEW_AWS_METHOD

  int32_t adc_ch;
  float resistance;
  float temperature;
  float voltage;
  const pt100_cfg_t *cfg = ((driver_t *)driver)->cfg;

  switch (cfg->channel)
  {
    case PT100_A:
      adc_ch = eADC_S_CH_16;
      break;

    case PT100_B:
      adc_ch = eADC_S_CH_17;
      break;
  }

  if (cfg->channel == PT100_A)
  {
    voltage = adc_read_single_avg(adc_ch, err, PT100_ADC_AVG_CNT);
    resistance = voltage;  // 이채널은은 하드웨어 설계 특성상 저항이됨,관련 자료 참고
    temperature = pt100_resistance_to_temperature(resistance);
    return temperature;
  }

  if (cfg->channel == PT100_B)
  {
    voltage = adc_read_single_avg(adc_ch, err, PT100_ADC_AVG_CNT);
    resistance = voltage;  // 이채널은은 하드웨어 설계 특성상 저항이됨,관련 자료 참고
    temperature = pt100_resistance_to_temperature(resistance);
    return temperature;
  }
  
  return temperature;
#else

  int32_t sAdval;
  int32_t fullset;
  int32_t offset;
  int32_t sSpan;
  int32_t sMinus45;
  int32_t sPlus65;
  int32_t errTmp;
  int32_t ss;
  int32_t i;
  int32_t      tt;
  float temperature;
  int32_t adc_ch;
  const pt100_cfg_t *cfg = ((driver_t *)driver)->cfg;
  float x, resistance;


  switch (cfg->channel)
  {
    case PT100_A:
    adc_ch = eADC_S_CH_16;
    break;

    case PT100_B:
    adc_ch = eADC_S_CH_17;
    break;
  }

  if (cfg->channel == PT100_A)
  {
    float voltage;
    voltage = adc_read_single_avg(adc_ch, err,5);

    //resistance = voltage / kConstanctA;
    resistance = voltage;//
    temperature = pt100_resistance_to_temperature(resistance);

    return temperature;
  }


  if(cfg->channel == PT100_B)
  {
    float voltage;
    voltage = adc_read_volate_single(adc_ch,err);

    resistance = voltage/kConstanctA;
    temperature = pt100_resistance_to_temperature(resistance);

    return temperature;
  }

  offset  = get_adc_single_offset(adc_ch);
  fullset = get_adc_single_fullset(adc_ch);
  sAdval  = adc_read_single_avg(adc_ch, err,10);


  //여기서 부터 기존 AWS 온도 코드인데 이해가 안됨.일단 사용
  //원분석: sSpan은 -40,60도 ADC값 
  sSpan    = fullset - offset;   //저항 1개당 ADC값 ,79%,77%로 offset %출처 모름름
  sMinus45 = (int32_t)(((float)sSpan / 38.97) * 0.79);//원 분석:38.97= 123.24-84.27
  sPlus65  = (int32_t)(((float)sSpan / 38.97) * 0.77);

  sSpan   = ((fullset + sMinus45) - (offset - sPlus65));//범위를 더 넓게 줌?
  errTmp = (int32_t)((float)sSpan * 0.05); 


  if((sAdval > ((offset - sMinus45) - errTmp)) && 
     (sAdval < ((fullset+ sPlus65) + errTmp)))
  {
    if(sSpan > 0)
    {
      x = 40.53 / (float)sSpan;     //40.53= 124.01(62도)-83.48(-42도),  AD Convertion Value 값을 저항 테이블에 맞춤
      resistance = (float)(sAdval - (offset- sMinus45)) * x + pt100_table[0]; //  저항값 검색 하기위해 전압을 저항으로 변환
      //f는 저항값
      temperature = pt100_resistance_to_temperature(resistance);
    }
    else
    {
      temperature = 9999.0f;
    }
  }
  else
  {
    temperature = 9999.0f;
  }
  return(temperature);
  #endif
}

driver_t pt100_driver[PT100_CNT];
pt100_cfg_t pt100_cfg[PT100_CNT];

temperature_api_t temperature_api ={.read= read_pt100_temperature};

void *pt100_open(uint8_t num,void *opt)
{

  if(pt100_driver[num].opened)
  {
    return &pt100_driver;
  }
  pt100_driver[num].opened = true;
  pt100_cfg[num].channel = num;

  pt100_driver[num].cfg = &pt100_cfg[num];
  pt100_driver[num].api = &temperature_api;

  pt100_cfg[num].adc_io = driver_adc_open(ADC_ADS1220,0);



  return &pt100_driver[num];
}