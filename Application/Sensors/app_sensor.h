#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <stdint.h>


typedef enum S_T_e
{
  S_T_UNSUED                =  0, 
  S_T_ADC                   =  1,
  S_T_TEMP_232              =  2,
  S_T_TEMP_485              =  3,
  S_T_MODBUS                =  4 ,
  S_T_HART                  =  5,
  S_T_FREQ_0                =  6,
  S_T_RAIN_REED_05MM        =  7,
  S_T_RAIN_REED_1MM         =  8,
  S_T_RAIN_HALL_05MM        =  9,
  S_T_RAIN_HALL_1MM         = 10,
  S_T_DI_0                  = 11,
  S_T_SNOW_HJ_485           = 12,
  S_T_RAIN_SERIAL_232       = 13,
  S_T_WIND_SPEED_485         =14,
  S_T_WIND_DIRECTION_485     =15,
  S_T_HUMI_HJ_485            =16,
  S_T_WIND_SPEED_MAX_VAL     =17,
  S_T_WIND_DIRECTION_MAX_VAL =18,
  S_T_PRESSURE_485           = 19,
  S_T_HUMI_RS485             = 20,
  S_T_RAIN_PRESENT_DI        = 21,
  S_T_SNOW_HJ_232            = 22,
}eSENSOR_MODEL_t;

typedef struct di_s
{
  uint8_t num; //DI 핀 번호
}di_config_t;

typedef struct rs232_s
{
  uint32_t baud;
  uint8_t port;
  uint8_t parityIdx;
}rs232_config_t;

typedef struct modbus_s
{
  uint8_t mode; //0 rtu 1 tcp 2 ascii
  rs232_config_t rs232;
}modbus_config_t;



typedef enum adcChType_e
{
  eSINGLE_ADC=0,
  eDIFF_ADC
}eADC_CH_TYPE_t;


typedef struct 
{
  uint8_t mode; //0 single, 1 diff
  uint8_t channel;
  int32_t highScale;
  int32_t lowScale;
}adc_config_t;


typedef struct sensor_s
{
  eSENSOR_MODEL_t type; 
  int16_t scale;
  uint8_t configCnt;
  uint8_t config[4][2];
}sensor_t;



typedef struct config_manage_s
{
  uint8_t adc_cnt;
  adc_config_t adc[50];
  uint8_t rs232_cnt;
  rs232_config_t rs232[10];
  uint8_t rs485_cnt;
  rs232_config_t rs485[10];
  uint8_t modbus_cnt;
  modbus_config_t modbus[10];
  uint8_t di_cnt;
  di_config_t di[10];
}config_manager_t;


adc_config_t * get_adc_config(sensor_t *sensor);
void set_adc_config(sensor_t *sensor,adc_config_t *adc);
void add_adc_sensor_config(sensor_t *sensor,adc_config_t *adc);
void add_sensor_config(sensor_t *sensor,void *config,uint8_t sensorType);
void * get_sensor_config(sensor_t *sensor,uint8_t sensorType);


extern config_manager_t s_config;

extern const char *sensorTypeList[23];

//지원되는 센서 목록 
extern const uint8_t temperatureList[4];
extern const uint8_t windDirectionList[3];
extern const uint8_t windSpeedList[3];
extern const uint8_t windDirectionInstantList[2];
extern const uint8_t windSpeedInstantList[2];
extern const uint8_t pressureList[3];
extern const uint8_t rainList[6];
extern const uint8_t snowList[4];
extern const uint8_t rainPresentList[2];
extern const uint8_t humiList[3];

#endif