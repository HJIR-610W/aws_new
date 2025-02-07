
#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

#include "app_fram.h"
#include "app_sensor.h"
#include "utile.h"
#include "driver_fram.h"

#define SENSOR_COUNT_MAX  SENSOR_LIST_MAX

#define ADC_CALI_START_ADDRESS 0x00000000 //0x00000000~0x000003FF 1024KB
#define S_CONFIG_START_ADDRESS 0x00000400 //0x00000400~0x00000FFF 1024KB
#define CONFIG_START_ADDRESS   0x00001000 //0x00001000~



#define WRITE_CFG_CALI(x) fram_write((uint32_t)OFFSET_OF_STRUCT(adc_cali_config_t, x),(uint8_t *)&g_adc_cali_config.x,sizeof(g_adc_cali_config.x));
#define WRITE_CFG(x) fram_write(CONFIG_START_ADDRESS +(uint32_t)OFFSET_OF_STRUCT(config_t, x),(uint8_t *)&config.x,sizeof(config.x));


#define WRITE_S_CFG(x) fram_write(S_CONFIG_START_ADDRESS +(uint32_t)OFFSET_OF_STRUCT(config_manager_t, x),(uint8_t *)&s_config.x,sizeof(s_config.x));

#define WRITE_CFG_MEM(dataAdd,len) fram_write(CONFIG_START_ADDRESS +(uint32_t)OFFSET_S(&config, dataAdd),(uint8_t *)dataAdd,len);


typedef struct adc_calibraion_s
{
  int32_t offset;       //   0v 입력 시 ADC값
  int32_t fullset;      // refv 입력 시 ADC 값
  int32_t offset_input; //   0mv
  int32_t fullset_input;//5000mv 예)5v ref일 때
  float gain;
}adc_calibraion_t;

typedef struct adc_cali_s
{
  adc_calibraion_t single[32];
  adc_calibraion_t diff[8];
}adc_cali_config_t;

#define ETH_MODE_CLIENT 0
#define ETH_MODE_SERVER 1
typedef struct config_s
{
  uint16_t id;
  uint16_t password;
  uint8_t chgType;
  uint16_t logCnt;
  uint8_t eth_mode;
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
  sensor_t sensor[SENSOR_COUNT_MAX];
  
  bool encryptUse;
  uint8_t netMode;
}config_t;


#define STATUS_IDLE -1
#define LINK_UP 0
#define LINK_DOWN 1
typedef struct system_s
{
  int8_t doorStatus;
  int8_t eth_link_status;//0정상, 1 다운
  int8_t eth_tx_cnt;
  int8_t eth_rx_cnt;
  int8_t cdma_link_status;
  uint8_t cdma_tx_cnt;
  uint8_t cdma_rx_cnt;
  int8_t cdma_rssi;
  char cdma_num[20];
  int8_t direct_link_status;
  int8_t direct_tx_cnt;
  int8_t direct_rx_cnt;
  int8_t vhf_tx_cnt;
  int8_t vhf_rx_cnt;
  int8_t charger_status;
  
  uint8_t TcpCntStat;
  uint8_t ModemRcvLevel;
  uint8_t netRun;

  
  
}system_t;

void config_init(void);
void config_write_adcCalibraion(void);
void write_s_config(void);
void write_config(void);


void update_cnt(uint8_t *cnt);

extern config_t config;
extern system_t System;
extern adc_cali_config_t g_adc_cali_config;



#endif