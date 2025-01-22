#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <stdint.h>


#define S_T_UNSUED        0
#define S_T_ADC           1
#define S_T_RS232         2
#define S_T_RS485         3
#define S_T_MODBUS        4 
#define S_T_HART          5
#define S_T_FREQ          6
#define S_T_RAIN_REED     7
#define S_T_RAIN_HALL     8
#define S_T_DI            9

#define TEMP_TYPE_UNUSED 0
#define TEMP_TYPE_ADC    1
#define TEMP_TYPE_RS485  2



typedef struct rs232_s
{
  uint8_t port;
  uint8_t baudIdx;
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
  uint32_t highScale;
  uint32_t lowScale;
}adc_config_t;


typedef struct sensor_s
{
  int16_t data;
  uint16_t type;//ADC
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
}config_manager_t;


adc_config_t * get_adc_config(sensor_t *sensor);
void add_adc_sensor_config(sensor_t *sensor,adc_config_t *adc);


extern config_manager_t s_config;
#endif