#include <math.h>
#include "driver_adc.h"

#define VIN_SOLA_V_MAX 2003          /* full scale 일때 최대값 */
#define VIN_SOLA_FULL_SCALE 12000 
#define ADC_REF_VOLTAGE 3300
#define ADC_SCALE(max,full)  ((double)max/(double)full)


driver_t *g_adcStm;




int32_t get_mV(int32_t adc, uint32_t bitCnt,double refVolt,double scale)
{
  int32_t val;

  if(scale != 0)
  {
    val = (int32_t)((adc / (float)bitCnt) * refVolt / scale);
  }
  else
  {
    val = 0;
  }
  return val;
}

void battery_init(void)
{
  g_adcStm = driver_adc_open(ADC_STM32,0);
}

float read_battery(void)
{
  int32_t val;
  uint8_t err;

  val = driver_adc_single_read(g_adcStm,ADC_STM32_S_CH_0,1,&err);
  val = get_mV(val,4095,ADC_REF_VOLTAGE,ADC_SCALE(VIN_SOLA_V_MAX,VIN_SOLA_FULL_SCALE));

  return (float)val/1000.0;

}


#define VREF 3.3f           // ADC 기준 전압
#define ADC_MAX 4095.0f     // 12비트 ADC 최대값
#define R_PULLUP 10000.0f   // 10kΩ 풀업 저항

// ADC 값을 서미스터 저항값으로 변환하는 함수
float calculate_ntc_resistance(int32_t adc_value)
{
  if (adc_value <= 0 || adc_value >= ADC_MAX) // 값이 범위를 벗어나면 무효
    return 0.0f;

  // ADC 값 → 전압 변환
  float v_ntc = (adc_value / ADC_MAX) * VREF;

  // 서미스터 저항값 계산
  float r_ntc = R_PULLUP * (v_ntc / (VREF - v_ntc));

  return r_ntc;  // 서미스터 저항값 반환 (Ω 단위)
}


typedef struct {
  float temperature;
  float resistance;
} NTC_Lookup;

// LNSK 103 서미스터 저항-온도 테이블 (일부 주요 값)
const NTC_Lookup ntc_table[] = {
  { -40.0, 200800 }, { -35.0, 152900 }, { -30.0, 117200 }, { -25.0, 90510 },
  { -20.0, 70400 }, { -15.0, 55140 }, { -10.0, 43510 }, { -5.0, 34570 },
  { 0.0, 27660 }, { 5.0, 22280 }, { 10.0, 18070 }, { 15.0, 14740 },
  { 20.0, 12110 }, { 25.0, 10000 }, { 30.0, 8307 }, { 35.0, 6938 },
  { 40.0, 5824 }, { 45.0, 4913 }, { 50.0, 4164 }, { 55.0, 3543 },
  { 60.0, 3028 }, { 65.0, 2597 }, { 70.0, 2235 }, { 75.0, 1930 },
  { 80.0, 1671 }, { 85.0, 1452 }, { 90.0, 1264 }, { 95.0, 1104 },
  { 100.0, 966 }, { 105.0, 848 }, { 110.0, 746 }, { 115.0, 657 },
  { 120.0, 581 }
};
#define TABLE_SIZE (sizeof(ntc_table) / sizeof(ntc_table[0]))

float ntc_resistance_to_temperature(float resistance)
{
  if (resistance >= ntc_table[0].resistance)
    return ntc_table[0].temperature;  // 최소 온도 이하
  if (resistance <= ntc_table[TABLE_SIZE - 1].resistance)
    return ntc_table[TABLE_SIZE - 1].temperature;  // 최대 온도 이상

  // 테이블에서 적절한 범위를 찾음
  for (int i = 0; i < TABLE_SIZE - 1; i++)
  {
    if (resistance <= ntc_table[i].resistance && resistance > ntc_table[i + 1].resistance)
    {
      // 선형 보간법 적용
      float temp1 = ntc_table[i].temperature;
      float temp2 = ntc_table[i + 1].temperature;
      float res1 = ntc_table[i].resistance;
      float res2 = ntc_table[i + 1].resistance;

      // y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
      return (temp1 + (resistance - res1) * (temp2 - temp1) / (res2 - res1));
    }
  }

  return 0.0f; // 이론적으로 도달하지 않음
}

float read_temperature(void)
{
  int32_t val;
  uint8_t err;
  

  val = driver_adc_single_read(g_adcStm,ADC_STM32_S_CH_1,1,&err);


  return ntc_resistance_to_temperature(calculate_ntc_resistance(val));

}