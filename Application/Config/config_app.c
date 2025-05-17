#include "config_app.h"
#include "config_sensor.h"
#include "app_rs232.h"
#include "app_rs485.h"
#include "app_sensor.h"
#include "app_version.h"
#include "config_app.h"
#include "crc.h"
#include "user_heap.h"

config_t config;
system_t System;

const config_t config_app_default = {.id = 0,
                                 .password = 7777,
                                 .charger_model = eCHARGER_SMART,
                                 .eth_mode = eETH_MODE_SERVER,
                                 .eth_subnet = {255, 255, 255, 0},
                                 .eth_gateway = {192, 168, 1, 1},
                                 .eth_ip = {192, 168, 1, 180},
                                 .eth_server_ip = {112, 221, 177, 172},
                                 .eth_server_port = 6442,
                                 .eth_protocol = eETH_PROTOCOL_KMA3,
                                 .cdma_server_ip = {192,168,1,1},
                                 .cdma_port = 0,
                                 .cdma_protocol = eETH_PROTOCOL_KMA3,
                                 .cdma_model = eCDMA_NTLE9607,
                                 .eth_use = false,
                                 .cdma_use = false,
                                 .direct_use = false,
                                 .direct_protocol = 0,
                                 .direct_baud = 19200,
                                 .panel_model = ePANEL_STD,
                                 .vhf_id = 0,
                                 .vhf_group = 0,
                                 .vhf_host_id = 0,
                                 .vhf_repeater_id = 0,
                                 .vhf_ptt_delay = 10,
                                 .encrypt_use = false,
                                 .network_mode = eNET_MODE_TCP_SERVER,
                                 .ac_use = 0};

