

#include "config_adc.h"

#include "app_version.h"
#include "config_memory_map.h"
#include "adc_calibration.h"
#include "crc.h"
#include <math.h>

config_adc_t g_config_adc;

config_adc_t g_config_adc_default = {
    .single[0] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[1] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[2] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[3] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[4] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[5] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[6] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[7] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[8] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[9] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[10] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[11] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[12] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[13] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[14] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[15] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[16] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[17] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[18] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[19] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[20] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[21] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[22] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[23] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[24] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[25] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[26] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[27] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[28] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[29] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[30] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .single[31] =
        {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},

    .diff[0] = {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .diff[1] = {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .diff[2] = {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .diff[3] = {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .diff[4] = {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .diff[5] = {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .diff[6] = {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1},
    .diff[7] = {.offset = 10, .fullset = 1000, .offset_input = 0, .fullset_input = 5000, .gain = 1}
};

void save_config_adc(void)
{
  uint32_t crc;
  uint8_t temp;

  g_config_adc.start = 0;
  crc = crc32_hw_with_padding(&g_config_adc.start,
                              sizeof(config_adc_t) - sizeof(g_config_adc.header));

  g_config_adc.header.magicNum = CONFIG_MAGIC;
  g_config_adc.header.crc = crc;
  g_config_adc.header.version = get_app_version();

  fram_write(CONFIG_ADC_START_ADDRESS, (uint8_t *)&g_config_adc, sizeof(g_config_adc));
}



void load_config_adc(void)
{


#if 0
    config_adc_t *p_config = aws_malloc(sizeof(config_adc_t));

  fram_read(CONFIG_ADC_START_ADDRESS, (uint8_t *)p_config, sizeof(config_adc_t));

  if (p_config->header.magicNum == CONFIG_MAGIC)
  {
    crc = crc32_hw_with_padding(&p_config->single,
                                sizeof(config_adc_t) - sizeof(p_config->header));
    if( crc == p_config->header.crc)
    {
      memcpy(g_config_adc, p_config, sizeof(config_adc_t));
      crc_result = true;
    }
  }
  if(crc_result == false)
  {
   //config_adc_reset();
  }
  aws_free(p_config);
  #endif
}

config_adc_t *get_config_adc(void)
{
  // 필요시 적절한 조치 처리
  return &g_config_adc;
}


void config_adc_reset(void)
{
  g_config_adc = g_config_adc_default;
}

extern config_adc_nvm_t g_adc_config_nvm;

void save_adc_cali(void)
{
  fram_write(CONFIG_CALI_START_ADDRESS, (uint8_t *)&g_adc_config_nvm, sizeof(g_adc_config_nvm));
}

void load_adc_cali(void)
{
  fram_read(CONFIG_CALI_START_ADDRESS, (uint8_t *)&g_adc_config_nvm, sizeof(g_adc_config_nvm));
  adc_config_map();
}