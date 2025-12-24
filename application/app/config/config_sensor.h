


#ifndef CONFIG_SENSOR_H
#define CONFIG_SENSOR_H

#include <stdbool.h>

#include "drv_fram.h"
#include "config_define.h"
#include "config_memory_map.h"
#include "drv_rs232.h"
#include "drv_rs485.h"
#include "util_memory.h"


#define SENSOR_ERR_CFG 2

#define ADC_CFG_MODE_SE 0
#define ADC_FG_MODE_DIFF 1

#define WRITE_CFG_SENSOR(x)                                                                    \
  drv_fram_write(CONFIG_SENSOR_START_ADDRESS + (uint32_t)OFFSET_OF_STRUCT(config_sensor_t, x), \
                 (uint8_t *)&g_config_sensor.x, sizeof(g_config_sensor.x));


//화진 기본 구성 
#define ADC_PRESSURE_RMYOUNG_61402V 0
#define ADC_SUNSHINE_CSD3            1
#define ADC_SOIL5CM                 2
#define ADC_SOIL10CM                3
#define ADC_SOIL20CM                4
#define ADC_SOIL30CM                5
#define ADC_SOIL50CM                6
#define ADC_SOIL1M                  7
#define ADC_SOIL1_5M               8
#define ADC_SOIL3M                  9
#define ADC_SOIL5M                 10


typedef struct
{
  uint8_t mode;  // 0 single, 1 diff
  uint8_t single_channel;
  uint8_t diff_channel;
  int32_t high_scale;
  int32_t low_scale;
  int32_t scale;    // 원본값에 몇배 곱해졌다의 의미 high_scale 100, low_scale 0이면
                    // 0~100으로 값이 나옴
                    //  scale 10이면 최종 값은 나누기 10해야함
  int32_t out_max_mv;  // 센서의 출력 전압 최고
  int32_t out_min_mv;  // 센서의 출력 전압 최저
} adc_config_t;





//화진 풍속 구형 타입 
typedef struct hj_wind_speed_s
{
  uint8_t rs485_port;
  int32_t offset;
  int32_t full;
} wind_speed_hj_pulse_config_t;

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
} temp_hj_config_t;

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
} humi_hj_config_t;



typedef struct hjwindDirection_s
{
  uint8_t rs485_port;
} wind_direction_hj_pulse_config_t;

// 화진 적설 232,485
typedef struct hjsnow_config_s
{
  ePHYSOCAL_LAYER_t physical_layer;
  uint8_t rs232_port;
  uint8_t rs485_port;
} snow_hj_config_t;

typedef struct ottSMP3_config_s
{
  uint8_t rs485_port;
  uint8_t modbus_id;
  uart_config_t uart_config;
} solar_r_ott_smp3_config_t;

typedef struct rain_present_config_s
{
  uint16_t off_delay_sec;
}rain_present_config_t;

typedef struct frequency_config_s
{
  float scale_factor;
  int channel;
} frequency_config_t;

typedef struct jinsung_sjgp215_config_s
{
  uint8_t rs232_port;
} barometer_jinsung_sjgp215_config_t;

typedef struct rmyoung_05103v_wind_direction_config_s
{
  uint8_t adc_channel;
} wind_direction_rmyoung_05103v_config_t;

typedef struct rmyoung_05103v_wind_speed_config_s
{
  uint8_t frequency_channel;
} wind_speed_rmyoung_05103v_config_t;

typedef struct rmyoung_61402v_barometer_config_s
{
  uint8_t adc_channel;
} barometer_rmyoung_61402v_config_t;


typedef struct solar_duration_csd3_s
{
  uint8_t adc_channel;
} solar_duration_csd3_t;




typedef struct wind_speed_hj_config_s
{
  uint8_t rs485_port;
  uint8_t modbus_id;
} wind_speed_hj_config_t;

typedef struct wind_direction_hj_config_s
{
  uint8_t rs485_port;
  uint8_t modbus_id;
} wind_direction_hj_config_t;

typedef struct temperature_pt100_s
{
  int channel;
}temperature_pt100_t;



#define RAIN_MM_LIST               \
  X(eRAIN_05MM, "0.5")\
  X(eRAIN_1MM, "1.0")

  typedef enum rain_mm_e{
#define X(code, name) code,
  RAIN_MM_LIST
#undef X
  RAIN_MM_COUNT
} eRAIN_MM_t;


typedef struct rainfall_reed_config_s
{
  eRAIN_MM_t mm;
}rainfall_reed_t;

typedef struct rainfall_hall_config_s
{
  eRAIN_MM_t mm;
}rainfall_hall_t;








typedef struct temperature_config_s
{
  temp_hj_config_t hj;
  temperature_pt100_t pt100;
  adc_config_t   adc; 
}temperature_config_t;

typedef struct windspeed_config_s
{
  wind_speed_hj_config_t hj_modbus;
  wind_speed_hj_pulse_config_t hj;//구형 타입 켈리브 필요한 타입
  wind_speed_rmyoung_05103v_config_t rmyoung_05103v;
  frequency_config_t frequency;
}wind_speed_config_t;



typedef struct windspeed_direction_s
{
  wind_direction_hj_pulse_config_t      hj;//구형 타입 켈리브 필요한 타입
  wind_direction_hj_config_t            hj_modbus;
  wind_direction_rmyoung_05103v_config_t rmyoung_05103v;
}wind_direction_config_t;

typedef struct rain_config_s
{
  rainfall_reed_t reed;
  rainfall_hall_t hall;
}rain_config_t;

typedef struct barometer_config_s
{
  barometer_jinsung_sjgp215_config_t jinsung_sjgp215;
  barometer_rmyoung_61402v_config_t rmyoung_61402v_barometer;
  adc_config_t adc;
}barometer_config_t;


typedef struct rain_present_s
{
  rain_present_config_t rain_present;
}rain_present_t;


typedef struct snow_config_s
{
  snow_hj_config_t hj;
}snow_config_t;


typedef struct huminity_config_s
{
  humi_hj_config_t hj;
  adc_config_t   adc; 
}huminity_config_t;

typedef struct solar_radication_config_s
{
  solar_r_ott_smp3_config_t ott_smp3;//일사
  adc_config_t   adc; 
}solar_radication_config_t;

typedef struct sunshine_config_s
{
  solar_duration_csd3_t solar_duration_csd3;//일조
  adc_config_t   adc; 
}sunshine_config_t;


typedef struct soil_temp_s
{
  adc_config_t   adc; 
}soil_temp_t;

#define CONFIG_SENSOR_VERSION 0x00000001
typedef struct sensor_configs_s
{
  config_header_t header;
  temperature_config_t temp;
  wind_speed_config_t wind_speed;
  wind_direction_config_t wind_direction;
  rain_config_t rain;
  barometer_config_t baromater;
  rain_present_config_t rain_present;
  snow_config_t snow;
  huminity_config_t humi;
  solar_radication_config_t solar_radication;
  sunshine_config_t sunshine;
  soil_temp_t soil_temp[9];
}config_sensor_t;

void save_config_sensor(void);
void load_config_sensor(void);
void config_sensor_reset(void);
void backup_config_sensor(void);
void restore_config_sensor(void);

extern config_sensor_t g_config_sensor;
config_sensor_t *get_config_sensor(void);

#endif