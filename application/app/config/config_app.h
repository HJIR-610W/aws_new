
#ifndef CONFIG_APP_H
#define CONFIG_APP_H

#include <stdbool.h>
#include <stdint.h>

#include "drv_fram.h"
#include "app_sensor.h"
#include "config_define.h"
#include "config_memory_map.h"
#include "util_memory.h"
#include "tcp_define.h"




#define WRITE_CFG(x)                                                             \
  drv_fram_write(CONFIG_START_ADDRESS + (uint32_t)OFFSET_OF_STRUCT(config_t, x), \
                 (uint8_t *)&config.x, sizeof(config.x));

#define WRITE_CFG_MEM(dataAdd, len)                                                               \
  drv_fram_write(CONFIG_START_ADDRESS + (uint32_t)OFFSET_S(&config, dataAdd), (uint8_t *)dataAdd, \
                 len);

typedef enum uart_baud_e
{
  eBAUD_1200,
  eBAUD_9600,
  eBAUD_19200,
  eBAUD_38400,
  eBAUD_57600,
  eBAUD_115200
} eUART_BAUD_t;

typedef enum eth_mode_e
{
  eETH_MODE_CLINET,
  eETH_MODE_SERVER
}eETH_MODE_t;

typedef enum charger_model_e
{
  eCHARGER_NONE,
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
  ePANEL_NOT_USED,
  ePANEL_AWS_STD,
  ePANEL_HJ_STD,
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

typedef enum telnet_mode_e
{
  eTELNET_SERVER,
  eTELNET_CLIENT
} eTELNET_MODE_t;

typedef enum 
{
  eLCD_OFF_10SEC,
  eLCD_OFF_600SEC,
  eLCD_OFF_ALWAYS_ON
} eLCD_OFF_TIME_t;

typedef struct config_s
{
  config_header_t header;
  uint16_t id;
  sensor_t sensor[SENSOR_LIST_MAX];
  uint16_t password;
  eCHARGER_MODEL_t charger_model;  // 설정 후 리셋 요구됨
  eAWS_PROTOCOL_t aws_protocol_type;
  eETH_MODE_t eth_mode;     // 설정 후 리셋 요구됨
  uint8_t eth_subnet[4];    // 설정 후 리셋 요구됨
  uint8_t eth_gateway[4];   // 설정 후 리셋 요구됨
  uint8_t eth_ip[4];        // 설정 후 리셋 요구됨
  uint8_t eth_mac[6];       // 설정 후 리셋 요구됨
  uint8_t eth_remote_server_ip[4];
  uint16_t eth_remote_server_port;
  uint16_t eth_local_port;   // 설정 후 리셋 요구됨
  uint8_t cdma_server_ip[4];
  uint16_t cdma_port;
  eCDMA_MODEL_t cdma_model;  // 설정 후 리셋 요구됨
  uint8_t eth_active;        // 설정 후 리셋 요구됨
  uint8_t cdma_active;       // 설정 후 리셋 요구됨
  uint8_t direct_active;     // 설정 후 리셋 요구됨
  eUART_BAUD_t direct_baud_index;  // 설정 후 리셋 요구됨
  ePANEL_MODEL_t panel_model; // 경우에 따라 리셋 요구됨 ㄴ
  uint8_t panel_snow_active;
  uint8_t panel_barometer_active;
  uint8_t vhf_id;
  uint8_t vhf_group;
  uint8_t vhf_host_id;
  uint8_t vhf_repeater_id;
  uint16_t vhf_ptt_delay;
  uint8_t com_encrypt_active;
  uint8_t cdma_vpn_active;
  uint8_t ac_active;
  eTELNET_MODE_t dev_telnet_mode;
  uint8_t dev_telnet_ip[4];
  uint16_t dev_telnet_port;
  eLCD_OFF_TIME_t lcd_off_time_index; 
}config_t;


typedef enum config_app_field_e
{
  eCONFIG_APP_SENSOR
} eCONFIG_APP_FIELD_t;



typedef struct system_s
{
  bool dc_error;
  bool battery_error;
  bool door_opened;
  bool sdcard_inserted;
  uint8_t ac_status;//00 110v,01 220v,11 ADC OFF
  float charger_solar1_voltage;
  float charger_solar2_voltage;
  float charger_solar1_currnet;
  float charger_solar2_currnet;
  float charger_battery1_voltage;
  float charger_battery2_voltage;
  float charger_load1_currnet;
  float charger_load2_currnet;
  float charger_load3_currnet;
  float battery_voltage;
}system_t;

void config_app_sensor_reset(void);
void config_app_reset(void);
void save_config_app(void);
void load_config_app(void);
void save_config_app_field(eCONFIG_APP_FIELD_t field);
void set_sensor_offset(eSENSOR_TYPE_t sensor,float offset);
void backup_config_app(void);
void restore_config_app(void);
config_t *get_config_app(void);
void make_comList(char *out, uint16_t outsize) ;

void set_config_app_password(uint16_t password);
void set_config_app_cdma_port(uint16_t port);
void set_config_app_cdma_ip(uint8_t ip[4]);

uint16_t get_lcd_off_time(void);

eUART_BAUD_t uart_baud_to_config_index(uint32_t baud);
uint32_t config_index_to_uart_baud(eUART_BAUD_t index) ;


extern config_t config;
extern system_t System;
extern sensor_t g_sensor_config_bk[SENSOR_LIST_MAX]; // config 센서의 복사본
system_t *get_system(void);
sensor_t *get_sensor_config_copy(void);
#endif