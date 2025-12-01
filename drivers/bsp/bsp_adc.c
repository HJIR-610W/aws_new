
#include "bsp_adc.h"

#include <stdint.h>
#include <math.h>

#include "adc_calibration.h"
#include "ads1220.h"
#include "driver_adc_define.h"
#include "bsp_stm32_adc.h"

typedef enum
{
  ADC_DRIVER_STM32,
  ADC_DRIVER_ADS1220
} adc_driver_type_t;

typedef struct
{
  adc_driver_type_t driver_type;
  int driver_num;
} adc_pinmap_t;

static const adc_pinmap_t adc_pinmap[BSP_ADC_MAX] = {
    [BSP_ADS1220_S_CH_0] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_0},
    [BSP_ADS1220_S_CH_1] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_1},
    [BSP_ADS1220_S_CH_2] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_2},
    [BSP_ADS1220_S_CH_3] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_3},
    [BSP_ADS1220_S_CH_4] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_4},
    [BSP_ADS1220_S_CH_5] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_5},
    [BSP_ADS1220_S_CH_6] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_6},
    [BSP_ADS1220_S_CH_7] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_7},
    [BSP_ADS1220_S_CH_8] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_8},
    [BSP_ADS1220_S_CH_9] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_9},
    [BSP_ADS1220_S_CH_10] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_10},
    [BSP_ADS1220_S_CH_11] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_11},
    [BSP_ADS1220_S_CH_12] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_12},
    [BSP_ADS1220_S_CH_13] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_13},
    [BSP_ADS1220_S_CH_14] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_14},
    [BSP_ADS1220_S_CH_15] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_15},
    [BSP_ADS1220_S_CH_16] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_16},
    [BSP_ADS1220_S_CH_17] = {ADC_DRIVER_ADS1220, ADC_ADS1220_S_CH_17},
    [BSP_ADC_SYS_BATTERY] = {ADC_DRIVER_STM32, STM32_ADC_SE_CH_0_VOLT},
    [BSP_ADC_SYS_TEMP] = {ADC_DRIVER_STM32, STM32_ADC_SE_CH_1_TEMP},
    [BSP_ADS1220_D_CH_0] = {ADC_DRIVER_ADS1220, ADC_ADS1220_D_CH_0},
    [BSP_ADS1220_D_CH_1] = {ADC_DRIVER_ADS1220, ADC_ADS1220_D_CH_1},
    [BSP_ADS1220_D_CH_2] = {ADC_DRIVER_ADS1220, ADC_ADS1220_D_CH_2},
    [BSP_ADS1220_D_CH_3] = {ADC_DRIVER_ADS1220, ADC_ADS1220_D_CH_3},
    [BSP_ADS1220_D_CH_4] = {ADC_DRIVER_ADS1220, ADC_ADS1220_D_CH_4},
    [BSP_ADS1220_D_CH_5] = {ADC_DRIVER_ADS1220, ADC_ADS1220_D_CH_5},
    [BSP_ADS1220_D_CH_6] = {ADC_DRIVER_ADS1220, ADC_ADS1220_D_CH_6},
    [BSP_ADS1220_D_CH_7] = {ADC_DRIVER_ADS1220, ADC_ADS1220_D_CH_7}};

static inline bool is_valid_adc_num(int num)
{
  return (num >= 0 && num < BSP_ADC_MAX);
}

static inline const adc_pinmap_t* get_adc_pinmap(int num)
{
  if (!is_valid_adc_num(num))
  {
    return 0;
  }
  return &adc_pinmap[num];
}


void bsp_adc_init(void)
{
  stm32_adc_init();
  ads1220_init();
}


float bsp_adc_single_read_voltage(int channel, uint16_t avg, uint8_t *err)
{
  const adc_pinmap_t* pinmap = get_adc_pinmap(channel);
  int32_t raw_now;
  float voltage = 0.0f;

  if (!pinmap)
  {
    return NAN;
  }

  switch (pinmap->driver_type)
  {
  case ADC_DRIVER_STM32:
    raw_now = stm32_adc_read_single(pinmap->driver_num, avg, err);
    voltage = adc_driver_get_value(get_adc_config(1), ADC_CHANNEL_TYPE_SINGLE_ENDED,
                                   pinmap->driver_num, raw_now);
    break;
  case ADC_DRIVER_ADS1220:
    raw_now = ads1220_single_read(channel, avg, err);
    voltage = adc_driver_get_value(get_adc_config(0), ADC_CHANNEL_TYPE_SINGLE_ENDED, channel, raw_now);
      default:
    break;
  }

  return voltage;
}

float bsp_adc_diff_read_voltage(int channel, uint16_t avg, uint8_t *err)
{
  const adc_pinmap_t* pinmap = get_adc_pinmap(channel);
  if (!pinmap)
  {
    return NAN;
  }

  int32_t raw_now;
  float voltage = 0.0f;

  switch (pinmap->driver_type)
  {
    case ADC_DRIVER_ADS1220:
      raw_now = ads1220_diff_read(channel, avg, err);
      voltage = adc_driver_get_value(get_adc_config(0), ADC_CHANNEL_TYPE_DIFFERENTIAL, channel, raw_now);
      break;
    default:
      break;
  }
  
  return voltage;
}

int32_t bsp_adc_single_raw_read(int channel, uint16_t avg, uint8_t *err)
{
  const adc_pinmap_t* pinmap = get_adc_pinmap(channel);
  if (!pinmap) {
    return 0;
  }

  switch (pinmap->driver_type) {
    case ADC_DRIVER_STM32:
      return stm32_adc_read_single(pinmap->driver_num, avg, err);
    case ADC_DRIVER_ADS1220:
      return ads1220_single_read(channel, avg, err);
    default:
      return 0;
  }
}

int32_t bsp_adc_diff_raw_read(int channel, uint16_t avg, uint8_t *err)
{
  const adc_pinmap_t* pinmap = get_adc_pinmap(channel);
  if (!pinmap)
  {
    return 0;
  }

  switch (pinmap->driver_type)
  {
    case ADC_DRIVER_ADS1220:
      return ads1220_diff_read(channel, avg, err);
    default:
      return 0;
  }
}

void bsp_adc_set_offset(int channel, float offset)
{
  const adc_pinmap_t* pinmap = get_adc_pinmap(channel);
  if (!pinmap) {
    return;
  }

  switch (pinmap->driver_type) {
    case ADC_DRIVER_ADS1220:
      adc_driver_adjust_offset_trim(get_adc_config(0), ADC_CHANNEL_TYPE_SINGLE_ENDED, channel, offset);
      break;
    default:
      break;
  }
}

bool bsp_adc_get_offset(int channel, float *p_offset)
{
  const adc_pinmap_t* pinmap = get_adc_pinmap(channel);
  float offset;
  bool ret = false;

  if (!pinmap)
  {
    return false;
  }

  switch (pinmap->driver_type) {
    case ADC_DRIVER_ADS1220:
      ret = adc_driver_read_offset_trim(get_adc_config(0), ADC_CHANNEL_TYPE_SINGLE_ENDED, channel, &offset);
      if (ret == 0) {
        *p_offset = offset;
      }
      break;
    default:
      break;
  }

  return ret;
}