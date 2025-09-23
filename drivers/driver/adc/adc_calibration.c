

#include "adc_calibration.h"
#include "config_adc.h"
#include "system_err.h"
config_adc_nvm_t g_adc_config_nvm;
config_adc_adv_t g_adc_config_stm32;
config_adc_adv_t g_adc_config_ads1220;


float g_current_temp = 25.0f;  // 공장 초기화시 온도가 25라고 하자

int32_t (*adc_printf)(const char* , ...);

config_adc_adv_t *get_adc_config(int type)
{
  if(type==0)
  {
    return &g_adc_config_ads1220;
  }

    return &g_adc_config_stm32;



}

    void
    set_adc_printf(void* func)
{
  adc_printf = (int32_t (*)(const char* , ...))func;
}



// ---  LUT 보간 함수 ---
/** @brief 온도 LUT에서 현재 온도에 해당하는 보상 계수를 선형 보간합니다. LUT는 온도로 정렬되어
 * 있어야 합니다. */
 bool interpolate_lut(const temp_lut_point_t lut[], uint8_t size, float current_temp,
                                float* interp_slope_mult, float* interp_offset_corr)
{
  if (lut == NULL || size == 0 || interp_slope_mult == NULL || interp_offset_corr == NULL)
  {
    return false;  // 기본 파라미터 오류
  }

  // LUT 크기가 1인 경우
  if (size == 1)
  {
    *interp_slope_mult = lut[0].slope_multiplier;
    *interp_offset_corr = lut[0].offset_correction;
    return true;
  }

  // 현재 온도가 LUT 범위 밖인 경우: 가장 가까운 끝점 값 사용 (Clamping)
  if (current_temp <= lut[0].temperature)
  {
    *interp_slope_mult = lut[0].slope_multiplier;
    *interp_offset_corr = lut[0].offset_correction;
    return true;
  }
  if (current_temp >= lut[size - 1].temperature)
  {
    *interp_slope_mult = lut[size - 1].slope_multiplier;
    *interp_offset_corr = lut[size - 1].offset_correction;
    return true;
  }

  // 현재 온도를 포함하는 두 LUT 포인트 찾기 (LUT는 온도로 정렬 가정)
  for (uint8_t i = 0; i < size - 1; ++i)
  {
    if (current_temp >= lut[i].temperature && current_temp <= lut[i + 1].temperature)
    {
      const temp_lut_point_t* p1 = &lut[i];
      const temp_lut_point_t* p2 = &lut[i + 1];

      // 선형 보간
      float temp_range = p2->temperature - p1->temperature;
      // 온도 범위가 0에 가까우면 보간 불가 (또는 p1 값 사용)
      if (fabsf(temp_range) < 1e-6f)
      {
        *interp_slope_mult = p1->slope_multiplier;
        *interp_offset_corr = p1->offset_correction;
        return true;
      }

      float ratio = (current_temp - p1->temperature) / temp_range;

      *interp_slope_mult =
          p1->slope_multiplier + ratio * (p2->slope_multiplier - p1->slope_multiplier);
      *interp_offset_corr =
          p1->offset_correction + ratio * (p2->offset_correction - p1->offset_correction);
      return true;
    }
  }

  // 여기까지 오면 안됨 (범위 체크에서 걸렸어야 함)
  if (adc_printf)
    adc_printf("오류: LUT 보간 중 로직 오류.\n");
  return false;
}


void adc_config_map(void)
{
  g_adc_config_ads1220.bits = &g_adc_config_nvm.ads1220_bits;
  g_adc_config_ads1220.single_ended_cal = g_adc_config_nvm.ads1220_se_cal;
  g_adc_config_ads1220.params_se_cnt = ADS1220_NUM_SINGLE_ENDED_CHANNELS;

  g_adc_config_ads1220.differential_cal = g_adc_config_nvm.ads1220_di_cal;
  g_adc_config_ads1220.params_di_cnt = ADS1220_NUM_DIFFERENTIAL_CHANNELS;

  g_adc_config_stm32.bits = &g_adc_config_nvm.stm32_bits;
  g_adc_config_stm32.single_ended_cal = g_adc_config_nvm.stm32_se_cal;
  g_adc_config_stm32.params_se_cnt = STM32_NUM_SINGLE_ENDED_CHANNELS;

  g_adc_config_stm32.params_di_cnt = 0;
}


