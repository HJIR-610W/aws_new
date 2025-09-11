

#include "config_adc.h"

#include <math.h>

#include "app_file.h"
#include "app_version.h"
#include "config_memory_map.h"
#include "adc_calibration.h"
#include "dev_io.h"
#include "drv_crc.h"
#include "ff.h"

#include "drv_fram.h"

extern config_adc_nvm_t g_adc_config_nvm;

void save_adc_cali(void)
{
  uint32_t crc;
  uint8_t *p_start;
  uint32_t config_len;

  p_start = (uint8_t *)(&g_adc_config_nvm) + sizeof(g_adc_config_nvm.header);
  config_len = sizeof(config_adc_nvm_t) - sizeof(g_adc_config_nvm.header);
  crc = drv_crc32_with_padding(p_start, config_len);

  g_adc_config_nvm.header.magicNum = CONFIG_MAGIC;
  g_adc_config_nvm.header.crc = crc;
  g_adc_config_nvm.header.version = get_app_version(NULL, NULL, NULL, NULL);

  drv_fram_write(CONFIG_CALI_START_ADDRESS, (uint8_t *)&g_adc_config_nvm, sizeof(g_adc_config_nvm));
}

void load_adc_cali(void)
{
  drv_fram_read(CONFIG_CALI_START_ADDRESS, (uint8_t *)&g_adc_config_nvm, sizeof(g_adc_config_nvm));
  adc_config_map();
}

#define PATH_ADC_CALIBRATION_BIN "0:back_up/adc_calibraion.bin"

void backup_adc_calibration(void)
{
    FRESULT f_ret;

    make_path(PATH_ADC_CALIBRATION_BIN);
      
      
    f_ret = write_file(PATH_ADC_CALIBRATION_BIN, (uint8_t *)&g_adc_config_nvm, sizeof(g_adc_config_nvm), 0);
    if (f_ret == FR_OK)
    {
        io_printf("%s에 저장되었습니다\r\n",PATH_ADC_CALIBRATION_BIN);
    }
}