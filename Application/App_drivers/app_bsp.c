#include "app_bsp.h"
#include <math.h>
#include "driver_adc.h"
#include "driver_led.h"
#include "driver_do.h"
#include "driver_di.h"

driver_t *g_adc_stm;
driver_t *g_status_led;
driver_t *g_cdma_power;
driver_t *g_port_mode;
driver_t *g_door_status;

void status_led_init(void);

void set_portd_hart_mode(void)
{
  driver_do_high(g_port_mode);
}

void set_portd_rs232_mode(void)
{
  driver_do_low(g_port_mode);
}

void app_bsp_init(void)
{
  g_adc_stm = driver_adc_open(ADC_STM32,0);

  status_led_init();

  g_cdma_power = driver_do_open(DO_PWR_CDMA,0);

  g_port_mode = driver_do_open(DO_HART_SEL, 0);

  g_door_status = driver_di_open(DI_EXT_0,0);

  set_portd_rs232_mode();
}


bool door_opened(void)
{
  if(driver_di_read(g_door_status))
  {
    return false;
  }

  return true;
}

void cdma_power_on(void)
{
  driver_do_high(g_cdma_power);
}

void cdma_power_off(void)
{
  driver_do_low(g_cdma_power);
}

/*
공급전압 최대 입력을 15V로 하자
0~2.5V => 0~15V
12V(전압)
|
49.9K
|-------1K---ADC
10K
|
GND
*/

float read_battery(void)
{
  const float slope = 6;  // (float)(15.0f-0.0f)/(float)(2.5-0);
  const float offset = 0.0;
  uint8_t err;
  float voltage;
  float battery;

  voltage = driver_adc_single_read(g_adc_stm, ADC_STM32_S_CH_0, 1, &err);

  battery = voltage * slope + offset;

  return (float)battery;

}


/*
온도측정정
3.3V(VREF)
|
10K(R1)
|----------ADC
10K(NTC)
|
GND

25도라면 3.3V/2 = 1.65v가 ADC되어야함
3.3V *(NTC/(R1+NTC)) = ADC전압값
NTC = (ADC*R1)/(3.3V-ADC)
*/

#define VREF 3.3f           // ADC 기준 전압
#define R1 10000.0f   // 10kΩ 풀업 저항

typedef struct {
  float temperature;
  float resistance;
} NTC_Lookup;

/*
LNSK16G103 NTC써미스터
10kΩ (25도 기준)
온도에 따라 저항이 변함
온도가 높아질수록 저항이 감소
*/
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
  uint8_t err;
  float voltage;
  float resistance;
  
  voltage = driver_adc_single_read(g_adc_stm, ADC_STM32_S_CH_1, 1, &err);

  resistance = (voltage * R1) /(VREF - voltage);

  return ntc_resistance_to_temperature(resistance);
}

void status_led_init(void)
{
  led_freq_cfg_t cfg = {.freq = 5, .highDuty = 10};

  g_status_led = driver_led_open(LED_SYS_RUN);

  driver_led_set(g_status_led, LED_CMD_SET_TOGGLE_FREQ, &cfg);
  driver_led_set(g_status_led, LED_CMD_START, NULL);
}

void status_led_on(void)
{
  driver_led_set(g_status_led, LED_CMD_START, NULL);
}

void status_led_off(void)
{
  driver_led_set(g_status_led, LED_CMD_STOP, NULL);
}



void status_led_set(int mode)
{
  switch (mode)
  {
    case LED_BLINK:
       led_freq_cfg_t cfg = {.freq = 1, .highDuty = 10};
      driver_led_set(g_status_led, LED_CMD_SET_TOGGLE_FREQ, &cfg);
      break;
    case LED_ON:
    break;
    default:
      break;
  }
}