bool g_config_app_dirty_flag = false;

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
  int check_cnt=0;
  void *p_config;
  if (config.charger_model > eCHARGER_LS)
  {
    config.charger_model = config_app_default.charger_model;
    g_config_app_dirty_flag = true;
  }

  if (config.eth_mode > eETH_MODE_SERVER)
  {
    config.eth_mode = config_app_default.eth_mode;
    g_config_app_dirty_flag = true;
  }

  if (config.eth_protocol > eETH_PROTOCOL_KMA3)
  {
    config.eth_protocol = config_app_default.eth_protocol;
    g_config_app_dirty_flag = true;
  }

  if (config.cdma_model > eCDMA_TX700)
  {
    config.cdma_model = config_app_default.cdma_model;
    g_config_app_dirty_flag = true;
  }

  if (config.cdma_protocol > eETH_PROTOCOL_KMA3)
  {
    config.cdma_protocol = config_app_default.cdma_protocol;
    g_config_app_dirty_flag = true;
  }

  if (config.network_mode > eNET_MODE_TCP_CLIENT)
  {
    config.network_mode = config_app_default.network_mode;
    g_config_app_dirty_flag = true;
  }

  if (config.panel_model > ePANEL_HANSUNG)
  {
    config.panel_model = config_app_default.panel_model;
    g_config_app_dirty_flag = true;
  }

  if (config.direct_use && config.cdma_use)
  {
    config.direct_use = 0;
    config.cdma_use = 1;
    g_config_app_dirty_flag = true;
  }

  for (int i = 0; i < _countof(config.sensor); i++)
  {
    if (config.sensor[i].type > SENSOR_MODEL_MAX)
    {
      config.sensor[i].type = S_T_UNSUED;
      g_config_app_dirty_flag = true;
    }

  }

  if(!is_value_in_array(config.sensor[A3_WIND_SPEED].type,windSpeedList,_countof(windSpeedList)))
  {
    config.sensor[A3_WIND_SPEED].type = S_T_UNSUED;
  }

  if (!is_value_in_array(config.sensor[A2_WIND_DIRECTION].type, windDirectionList,
                          _countof(windDirectionList)))
  {
    config.sensor[A2_WIND_DIRECTION].type = S_T_UNSUED;
  }

  if (!is_value_in_array(config.sensor[A9_SNOW_DEPTH].type, snowList, _countof(snowList)))
  {
    config.sensor[A9_SNOW_DEPTH].type = S_T_UNSUED;
  }

  if (config.sensor[A1_TEMPERATURE].type == S_T_TEMPERATURE_HJ)
  {
    p_config = get_sensor_config(&config.sensor[A1_TEMPERATURE]);

    if (p_config == NULL)
    {
      config.sensor[A1_TEMPERATURE].type = S_T_UNSUED;
      g_config_app_dirty_flag = true;
    }
    else
    {
      hjtemp_config_t *p_hj_temp = p_config;

      if(p_hj_temp->physical_layer > ePHYSICAL_RS485)
      {
        p_hj_temp->physical_layer = ePHYSICAL_RS232;
        g_config_app_dirty_flag = true;
      }

      switch (p_hj_temp->physical_layer)
      {
        case ePHYSICAL_RS485:
          if (p_hj_temp->port > eAPP_RS485_MAX)
          {
            p_hj_temp->port = eAPP_RS485_A;
            g_config_app_dirty_flag = true;
          }
            break;
        case ePHYSICAL_RS232:
          if (p_hj_temp->port > eRS232_MAX)
          {
            p_hj_temp->port = eRS232_1;
            g_config_app_dirty_flag = true;
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
      g_config_app_dirty_flag = true;
    }
    else
    {
      hjtemp_config_t *p_hj_temp = p_config;

      if (p_hj_temp->physical_layer > ePHYSICAL_RS485)
      {
        p_hj_temp->physical_layer = ePHYSICAL_RS232;
        g_config_app_dirty_flag = true;
      }

      switch (p_hj_temp->physical_layer)
      {
        case ePHYSICAL_RS485:
          if (p_hj_temp->port > eAPP_RS485_MAX)
          {
            p_hj_temp->port = eAPP_RS485_A;
            g_config_app_dirty_flag = true;
          }
          break;
        case ePHYSICAL_RS232:
          if (p_hj_temp->port > eRS232_MAX)
          {
            p_hj_temp->port = eRS232_1;
            g_config_app_dirty_flag = true;
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
    }
  }
}



void save_config_app(void)
{ 
  uint32_t crc;
  uint8_t temp;

  config.start = 0;
  crc = crc32_hw_with_padding( &config.start,sizeof(config_t)-sizeof(config.header));
  
  config.header.magicNum = CONFIG_MAGIC;
  config.header.crc = crc;
  config.header.version = get_app_version(); 

  fram_write(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config)); 
}

void load_config_app(void)
{

#if 0
  crc_result = false;

  config_t *p_config = aws_malloc(sizeof(config_t));

  fram_read(CONFIG_START_ADDRESS, (uint8_t *)p_config, sizeof(config_t));

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
  aws_free(p_config);
#endif
  fram_read(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config));

  check_config_app();

  if (g_config_app_dirty_flag)
  {
    save_config_app();
  }
}




config_t *get_config_app(void)
{ 
  //필요시 적절한 조치 처리 
  return &config; 
}

void set_sensor_offset(eSENSOR_LIST_t sensor,float offset)
{
  config.sensor[sensor].offset = offset;
  WRITE_CFG(sensor[sensor].offset);
}

//task 실행 안되게 하고 업데이트 후 장비 재시작
void config_app_reset(void)
{
  config = config_app_default;
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
      fram_write(CONFIG_START_ADDRESS + (uint32_t)offset, (uint8_t *)config.sensor, member_size);
      break;

    default:
      break;
  }
}

void make_comList(char *out, uint16_t outsize)
{
  int32_t len = 0;
  if (config.eth_use)
  {
    len = snprintf(&out[len], outsize - len, "[ETH]");
  }
  if (config.cdma_use)
  {
    len += snprintf(&out[len], outsize - len, "[CDMA]");
  }
  if (config.direct_use)
  {
    len += snprintf(&out[len], outsize - len, "[DIRECT]");
  }

  if (len == 0)
  {
    snprintf(&out[len], outsize - len, "미사용");
  }
}