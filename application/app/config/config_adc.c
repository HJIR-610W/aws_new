

#include "config_adc.h"

#include <math.h>

#include "app_file.h"
#include "app_version.h"
#include "config_memory_map.h"
#include "adc_calibration.h"
#include "debug_io.h"
#include "drv_crc.h"
#include "ff.h"
#include "system_err.h"
#include "drv_fram.h"
#include "utils\util_stdio.h"

extern config_adc_t g_config_adc;

void save_config_adc_cali(void)
{
  uint32_t crc;
  uint8_t *p_start;
  uint32_t config_len;

  p_start = (uint8_t *)(&g_config_adc) + sizeof(g_config_adc.header);
  config_len = sizeof(config_adc_t) - sizeof(g_config_adc.header);
  crc = drv_crc32_with_padding(p_start, config_len);

  g_config_adc.header.magicNum = CONFIG_MAGIC;
  g_config_adc.header.crc = crc;
  g_config_adc.header.version = CONFIG_ADC_CALI_VERSION;

  drv_fram_write(CONFIG_CALI_START_ADDRESS, (uint8_t *)&g_config_adc, sizeof(g_config_adc));
}



void load_config_adc_cali(void)
{
  drv_fram_read(CONFIG_CALI_START_ADDRESS, (uint8_t *)&g_config_adc, sizeof(g_config_adc));

  //fram이 0xFF상태에서는 is_calibrated가 참으로 평가되는 문제처리 
  for (int i = 0; i < ADS1220_NUM_SINGLE_ENDED_CHANNELS;i++)
  {
    g_config_adc.ads1220_se_cal[i].is_calibrated = normalize_bool((uint8_t)g_config_adc.ads1220_se_cal[i].is_calibrated);
  }
  for (int i = 0; i < ADS1220_NUM_DIFFERENTIAL_CHANNELS; i++)
  {
    g_config_adc.ads1220_di_cal[i].is_calibrated = normalize_bool((uint8_t)g_config_adc.ads1220_di_cal[i].is_calibrated);
  }
  g_config_adc.stm32_bits.resolution_bits = 12;
  g_config_adc.stm32_bits.reference_voltage=3.3;
  g_config_adc.stm32_bits.min_raw_value=0; ///< ADC 최소 원시 값 (예: -2^23)
  g_config_adc.stm32_bits.max_raw_value=4095; ///< ADC 최대 원시 값 (예: 2^23 - 1)

  //STM32는 하드웨어적으로 켈리브레이션을 못하게 되어 있어서 이상적인 조건으로 강제 설정
  for (int i = 0; i < STM32_NUM_SINGLE_ENDED_CHANNELS; i++)
  {
    g_config_adc.stm32_se_cal[i].comp_method = TEMP_COMP_NONE;
    g_config_adc.stm32_se_cal[i].factory_slope = 8.05e-04f;//(3.3-0)/4095
    g_config_adc.stm32_se_cal[i].factory_offset=0;
    g_config_adc.stm32_se_cal[i].factory_offset_trim=0;
    g_config_adc.stm32_se_cal[i].factory_cal_temp = 25;
    g_config_adc.stm32_se_cal[i].is_calibrated = true;

    g_config_adc.stm32_se_cal[i].offset_temp_coeff = 1;
    g_config_adc.stm32_se_cal[i].slope_temp_coeff = 1;
    g_config_adc.stm32_se_cal[i].p1_cal_point.raw_value = 0;
    g_config_adc.stm32_se_cal[i].p1_cal_point.reference_value = 0;
    g_config_adc.stm32_se_cal[i].p2_cal_point.raw_value = 4095;
    g_config_adc.stm32_se_cal[i].p2_cal_point.reference_value = 3.3;
  }

  adc_config_map();
  


  DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"sizeof(config_adc_t):%d",sizeof(config_adc_t));
}

#define PATH_ADC_CALIBRATION_BIN "0:back_up/adc_calibraion.bin"

void backup_adc_calibration(void)
{
    FRESULT f_ret;

    make_path(PATH_ADC_CALIBRATION_BIN);
      
      
    f_ret = write_file(PATH_ADC_CALIBRATION_BIN, (uint8_t *)&g_config_adc, sizeof(g_config_adc), 0);
    if (f_ret == FR_OK)
    {
          DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"%s에 저장되었습니다\r\n",PATH_ADC_CALIBRATION_BIN);
    }
}