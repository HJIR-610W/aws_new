


#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

#include <stdbool.h>
#include "utile.h"
#include "driver_fram.h"
#include "app_fram.h"

#define ADC_CALI_START_ADDRESS 0x00000000
#define CONFIG_START_ADDRESS   0x00002800 
#define WRITE_CFG_CALI(x) fram_write((uint32_t)OFFSET_OF_STRUCT(adc_cali_config_t, x),(uint8_t *)&g_adc_cali_config.x,sizeof(g_adc_cali_config.x));
#define WRITE_CFG(x) fram_write(CONFIG_START_ADDRESS +(uint32_t)OFFSET_OF_STRUCT(config_t, x),(uint8_t *)&config.x,sizeof(config.x));

typedef struct adc_calibraion_s
{
  int32_t offset;
  int32_t fullset;
  int32_t offset_input;
  int32_t fullset_input;
  float gain;
}adc_calibraion_t;

typedef struct adc_cali_s
{
  adc_calibraion_t single[32];
  adc_calibraion_t diff[8];
}adc_cali_config_t;





#define USER_ENABLE  1
#define USER_DISABLE 0



typedef enum adcChType_e
{
  eSINGLE_ADC,
  eDIFF_ADC
}eADC_CH_TYPE_t;


typedef struct 
{
  uint8_t mode; //0 single, 1 diff
  uint8_t channel;
  uint32_t highScale;
  uint32_t lowScale;
}adc_cfg_t;

typedef struct rs232_s
{
  uint32_t buad;
  uint8_t  port;
  uint8_t  parity;
}rs232_cfg_t;

typedef struct sensor_s
{
  uint8_t type;
  uint8_t adc_cfg_num;
  uint8_t rs232_cfg_num;  
}sensor_t;


#define TEMP_TYPE_UNUSED 0
#define TEMP_TYPE_ADC    1
#define TEMP_TYPE_RS485  2

typedef enum temp_type_e
{
  eSENSOR_TYPE_UNUSED,
  eSENSOR_TYPE_ADC,
  eSENSOR_TYPE_RS485
}eTEMP_TYPE_t;

typedef struct temp_config_s
{
  char name[10];
  eTEMP_TYPE_t type;  
  adc_cfg_t adc;
  rs232_cfg_t rs232;
  rs232_cfg_t rs485;
}temp_config_t;

typedef enum windDirection_type_e
{
  eWIND_DIRECTION_TYPE_UNUSED,
  eWIND_DIRECTION_ADC,
  eWIND_DIRECTION_RS485
}eWIND_DIRECTION_TYPE_t;

typedef struct windDirection_config_s
{
  char name[10];
  eWIND_DIRECTION_TYPE_t type;   
  adc_cfg_t adc;
  rs232_cfg_t rs485;
}windDirection_config_t;

typedef struct windSpeed_config_s
{
  char name[10];
  eWIND_DIRECTION_TYPE_t type;   
  adc_cfg_t adc;
  rs232_cfg_t rs485;
}windSpeed_config_t;




typedef struct config_s
{
  uint16_t id;
  uint16_t password;
  uint8_t chgType;
  sensor_t sensor[50];
  uint16_t logCnt;
  uint8_t eth_subnet[4];
  uint8_t eth_gateway[4];
  uint8_t eth_ip[4];
  uint8_t eth_server_ip[4];
  uint16_t eth_server_port;
  uint8_t eth_protocol;
  uint8_t cdma_server_ip[4];
  uint16_t cdma_port;
  uint8_t cdma_protocol;
  uint8_t cdmaType;
  bool eth_use;
  bool cdma_use;
  bool direct_use;
  uint8_t direct_protocol;
  uint32_t direct_baud;
  uint8_t panelType;
  uint8_t vhf_id;
  uint8_t vhf_group;
  uint8_t vhf_host_id;
  uint8_t vhf_repeater_id;
  uint16_t vhf_ptt_delay;
}config_t;


typedef struct system_s
{
  uint8_t doorStatus;
  uint8_t eth_link_status;//0정상, 1 다운
  uint8_t eth_tx_cnt;
  uint8_t eth_rx_cnt;
  uint8_t cdma_link_status;
  uint8_t cdma_tx_cnt;
  uint8_t cdma_rx_cnt;
  int8_t cdma_rssi;
  char cdma_num[20];
  uint8_t direct_link_status;
  uint8_t direct_tx_cnt;
  uint8_t direct_rx_cnt;
  uint8_t vhf_tx_cnt;
  uint8_t vhf_rx_cnt;
  uint8_t charger_status;
}system_t;

void config_init(void);
void config_write_adcCalibraion(void);

extern config_t config;
extern system_t System;
extern temp_config_t g_temp_config;;
extern adc_cali_config_t g_adc_cali_config;



#endif