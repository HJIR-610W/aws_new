
#ifndef CONFIG_APP_H
#define CONFIG_APP_H

#include <stdbool.h>
#include <stdint.h>

#include "app_fram.h"
#include "app_sensor.h"
#include "config_define.h"
#include "config_memory_map.h"
#include "utile.h"


#define WRITE_CFG(x)                                                                               \
  fram_write(CONFIG_START_ADDRESS + (uint32_t)OFFSET_OF_STRUCT(config_t, x), (uint8_t *)&config.x, \
             sizeof(config.x));

#define WRITE_CFG_MEM(dataAdd, len) \
  fram_write(CONFIG_START_ADDRESS + (uint32_t)OFFSET_S(&config, dataAdd), (uint8_t *)dataAdd, len);

typedef enum eth_mode_e
{
  eETH_MODE_CLINET,
  eETH_MODE_SERVER
}eETH_MODE_t;

typedef enum charger_model_e
{
  eCHARGER_SMART,
  eCHARGER_LS
} eCHARGER_MODEL_t;

typedef enum cdma_model_e
{
  eCDMA_NTLE9607,
  eCDMA_TX700
} eCDMA_MODEL_t;

typedef enum panel_model_e
{
  ePANEL_STD,
  ePANEL_MUJU,
  ePANEL_HANSUNG,
} ePANEL_MODEL_t;

typedef enum net_mode_e
{
  eNET_MODE_TCP_SERVER,
  eNET_MODE_TCP_CLIENT
} eNET_MODE_t;

typedef enum eth_protocol_e
{
  eETH_PROTOCOL_KMA2,
  eETH_PROTOCOL_KMA3
} eETH_PROTOCOL_t;

typedef enum aws_protocol_e
{
  eAWS_PROTOCOL_KMA2,
  eAWS_PROTOCOL_KMA3
} eAWS_PROTOCOL_t;

typedef struct config_s
{
  config_header_t header;
  uint8_t start;  //  bool restart_required;
  uint16_t id;
  uint16_t password;
  eCHARGER_MODEL_t charger_model;  // 설정 후 리셋 요구됨
  eETH_MODE_t eth_mode;
  uint8_t eth_subnet[4];    // 설정 후 리셋 요구됨
  uint8_t eth_gateway[4];   // 설정 후 리셋 요구됨
  uint8_t eth_ip[4];        // 설정 후 리셋 요구됨
  uint8_t eth_server_ip[4];
  uint16_t eth_server_port;
  uint16_t eth_local_port;   // 설정 후 리셋 요구됨
  uint8_t cdma_server_ip[4];
  uint16_t cdma_port;
  eAWS_PROTOCOL_t aws_protocol_type;
  eCDMA_MODEL_t cdma_model;  // 설정 후 리셋 요구됨
  bool eth_use;              // 설정 후 리셋 요구됨
  bool cdma_use;             // 설정 후 리셋 요구됨
  bool direct_use;           // 설정 후 리셋 요구됨
  uint32_t direct_baud;      // 설정 후 리셋 요구됨
  ePANEL_MODEL_t panel_model;
  bool panel_snow_use;
  bool panel_barometer_use;
  uint8_t vhf_id;
  uint8_t vhf_group;
  uint8_t vhf_host_id;
  uint8_t vhf_repeater_id;
  uint16_t vhf_ptt_delay;
  bool encrypt_use;

  bool ac_use;
  uint16_t m_usRainDtOffDelay;//구 AWS
  sensor_t sensor[SENSOR_LIST_MAX];
}config_t;


typedef enum config_app_field_e
{
  eCONFIG_APP_SENSOR
} eCONFIG_APP_FIELD_t;

typedef enum link_status_e
{
  eLINK_IDLE=0,
  eLINK_UP=1,
  eLINK_DOWN=2
}eLINK_STATUS_t;

typedef struct system_s
{
  bool door_opened;
  bool dc_error;
  bool battery_error;
  uint8_t ac_status;//00 110v,01 220v,11 ADC OFF
  eLINK_STATUS_t cdma_link_status;
  int8_t cdma_rssi;
  char cdma_num[20];
  int8_t vhf_tx_cnt;
  int8_t vhf_rx_cnt;
  int8_t charger_status;
  float chg_solarV1;
  float chg_solarV2;
  float chg_solarC1;
  float chg_solarC2;
  float chg_batV1;
  float chg_batV2;
  float chg_loadC1;
  float chg_loadC2;
  float chg_loadC3;
}system_t;

void config_app_reset(void) ;
void save_config_app(void);
void load_config_app(void);
void save_config_app_field(eCONFIG_APP_FIELD_t field);
void set_sensor_offset(eSENSOR_LIST_t sensor,float offset);
void backup_config_app(void);
void restore_config_app(void);
config_t *get_config_app(void);
void make_comList(char *out, uint16_t outsize) ;

void set_config_app_password(uint16_t password);
void set_config_app_cdma_port(uint16_t port);
void set_config_app_cdma_ip(uint8_t ip[4]);

extern config_t config;
extern system_t System;

#endif