// --- 4. 초기화 함수 ---
bool adc_config_init(config_adc_adv_t* cfg, uint32_t resolution_bits, float reference_voltage)
{
  if (cfg == NULL || resolution_bits == 0 || resolution_bits > 32 || reference_voltage <= 0.0f)
  {
    if (adc_printf)
      adc_printf("오류: adc_config_init 파라미터 오류.\n");
    return false;
  }



  cfg->bits->resolution_bits = resolution_bits;
  cfg->bits->reference_voltage = reference_voltage;

  int32_t range_limit = (1L << (resolution_bits - 1));  // 상위 비트는 부호 비트
  cfg->bits->min_raw_value = -range_limit;
  cfg->bits->max_raw_value = range_limit - 1;

  for (int i = 0; i < cfg->params_se_cnt; ++i)
  {
    cfg->single_ended_cal[i] = (adc_cal_params_t){.factory_slope = 1.0f,
                                                  .factory_offset = 0.0f,
                                                  .factory_cal_temp = DEFAULT_FACTORY_CAL_TEMP,
                                                  .is_calibrated = false,
                                                  .comp_method = TEMP_COMP_NONE,  // 기본: 보상 없음
                                                  .slope_temp_coeff = 0.0f,
                                                  .offset_temp_coeff = 0.0f,
                                                  .lut_size = 0};
  }
  for (int i = 0; i < cfg->params_di_cnt; ++i)
  {
    cfg->differential_cal[i] =
        (adc_cal_params_t){.factory_slope = 1.0f,
                           .factory_offset = 0.0f,
                           .factory_cal_temp = DEFAULT_FACTORY_CAL_TEMP,
                           .is_calibrated = false,
                           .comp_method = TEMP_COMP_NONE,
                           .slope_temp_coeff = 0.0f,
                           .offset_temp_coeff = 0.0f,
                           .lut_size = 0};
  }
  if (adc_printf)
    adc_printf("ADC 설정 초기화 완료: Res=%u, Vref=%.2fV, MaxRaw=%u\n", cfg->bits->resolution_bits,
               cfg->bits->reference_voltage, cfg->bits->max_raw_value);
  return true;
}


bool adc_perform_factory_calibration( adc_cal_params_t* cal_params,
                                     adc_cal_point_t p1, adc_cal_point_t p2, float cal_temp)
{
  if (!cal_params )
    return false;

  // 전압 = ADC*기울기 + 오프셋
  // 오프셋 = 전압 - ADC*기울기


  cal_params->factory_slope = (p2.reference_value - p1.reference_value) / (float)(p2.raw_value - p1.raw_value);
  cal_params->factory_offset = p1.reference_value - cal_params->factory_slope * (float)p1.raw_value;
  cal_params->factory_offset_trim = 0;
  cal_params->factory_cal_temp = cal_temp;
  cal_params->p1_cal_point = p1;
  cal_params->p2_cal_point = p2;

  cal_params->slope_temp_coeff =0;
  cal_params->offset_temp_coeff = 0;
  cal_params->lut_size = 0;

  cal_params->is_calibrated = true;
  DEBUG_PRINTF("캘리브레이션 성공 (%.1fC): Slope=%.6f, Offset=%.6f\n", cal_temp, cal_params->factory_slope, cal_params->factory_offset);

  return true;
}





