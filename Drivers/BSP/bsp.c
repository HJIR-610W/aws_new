
#include "bsp.h"

#include "bsp_di.h"
#include "bsp_do.h"
#include "driver_adc.h"
#include "driver_di.h"
#include "driver_do.h"
#include "driver_led.h"

static driver_t *g_power_cdma;
static driver_t *g_power_rain_detect_digital;
static driver_t *g_power_hart_24v;
static driver_t *g_power_rain_detect_analog;

static driver_t *g_adc_stm;
static driver_t *g_door_status;
static driver_t *g_port_mode;
static driver_t *g_status_led;


void bsp_status_led_init(void)
{
  led_freq_cfg_t cfg = {.freq = 5, .highDuty = 10};

  g_status_led = driver_led_open(LED_SYS_RUN);

  driver_led_set(g_status_led, LED_CMD_SET_TOGGLE_FREQ, &cfg);
  driver_led_set(g_status_led, LED_CMD_START, NULL);
}

void bsp_status_led_on(void) { driver_led_set(g_status_led, LED_CMD_START, NULL); }

void bsp_status_led_off(void) { driver_led_set(g_status_led, LED_CMD_STOP, NULL); }

void bsp_status_led_set(int mode)
{
  switch (mode)
  {
    case LED_BLINK:
    {
      led_freq_cfg_t cfg = {.freq = 1, .highDuty = 10};
      driver_led_set(g_status_led, LED_CMD_SET_TOGGLE_FREQ, &cfg);
    }
    break;
    case LED_ON:
      break;
    default:
      break;
  }
}

//CDMA 전원 제어 

void bsp_cdma_power_on(void)
{
  driver_do_high(g_power_cdma);
}

void bsp_cdma_power_off(void)
{
  driver_do_low(g_power_cdma);
}


void bsp_rain_digital_power_on(void)
{
  driver_do_high(g_power_rain_detect_digital);
}
void bsp_rain_digital_power_off(void)
{
  driver_do_low(g_power_rain_detect_digital);
}

void bsp_rain_analog_power_on(void)
{
  driver_do_high(g_power_rain_detect_analog);
}

void bsp_rain_analog_power_off(void)
{
  driver_do_low(g_power_rain_detect_analog);
}




void bsp_hart_24v_on(void)
{ 
  driver_do_high(g_power_hart_24v);
}

void bsp_hart_24v_off(void)
{ 
  driver_do_low(g_power_hart_24v); 
}


void bsp_power_init(void)
{
  g_power_cdma = driver_do_open(DO_POWER_CDMA, 0);
  bsp_cdma_power_on();

  g_power_rain_detect_digital = driver_do_open(DO_POWER_RAIN_DECT_DIGITAL, 0);
  bsp_rain_digital_power_on();

  g_power_rain_detect_analog = driver_do_open(DO_POWER_RAIN_DECT_ANALOG, 0);
  bsp_rain_analog_power_on();

  g_power_hart_24v = driver_do_open(DO_POWER_HART_24V, 0);
  bsp_hart_24v_on();

}








//RS232 D포트를 HART로 할지 RS232 할지 선택 


void bsp_set_portd_hart_mode(void)
{
  driver_do_high(g_port_mode);
}

void bsp_set_portd_rs232_mode(void)
{
  driver_do_low(g_port_mode);
}
void bsp_select_rs232_init(void)
{
  g_port_mode = driver_do_open(DO_HART_SEL, 0);

  bsp_set_portd_rs232_mode();
}

void bsp_door_status_init(void)
{
  g_door_status = driver_di_open(DI_EXT_0, 0);
}

bool bsp_door_opened(void)
{
  if(driver_di_read(g_door_status))
  {
    return false;
  }

  return true;
}




void bsp_adc_init(void)
{
  g_adc_stm = driver_adc_open(ADC_STM32, 0);
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
#define BATTERY_AVERAGE_SAMPLES 50
float bsp_read_battery(void)
{
  const float slope = 6;  // (float)(15.0f-0.0f)/(float)(2.5-0);
  const float offset = 0.0;
  uint8_t err;
  float voltage;
  float battery;

  voltage = driver_adc_single_read(g_adc_stm, ADC_STM32_S_CH_0, BATTERY_AVERAGE_SAMPLES, &err);

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

#define TEMP_AVERAGE_SAMPLES 50
float bsp_read_temperature(void)
{
  uint8_t err;
  float voltage;
  float resistance;

  voltage = driver_adc_single_read(g_adc_stm, ADC_STM32_S_CH_1, TEMP_AVERAGE_SAMPLES, &err);

  resistance = (voltage * R1) /(VREF - voltage);

  return ntc_resistance_to_temperature(resistance);
}

void bsp_init(void)
{
  bsp_rtc_init();
  bsp_power_init();
  bsp_door_status_init();
  bsp_select_rs232_init();
  bsp_adc_init();
  bsp_di_init();
  bsp_do_init();
  bsp_status_led_init();
}
