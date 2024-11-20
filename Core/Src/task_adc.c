



float calibrate_adc_with_offset(uint32_t adc_raw, float adc_offset)
{
    // 기울기와 절편 계산
    float m = (VOLTAGE_MAX - VOLTAGE_MIN) / (ADC_MAX_RAW - ADC_MIN_RAW);
    float b = VOLTAGE_MIN - m * ADC_MIN_RAW;

    // 오프셋을 고려하여 보정된 ADC 값 계산
    float adjusted_adc_raw = (float)adc_raw + adc_offset;

    // 오프셋을 반영한 보정된 전압 계산
    return (m * adjusted_adc_raw + b);
}