// --- 최종 보상 값 계산 함수 (보상 방법 선택 로직 포함) ---
float adc_get_compensated_value(int32_t raw_value, const adc_cal_params_t* cal_params,
                                float current_temperature)
{
  if (!cal_params || !cal_params->is_calibrated)
  {
    return NAN;
  }

  float effective_slope = cal_params->factory_slope;
  float effective_offset = cal_params->factory_offset + cal_params->factory_offset_trim;

  switch (cal_params->comp_method)
  {
    case TEMP_COMP_COEFF:
    {
      float delta_temp = current_temperature - cal_params->factory_cal_temp;
      effective_slope *= (1.0f + cal_params->slope_temp_coeff * delta_temp);
      effective_offset += cal_params->offset_temp_coeff * delta_temp;
      break;
    }
    #ifdef ADC_LUT
    case TEMP_COMP_LUT:
    {
      float slope_mult = 1.0f;
      float offset_corr = 0.0f;
      if (interpolate_lut(cal_params->temp_comp_lut, cal_params->lut_size, current_temperature,
                          &slope_mult, &offset_corr))
      {
        effective_slope *= slope_mult;
        effective_offset += offset_corr;
      }
      else
      {
        // fprintf(stderr, "경고: LUT 보간 실패, 공장 캘리브레이션 값 사용.\n");
        // 보간 실패 시 공장 값 사용 (위에서 이미 초기화됨)
      }
      break;
    }
#endif
    case TEMP_COMP_NONE:
    default:
      // 보상 없음, 공장 값 그대로 사용
      break;
  }

  return effective_slope * (float)raw_value + effective_offset;
}



// --- 동적 오프셋 조정 함수 (보상 방법 고려) ---
bool adc_perform_offset_adjustment(const config_adc_adv_t* adc_config, adc_cal_params_t* cal_params,
                                   adc_channel_type_t ch_type, int ch_idx, float current_temp,
                                   float target_ref, int32_t raw_now)
{

  if (!adc_config || !cal_params || !cal_params->is_calibrated)
    return false;



  float eff_slope = cal_params->factory_slope;
  float offset_correction = 0.0f;

  // 현재 온도에서의 유효 기울기 및 오프셋 보정량 계산
  switch (cal_params->comp_method)
  {
    case TEMP_COMP_COEFF:
    {
      float delta_temp = current_temp - cal_params->factory_cal_temp;
      eff_slope *= (1.0f + cal_params->slope_temp_coeff * delta_temp);
      offset_correction = cal_params->offset_temp_coeff * delta_temp;
      break;
    }
    #ifdef ADC_LUT
    case TEMP_COMP_LUT:
    {
      float slope_mult = 1.0f;
      if (interpolate_lut(cal_params->temp_comp_lut, cal_params->lut_size, current_temp,
                          &slope_mult, &offset_correction))
      {
        eff_slope *= slope_mult;
      }  // 보간 실패 시 factory_slope 사용, offset_correction은 0.0 유지
      break;
    }
#endif
    case TEMP_COMP_NONE:
    default:
      break;  // 보상 없음
  }

  // 새 factory_offset 계산: target = eff_slope * raw + (new_factory_offset + offset_correction)
  float new_factory_offset = target_ref - eff_slope * (float)raw_now - offset_correction;

  if (adc_printf)
    adc_printf(
        "채널 %d 오프셋 조정 (%.1f°C): Raw=%u, 목표=%.3f -> 새 Factory Offset=%.6f (기존=%.6f)\n",
        ch_idx, current_temp, raw_now, target_ref, new_factory_offset, cal_params->factory_offset);

  cal_params->factory_offset = new_factory_offset;
  save_adc_cali();
  return true;
}

