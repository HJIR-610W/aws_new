#include "config_app.h"

#include <string.h>

#include "app_file.h"
#include "app_rs232.h"
#include "app_rs485.h"
#include "app_sensor.h"
#include "app_version.h"
#include "config_app.h"
#include "config_sensor.h"
#include "dev_io.h"
#include "drv_crc.h"
#include "user_heap.h"

config_t config;
system_t System;

sensor_t g_sensor_config_bk[SENSOR_LIST_MAX]; // config 센서의 복사본



const config_t config_app_default = {.id = 0,
                                     .password = 7777,
                                     .charger_model = eCHARGER_LS,
                                     .aws_protocol_type = eAWS_PROTOCOL_KMA3,
                                     .eth_mode = eETH_MODE_SERVER,
                                     .eth_subnet = {255, 255, 255, 0},
                                     .eth_gateway = {192, 168, 1, 1},
                                     .eth_ip = {192, 168, 1, 180},
                                     .eth_remote_server_ip = {112, 221, 177, 172},
                                     .eth_remote_server_port = 6442,
                                     .eth_local_port = 9000,
                                     .eth_mac={0x00,0x80,0xE1,0x00,0x00,0x00},
                                     .cdma_server_ip = {112, 221, 177, 172},
                                     .cdma_port = 0,
                                     .cdma_model = eCDMA_NTLE9607,
                                     .eth_active = true,
                                     .cdma_active = true,
                                     .direct_active = false,
                                     .direct_baud = 19200,
                                     .panel_model = ePANEL_AWS_STD,
                                     .panel_snow_active = false,
                                     .panel_barometer_active = false,
                                     .vhf_id = 0,
                                     .vhf_group = 0,
                                     .vhf_host_id = 0,
                                     .vhf_repeater_id = 0,
                                     .vhf_ptt_delay = 10,
                                     .encrypt_active = false,
                                     .vpn_active = false,
                                     .ac_active = false,
                                     .dev_telnet_ip = {112, 221, 177, 172},
                                     .dev_telnet_port = 23001,
                                     .dev_telnet_mode = eTELNET_SERVER};

int32_t g_config_app_change_count = 0;

bool is_value_in_array(uint8_t target, const uint8_t *arr, size_t len)
{
  for (size_t i = 0; i < len; i++)
  {
    if (arr[i] == target)
      return true;
  }

  return false;
}

