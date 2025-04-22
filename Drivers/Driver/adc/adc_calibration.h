#ifndef APP_CALIBRATION_H
#define APP_CALIBRATION_H

#include <math.h>  
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NUM_SINGLE_ENDED_CHANNELS 20//18 ads1210 2 STM32
#define NUM_DIFFERENTIAL_CHANNELS 8
#define DEFAULT_FACTORY_CAL_TEMP 25.0f
#define MAX_LUT_SIZE 10  // 온도 보상 LUT 최대 크기


typedef enum
{
  ADC_CHANNEL_TYPE_SINGLE_ENDED = 0,
  ADC_CHANNEL_TYPE_DIFFERENTIAL = 1
} adc_channel_type_t;

typedef enum
{
  TEMP_COMP_NONE = 0,   // 온도 보상 없음
  TEMP_COMP_COEFF = 1,  // 온도 계수 사용
  TEMP_COMP_LUT = 2     // 룩업 테이블(LUT) 사용
} temp_comp_method_t;


typedef struct
{
  float temperature;        // 해당 엔트리의 온도 (C)
  float slope_multiplier;   // factory_slope에 곱할 값 (기준온도에서 1.0)
  float offset_correction;  // factory_offset에 더할 값 (기준온도에서 0.0)
} temp_lut_point_t;


typedef struct
{
  // 공장 캘리브레이션 정보
  float factory_slope;
  float factory_offset;
  float factory_cal_temp;
  bool is_calibrated;

  // 보상 방법 선택
  temp_comp_method_t comp_method;

  // 방법 1: 계수 사용 시
  float slope_temp_coeff;
  float offset_temp_coeff;
#ifdef ADC_LUT
  // 방법 2: LUT 사용 시 (NVM 로드/저장 필요)
  temp_lut_point_t temp_comp_lut[MAX_LUT_SIZE];
#endif
  uint8_t lut_size;  // LUT에 저장된 실제 포인트 수


} adc_cal_params_t;


typedef struct
{
  uint32_t resolution_bits;
  float reference_voltage;
  uint32_t max_raw_value;
  adc_cal_params_t single_ended_cal[NUM_SINGLE_ENDED_CHANNELS];
  adc_cal_params_t differential_cal[NUM_DIFFERENTIAL_CHANNELS];
} config_adc_adv_t;


typedef struct
{
  uint32_t raw_value;
  float reference_value;
} adc_cal_point_t;

bool adc_config_init(config_adc_adv_t* adc_config, uint32_t resolution_bits,
                     float reference_voltage);

float adc_driver_get_value(adc_channel_type_t channel_type, int channel_index, int32_t raw_value);

bool adc_perform_factory_calibration(config_adc_adv_t* adc_config, adc_cal_params_t* cal_params,
                                     adc_cal_point_t p1, adc_cal_point_t p2, float cal_temp);

void set_adc_printf(void* func);


float read_current_temperature(void) ;

float adc_get_compensated_value(uint32_t raw_value, const adc_cal_params_t* cal_params,
                                float current_temperature);

bool adc_perform_offset_adjustment(const config_adc_adv_t* adc_config, adc_cal_params_t* cal_params,
                                   adc_channel_type_t ch_type, int ch_idx, float current_temp,
                                   float target_ref, int32_t raw_now);
#endif