void populate_lut(adc_cal_params_t* params)
{
#ifdef ADC_LUT
  if (!params || MAX_LUT_SIZE < 3)
    return;  // 최소 3개 포인트 가정
  params->lut_size = 3;
  // 온도 오름차순으로 정렬되어야 함
  params->temp_comp_lut[0] = (temp_lut_point_t){
      .temperature = 0.0f, .slope_multiplier = 1.02f, .offset_correction = -0.05f};
  params->temp_comp_lut[1] = (temp_lut_point_t){
      .temperature = 25.0f, .slope_multiplier = 1.00f, .offset_correction = 0.00f}; 
  params->temp_comp_lut[2] = (temp_lut_point_t){
      .temperature = 50.0f, .slope_multiplier = 0.98f, .offset_correction = 0.08f};
#endif

}


float read_current_temperature(void) 
{ 
  return g_current_temp; 
}



float adc_driver_get_value(config_adc_adv_t *cfg,adc_channel_type_t channel_type, int channel_index,
                           int32_t raw_value)
{
  const adc_cal_params_t* cal_params;

  switch (channel_type)
  {
    case ADC_CHANNEL_TYPE_SINGLE_ENDED:
      if (channel_index < 0 || channel_index >= cfg->params_se_cnt)
        return NAN;
      cal_params = &cfg->single_ended_cal[channel_index];
      break;
    case ADC_CHANNEL_TYPE_DIFFERENTIAL:
      if (channel_index < 0 || channel_index >= cfg->params_di_cnt)
        return NAN;
      cal_params = &cfg->differential_cal[channel_index];
      break;
  }


  return adc_get_compensated_value(raw_value, cal_params, 0);
}

bool adc_driver_adjust_offset(config_adc_adv_t *cfg,adc_channel_type_t channel_type, int channel_index,
                              float target_reference_value, int32_t raw_value)
{
  adc_cal_params_t* cal_params_rw;
  if (channel_type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
  {
    if (channel_index < 0 || channel_index >= cfg->params_se_cnt)
      return false;
    cal_params_rw = &cfg->single_ended_cal[channel_index];
  }
  else if (channel_type == ADC_CHANNEL_TYPE_DIFFERENTIAL)
  {
    if (channel_index < 0 || channel_index >= cfg->params_di_cnt)
      return false;
    cal_params_rw = &cfg->differential_cal[channel_index];
  }
  else
  {
    return false;
  }
  float current_temp = read_current_temperature();
  bool success = adc_perform_offset_adjustment(cfg, cal_params_rw, channel_type, channel_index,
                                               current_temp, target_reference_value, raw_value);
  if(success)
  { 
    save_adc_cali();
  }
  return success;
}

bool adc_driver_adjust_offset_trim(config_adc_adv_t* cfg, adc_channel_type_t channel_type,
                              int channel_index,float offset_trim)
{
  adc_cal_params_t* cal_params_rw;
  if (channel_type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
  {
    if (channel_index < 0 || channel_index >= cfg->params_se_cnt)
      return false;
    cal_params_rw = &cfg->single_ended_cal[channel_index];
  }
  else if (channel_type == ADC_CHANNEL_TYPE_DIFFERENTIAL)
  {
    if (channel_index < 0 || channel_index >= cfg->params_di_cnt)
      return false;
    cal_params_rw = &cfg->differential_cal[channel_index];
  }
  else
  {
    return false;
  }

  cal_params_rw->factory_offset_trim = offset_trim;

  save_adc_cali();
  
  return true;

}

bool adc_driver_read_offset_trim(config_adc_adv_t* cfg, adc_channel_type_t channel_type,
                                   int channel_index, float *offset_trim)
{
  adc_cal_params_t* cal_params_rw;
  if (channel_type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
  {
    if (channel_index < 0 || channel_index >= cfg->params_se_cnt)
      return false;
    cal_params_rw = &cfg->single_ended_cal[channel_index];
  }
  else if (channel_type == ADC_CHANNEL_TYPE_DIFFERENTIAL)
  {
    if (channel_index < 0 || channel_index >= cfg->params_di_cnt)
      return false;
    cal_params_rw = &cfg->differential_cal[channel_index];
  }
  else
  {
    return false;
  }

  *offset_trim = cal_params_rw->factory_offset_trim;

  return true;

}