void check_config_app(void)
{
  void *p_config;
  g_config_app_change_count++;
  
  if (config.charger_model > eCHARGER_LS)
  {
    config.charger_model = config_app_default.charger_model;
  g_config_app_change_count++;
  }

  if (config.eth_mode > eETH_MODE_SERVER)
  {
    config.eth_mode = config_app_default.eth_mode;
  g_config_app_change_count++;
  }


  if (config.cdma_model > eCDMA_TX700)
  {
    config.cdma_model = config_app_default.cdma_model;
  g_config_app_change_count++;
  }

  if (config.aws_protocol_type > eAWS_PROTOCOL_KMA3)
  {
    config.aws_protocol_type = config_app_default.aws_protocol_type;
  g_config_app_change_count++;
  }



  if (config.panel_model > ePANEL_HANSUNG)
  {
    config.panel_model = config_app_default.panel_model;
  g_config_app_change_count++;
  }

  if (config.panel_snow_active > 1)
  {
    config.panel_snow_active = config_app_default.panel_snow_active;
  g_config_app_change_count++;
  }

  if ((int)config.panel_barometer_active > 1)
  {
    config.panel_barometer_active = config_app_default.panel_barometer_active;
  g_config_app_change_count++;
  }

  if (config.eth_active > 1)
  {
    config.eth_active = config_app_default.eth_active;
  g_config_app_change_count++;
  }

  if(config.direct_active >1)
  {
    config.direct_active = config_app_default.direct_active;
  g_config_app_change_count++;
  }

  if (config.cdma_active > 1)
  {
    config.cdma_active = config_app_default.cdma_active;
  g_config_app_change_count++;
  }

  if (config.direct_active && config.cdma_active)
  {
    config.direct_active = 0;
    config.cdma_active = 1;
  g_config_app_change_count++;
  }


  for (int i = 0; i < _countof(config.sensor); i++)
  {
    if (config.sensor[i].type > SENSOR_TYPE_MAX)
    {
      config.sensor[i].type = S_T_UNSUED;
    g_config_app_change_count++;
    }
  }

  if(!is_value_in_array(config.sensor[A3_WIND_SPEED].type,windSpeedList,_countof(windSpeedList)))
  {
    config.sensor[A3_WIND_SPEED].type = S_T_UNSUED;
  g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[A2_WIND_DIRECTION].type, windDirectionList,
                          _countof(windDirectionList)))
  {
    config.sensor[A2_WIND_DIRECTION].type = S_T_UNSUED;
  g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[A9_SNOW_DEPTH].type, snowList, _countof(snowList)))
  {
    config.sensor[A9_SNOW_DEPTH].type = S_T_UNSUED;
  g_config_app_change_count++;
  }

  if (config.sensor[A1_TEMPERATURE].type == S_T_TEMPERATURE_HJ)
  {
    p_config = get_sensor_config(&config.sensor[A1_TEMPERATURE]);

    if (p_config == NULL)
    {
      config.sensor[A1_TEMPERATURE].type = S_T_UNSUED;
    g_config_app_change_count++;
    }
    else
    {
      hjtemp_config_t *p_hj_temp = p_config;

      if(p_hj_temp->physical_layer > ePHYSICAL_RS485)
      {
        p_hj_temp->physical_layer = ePHYSICAL_RS232;
      g_config_app_change_count++;
      }

      switch (p_hj_temp->physical_layer)
      {
        case ePHYSICAL_RS485:
          if (p_hj_temp->rs485_port > eAPP_RS485_MAX)
          {
            p_hj_temp->rs485_port = eAPP_RS485_A;
          g_config_app_change_count++;
          }
            break;
        case ePHYSICAL_RS232:
          if (p_hj_temp->rs232_port > eRS232_MAX)
          {
            p_hj_temp->rs232_port = eRS232_RS485_A;
          g_config_app_change_count++;
          }
          break;
         default:
          break;
      }

    }
  }
  //화진 온습도 습도 범위 확인
  if (config.sensor[A10_RELATIVE_HUMIDITY].type == S_T_HUMINITY_HJ)
  {
    p_config = get_sensor_config(&config.sensor[A10_RELATIVE_HUMIDITY]);

    if (p_config == NULL)
    {
      config.sensor[A10_RELATIVE_HUMIDITY].type = S_T_UNSUED;
    g_config_app_change_count++;
    }
    else
    {
      hjtemp_config_t *p_hj_temp = p_config;

      if (p_hj_temp->physical_layer > ePHYSICAL_RS485)
      {
        p_hj_temp->physical_layer = ePHYSICAL_RS232;
      g_config_app_change_count++;
      }

      switch (p_hj_temp->physical_layer)
      {
        case ePHYSICAL_RS485:
          if (p_hj_temp->rs485_port > eAPP_RS485_MAX)
          {
            p_hj_temp->rs485_port = eAPP_RS485_A;
          g_config_app_change_count++;
          }
          break;
        case ePHYSICAL_RS232:
          if (p_hj_temp->rs232_port > eRS232_MAX)
          {
            p_hj_temp->rs232_port = eRS232_RS485_A;
          g_config_app_change_count++;
          }
          break;
        default:
          break;
      }
    }
  }

  if (config.sensor[A2_WIND_DIRECTION].type == S_T_WIND_DIRECTION_HJ_485)
  {
    p_config = get_sensor_config(&config.sensor[A2_WIND_DIRECTION]);

    if (p_config ==NULL)
    {
      config.sensor[A2_WIND_DIRECTION].type = S_T_UNSUED;
    g_config_app_change_count++;
    }
  }

  if (config.panel_snow_active > 1)
  {
    config.panel_snow_active = config_app_default.panel_snow_active;
  g_config_app_change_count++;
  }

  if (config.panel_barometer_active > 1)
  {
    config.panel_barometer_active = config_app_default.panel_barometer_active;
  g_config_app_change_count++;
  }

  if(config.vpn_active>1)
  {
    config.vpn_active = config_app_default.vpn_active;
  g_config_app_change_count++;
  }
}



void save_config_app(void)
{ 
  uint32_t crc;



  config.start = 0;
  crc = drv_crc32_with_padding( &config.start,sizeof(config_t)-sizeof(config.header));
  
  config.header.magicNum = CONFIG_MAGIC;
  config.header.crc = crc;
  config.header.version = get_app_version(0,0,0,0); 

  drv_fram_write(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config)); 
}

