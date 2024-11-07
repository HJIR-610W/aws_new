

#ifndef DRIVER_ADC_H
#define DRIVER_ADC_H


#define ADC_ADS1220 0
#define ADC_STM32   1

typedef struct driver_adc_s
{
    const char *name;
    uint8_t err;
    osSemaphoreId mutex;
    uint32_t num;
    void *api;
    void *apiCfg;
}driver_adc_t;

void driver_adc_init(driver_adc_t *adc,uint32_t num);
void driver_adc_read(driver_adc_t *adc,uint32_t *val,uint32_t ch);

#endif