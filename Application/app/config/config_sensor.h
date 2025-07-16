


#ifndef CONFIG_SENSOR_H
#define CONFIG_SENSOR_H

#include <stdbool.h>

#include "drv_fram.h"
#include "config_define.h"
#include "drv_rs232.h"
#include "app_rs232.h"
#include "app_rs485.h"
#include "config_memory_map.h"
#include "util_memory.h"


#define SENSOR_ERR_CFG 2

#define ADC_CFG_MODE_SE 0
#define ADC_FG_MODE_DIFF 1


typedef struct
{
  uint8_t mode;  // 0 single, 1 diff
  uint8_t single_channel;
  uint8_t diff_channel;
  int32_t highScale;
  int32_t lowScale;
  int32_t scale;    // 원본값에 몇배 곱해졌다의 의미 highScale 100, lowScale 0이면
                    // 0~100으로 값이 나옴
                    //  scale 10이면 최종 값은 나누기 10해야함
  int32_t outMaxV;  // 센서의 출력 전압 최고
  int32_t outMinV;  // 센서의 출력 전압 최저
  //float voltage_offset; // 센서값 = (전압 + 전압_오프셋)*gain + offset
  //float gain;
  //float offset;  사용자 직관성 위해 현재 미사용
} adc_config_t;





//화진 풍속
typedef struct hj_wind_speed_s
{
  uint8_t rs485_port;
  int32_t offset;
  int32_t full;
} hjwindspeed_config_t;

typedef enum physical_layer_e
{
  ePHYSICAL_RS232,
  ePHYSICAL_RS485
}ePHYSOCAL_LAYER_t;

// 화진 온도 센서 232만 사용
typedef struct hjtemp_s
{
  ePHYSOCAL_LAYER_t physical_layer;
  uint8_t rs485_port;
  uint8_t rs232_port;

  union 
  {
    int32_t i_data;
    float f_data;
  }ofset;
  uint8_t modbus_id;
} hjtemp_config_t;

// 화진 습도 센서 232만 사용
typedef struct hjhumi_s
{
  ePHYSOCAL_LAYER_t physical_layer;
  uint8_t rs485_port;
  uint8_t rs232_port;
  union 
  {
    int32_t i_data;
    float f_data;
  }ofset;
  uint8_t modbus_id;
} hjhumi_config_t;



typedef struct hjwindDirection_s
{
  uint8_t rs485_port;
} hjwindDirection_config_t;

// 화진 적설 232,485
typedef struct hjsnow_config_s
{
  ePHYSOCAL_LAYER_t physical_layer;
  uint8_t port;
} hjsnow_config_t;

typedef struct ottSMP3_config_s
{
  uint8_t port;
  uint8_t modbus_id;
} ott_smp3_config_t;

typedef struct rain_present_config_s
{
  uint8_t delay;
}rain_present_config_t;

typedef struct frequency_config_s
{
  float scale_factor;
  int channel;
} frequency_config_t;

typedef struct config_manage_s
{
  config_header_t header;
  uint8_t start;
  uint8_t adc_cnt;
  adc_config_t adc[50];
  hjtemp_config_t hjtemp;
  hjhumi_config_t hjhumi;
  hjwindspeed_config_t hjwind_speed;
  hjwindDirection_config_t hjwindDir;
  hjsnow_config_t hjsnow;
  ott_smp3_config_t ott_smp3;
  rain_present_config_t rain_present;
  frequency_config_t frequency;
} config_sensor_t;

#define WRITE_CFG_SENSOR(x)                                                                    \
  drv_fram_write(CONFIG_SENSOR_START_ADDRESS + (uint32_t)OFFSET_OF_STRUCT(config_sensor_t, x), \
                 (uint8_t *)&g_config_sensor.x, sizeof(g_config_sensor.x));

void save_config_sensor(void);
void load_config_sensor(void);
void config_sensor_reset(void);
void backup_config_sensor(void);
void restore_config_sensor(void);


config_sensor_t *get_config_sensor(void);

extern config_sensor_t g_config_sensor;

#endif