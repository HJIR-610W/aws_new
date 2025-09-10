

#include "config_adc.h"

#include "app_version.h"
#include "config_memory_map.h"
#include "adc_calibration.h"
#include "drv_crc.h"
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





extern config_adc_nvm_t g_adc_config_nvm;

void save_adc_cali(void)
{
  drv_fram_write(CONFIG_CALI_START_ADDRESS, (uint8_t *)&g_adc_config_nvm, sizeof(g_adc_config_nvm));
}

void load_adc_cali(void)
{
  drv_fram_read(CONFIG_CALI_START_ADDRESS, (uint8_t *)&g_adc_config_nvm, sizeof(g_adc_config_nvm));
  adc_config_map();
}