void load_config_app(void)
{

#if 0 // CRC 미사용(test 필요)
  crc_result = false;

  config_t *p_config = user_malloc(sizeof(config_t));

  drv_fram_read(CONFIG_START_ADDRESS, (uint8_t *)p_config, sizeof(config_t));

  if (p_config->header.magicNum == CONFIG_MAGIC)
  {
    crc = crc32_hw_with_padding(&p_config->single, sizeof(config_t) - sizeof(p_config->header));
    if (crc == p_config->header.crc)
    {
      memcpy(config, p_config, sizeof(config_t));
      crc_result = true;
    }
  }

  if (crc_result == false)
  {
  }
  user_free(p_config);
#endif
  drv_fram_read(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config));

  check_config_app();

  if (g_config_app_change_count)
  {
    save_config_app();
  }

  // 프로그램 실행 중 설정값 변경되어도 영향 없도록 측정 Task는 설정값 복사본으로 동작
  memcpy(g_sensor_config_bk, config.sensor, sizeof(g_sensor_config_bk));
}




config_t *get_config_app(void)
{ 
  //필요시 적절한 조치 처리 
  return &config; 
}

sensor_t *get_sensor_config_copy(void)
{
  return g_sensor_config_bk;
}

void set_config_app_password(uint16_t password)
{
  config.password = password;
  WRITE_CFG(password);
}

void set_config_app_cdma_port(uint16_t port)
{
  config.cdma_port = port;
  WRITE_CFG(cdma_port);
}

void set_config_app_cdma_ip(uint8_t ip[4])
{
  config.cdma_server_ip[0] = ip[0];
  config.cdma_server_ip[1] = ip[1];
  config.cdma_server_ip[2] = ip[2];
  config.cdma_server_ip[3] = ip[3];

  WRITE_CFG(cdma_server_ip);
}


/**
 * @brief config_app 값을 공장초기화값으로 설정
 * 저장되지는 않음 
 */
 
void config_app_reset(void)
{
  config = config_app_default;

  memset(config.sensor, 0, sizeof(config.sensor));


}



void save_config_app_field(eCONFIG_APP_FIELD_t field)
{
  int32_t member_size;
  uint32_t offset;

  switch (field)
  {
    case eCONFIG_APP_SENSOR:
      member_size = MEMBER_SIZE(config_t,sensor);
      offset = OFFSET_OF_STRUCT(config_t,sensor);
      drv_fram_write(CONFIG_START_ADDRESS + (uint32_t)offset, (uint8_t *)config.sensor,
                     member_size);
      break;

    default:
      break;
  }
}

void make_comList(char *out, uint16_t outsize)
{
  int32_t len = 0;
  if (config.eth_active)
  {
    len = snprintf(&out[len], outsize - len, "[ETH]");
  }
  if (config.cdma_active)
  {
    len += snprintf(&out[len], outsize - len, "[CDMA]");
  }
  if (config.direct_active)
  {
    len += snprintf(&out[len], outsize - len, "[DIRECT]");
  }

  if (len == 0)
  {
    snprintf(&out[len], outsize - len, "Disabled");
  }
}

#define PATH_CONFIG_APP_BIN "0:config_app.bin"
void backup_config_app(void)
{
  FRESULT f_ret;

  f_ret = write_file(PATH_CONFIG_APP_BIN,(uint8_t *)&config,sizeof(config),0);
  if(f_ret == FR_OK)
  {
    io_printf("0:config_app.bin has been saved.\r\n");
  }
}

void restore_config_app(void)
{
  config_t *p_config;
  bool crc_result= false;
  uint32_t crc;
  FRESULT f_ret;
  p_config = user_malloc(sizeof(config_t));

  if(p_config)
  {
    f_ret = read_file(PATH_CONFIG_APP_BIN,(uint8_t *)p_config,sizeof(config_t),0);
    
    if(f_ret != FR_OK)
    {
      io_printf("File read error  %d\r\n", f_ret);
      user_free(p_config);
      return ;
    }
      if (p_config->header.magicNum == CONFIG_MAGIC)
      {
        crc = drv_crc32_with_padding(&p_config->start, sizeof(config_t) - sizeof(p_config->header));
        if (crc == p_config->header.crc)
        {
          memcpy(&config, p_config, sizeof(config_t));
          crc_result = true;
          io_printf("0:config_app.bin has been restored.\r\n");
        }
      }
    
      if (crc_result == false)
      {
        io_printf("Checksum error\r\n");
      }
 
 

    user_free(p_config);
  }
}

system_t *get_system(void)
{
  return &System;
}