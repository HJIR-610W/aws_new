


#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

#include "driver_fram.h"
#include "app_fram.h"
#include "app_sensor.h"
#include "utile.h"


#define ADC_CALI_START_ADDRESS 0x00000000
#define S_CONFIG_START_ADDRESS 0x00000400
#define CONFIG_START_ADDRESS   0x00001000 


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


typedef struct config_s
{
  uint16_t id;
  uint16_t password;
  uint8_t chgType;
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
  sensor_t sensor[50];
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
void config_write_s_config(void);
extern config_t config;
extern system_t System;
extern adc_cali_config_t g_adc_cali_config;



#endif