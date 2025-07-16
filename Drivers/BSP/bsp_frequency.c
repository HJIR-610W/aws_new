#include "bsp_frequency.h"

#include <stdint.h>
#include <math.h>
#include "cmsis_os2.h"

#include "driver_stm32_frequency.h"

// 정의가 누락된 상수들을 임시로 정의
#define STM32_FREQ_CH_0 0
#define STM32_FREQ_CH_1 1



typedef enum
{
  FREQ_DRIVER_STM32
} freq_driver_type_t;

typedef struct
{
  freq_driver_type_t driver_type;
  int driver_num;
} freq_pinmap_t;

static const freq_pinmap_t freq_pinmap[BSP_FREQ_MAX] = {
    [BSP_FREQ_STM32_CH_0] = {FREQ_DRIVER_STM32, STM32_FREQ_CH_0},
    [BSP_FREQ_STM32_CH_1] = {FREQ_DRIVER_STM32, STM32_FREQ_CH_1}};

static inline bool is_valid_freq_num(int num)
{
  return (num >= 0 && num < BSP_FREQ_MAX);
}

static inline const freq_pinmap_t* get_freq_pinmap(int num)
{
  if (!is_valid_freq_num(num))
  {
    return 0;
  }
  return &freq_pinmap[num];
}

void bsp_frequency_init(void)
{
 // stm32_frequency_init();
  

}

float bsp_frequency_read(int channel, uint8_t *err)
{
  const freq_pinmap_t* pinmap = get_freq_pinmap(channel);
  
  if (!pinmap)
  {
    if (err) *err = 1;
    return NAN;
  }

  switch (pinmap->driver_type)
  {
    case FREQ_DRIVER_STM32:
    //  return stm32_frequency_read(pinmap->driver_num, err);
  
    default:
      if (err) *err = 1;
      return NAN;
  }
}

void bsp_frequency_start_measurement(int channel)
{
  const freq_pinmap_t* pinmap = get_freq_pinmap(channel);
  if (!pinmap)
  {
    return;
  }

  switch (pinmap->driver_type)
  {
    case FREQ_DRIVER_STM32:
     // stm32_frequency_start_measurement(pinmap->driver_num);
      break;

    default:
      break;
  }
}

void bsp_frequency_stop_measurement(int channel)
{
  const freq_pinmap_t* pinmap = get_freq_pinmap(channel);
  if (!pinmap)
  {
    return;
  }

  switch (pinmap->driver_type)
  {
    case FREQ_DRIVER_STM32:
     // stm32_frequency_stop_measurement(pinmap->driver_num);
      break;
    default:
      break;
  }
}

bool bsp_frequency_is_measuring(int channel)
{
  const freq_pinmap_t* pinmap = get_freq_pinmap(channel);
  if (!pinmap)
  {
    return false;
  }

  switch (pinmap->driver_type)
  {
    case FREQ_DRIVER_STM32:
    //  return stm32_frequency_is_measuring(pinmap->driver_num);
    default:
      return false;
  }
  
}