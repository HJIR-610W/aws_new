#include "config_app.h"

#include <string.h>

#include "app_file.h"
#include "drv_rs232.h"
#include "drv_rs485.h"
#include "app_sensor.h"
#include "app_version.h"
#include "const_string.h"
#include "config_sensor.h"
#include "dev_io.h"
#include "drv_crc.h"
#include "user_heap.h"
#include "FreeRTOS.h"
#include "util_stdio.h"

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
                                     .eth_mac = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x00},
                                     .cdma_server_ip = {112, 221, 177, 172},
                                     .cdma_port = 0,
                                     .cdma_model = eCDMA_NTLE9607,
                                     .eth_active = false,
                                     .cdma_active = false,
                                     .direct_active = false,
                                     .direct_baud_index = eBAUD_19200,
                                     .panel_model = ePANEL_AWS_STD,
                                     .panel_snow_active = false,
                                     .panel_barometer_active = false,
                                     .vhf_id = 0,
                                     .vhf_group = 0,
                                     .vhf_host_id = 0,
                                     .vhf_repeater_id = 0,
                                     .vhf_ptt_delay = 10,
                                     .com_encrypt_active = false,
                                     .cdma_vpn_active = false,
                                     .ac_active = false,
                                     .dev_telnet_ip = {112, 221, 177, 172},
                                     .dev_telnet_port = 23001,
                                     .dev_telnet_mode = eTELNET_SERVER,
                                     .lcd_off_time_index = eLCD_OFF_ALWAYS_ON};

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


  //특별 처리 
  for (int i = 0; i < _countof(config.sensor); i++)
  {
    if (config.sensor[i].type > SENSOR_TYPE_MAX)
    {
      config.sensor[i].type = S_T_UNSUED;
      memset(&config.sensor[i],0,sizeof(sensor_t));
      g_config_app_change_count++;
    }
    else
    {
      if(config.sensor[i].configCnt >= SENSOR_CONFIG_TABLE_MAX)
      {
        memset(&config.sensor[i],0,sizeof(sensor_t));
      }
    }
  }




  if(!is_value_in_array(config.sensor[A1_TEMPERATURE].type,temperature_list,_countof(temperature_list)))
  {
    config.sensor[A1_TEMPERATURE].type = S_T_UNSUED;
   g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[A2_WIND_DIRECTION].type, wind_direction_list,_countof(wind_direction_list)))
  {
    config.sensor[A2_WIND_DIRECTION].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if(!is_value_in_array(config.sensor[A3_WIND_SPEED].type,wind_speed_list,_countof(wind_speed_list)))
  {
    config.sensor[A3_WIND_SPEED].type = S_T_UNSUED;
   g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[A6_RAINFALL_DOT5_1MM].type, rainfall_list, _countof(rainfall_list)))
  {
    config.sensor[A6_RAINFALL_DOT5_1MM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

    if (!is_value_in_array(config.sensor[A7_PRESSURE].type, pressure_list, _countof(pressure_list)))
  {
    config.sensor[A7_PRESSURE].type = S_T_UNSUED;
    g_config_app_change_count++;
  }
    if (!is_value_in_array(config.sensor[A8_RAIN_PRESENT].type, rain_present_list, _countof(rain_present_list)))
  {
    config.sensor[A8_RAIN_PRESENT].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

    if (!is_value_in_array(config.sensor[A9_SNOW_DEPTH].type, snow_list, _countof(snow_list)))
  {
    config.sensor[A9_SNOW_DEPTH].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[A10_RELATIVE_HUMIDITY].type, humi_list, _countof(humi_list)))
  {
    config.sensor[A10_RELATIVE_HUMIDITY].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B1_SOLAR_RADIATION].type, solar_radiation_list, _countof(solar_radiation_list)))
  {
    config.sensor[B1_SOLAR_RADIATION].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B2_SUNSHINE_DURATION].type, solar_duration_list, _countof(solar_duration_list)))
  {
    config.sensor[B2_SUNSHINE_DURATION].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B5_SOIL_TEMPERATURE_5CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B5_SOIL_TEMPERATURE_5CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B6_SOIL_TEMPERATURE_10CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B6_SOIL_TEMPERATURE_10CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }


  if (!is_value_in_array(config.sensor[B7_SOIL_TEMPERATURE_20CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B7_SOIL_TEMPERATURE_20CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B8_SOIL_TEMPERATURE_30CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B8_SOIL_TEMPERATURE_30CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B9_SOIL_TEMPERATURE_50CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B9_SOIL_TEMPERATURE_50CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B10_SOIL_TEMPERATURE_100CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B10_SOIL_TEMPERATURE_100CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B11_SOIL_TEMPERATURE_150CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B11_SOIL_TEMPERATURE_150CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B12_SOIL_TEMPERATURE_300CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B12_SOIL_TEMPERATURE_300CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (!is_value_in_array(config.sensor[B13_SOIL_TEMPERATURE_500CM].type, soil_temp_list, _countof(soil_temp_list)))
  {
    config.sensor[B13_SOIL_TEMPERATURE_500CM].type = S_T_UNSUED;
    g_config_app_change_count++;
  }

  if (config.cdma_model > CDMA_MODEL_COUNT)
  {
    config.cdma_model = config_app_default.cdma_model;
    g_config_app_change_count++;
  }


  if (config.cdma_active > 1)
  {
    config.cdma_active = 0;
    g_config_app_change_count++;
  }
   
  if (config.eth_active > 1)
  {
    config.eth_active = 0;
    g_config_app_change_count++;
  }

 if(config.direct_active >1)
  {
    config.direct_active = 0;
    g_config_app_change_count++;
  }


  if (config.direct_active && config.cdma_active)
  {
    config.direct_active = config_app_default.direct_active;
    config.cdma_active = config_app_default.cdma_active;
    g_config_app_change_count++;
  }

  if (config.eth_mode > eETH_MODE_SERVER)
  {
    config.eth_mode = config_app_default.eth_mode;
    g_config_app_change_count++;
  }

  if (config.direct_baud_index > eBAUD_115200)
  {
    config.direct_baud_index = config_app_default.direct_baud_index;
    g_config_app_change_count++;
  }

  if (config.panel_snow_active > 1)
  {
    config.panel_snow_active = 0;
    g_config_app_change_count++;
  }

  if (config.panel_barometer_active > 1)
  {
    config.panel_barometer_active = 0;
    g_config_app_change_count++;
  }

  if(config.com_encrypt_active > 1)
  {
    config.com_encrypt_active = 0;
    g_config_app_change_count++;
  }

  if(config.cdma_vpn_active>1)
  {
    config.cdma_vpn_active = 0;
  g_config_app_change_count++;
  }



  if (config.panel_model > PANEL_COUNT)
  {
    config.panel_model = config_app_default.panel_model;
  g_config_app_change_count++;
  }


  if (config.charger_model > eCHARGER_LS)
  {
      config.charger_model = config_app_default.charger_model;
      g_config_app_change_count++;
  }



  if (config.aws_protocol_type > eAWS_PROTOCOL_KMA3)
  {
    config.aws_protocol_type = config_app_default.aws_protocol_type;
  g_config_app_change_count++;
  }






 
  if (config.ac_active > 1)
  {
    config.ac_active = config_app_default.ac_active;
    g_config_app_change_count++;
  }

  if (config.dev_telnet_mode > eTELNET_CLIENT)
  {
    config.dev_telnet_mode = config_app_default.dev_telnet_mode;
    g_config_app_change_count++;
  }

    if (config.lcd_off_time_index > _countof(lcd_off_time_list_eng))
    {
      config.lcd_off_time_index = config_app_default.lcd_off_time_index;;
      g_config_app_change_count++;
    }






}



void save_config_app(void)
{ 
  uint32_t crc;
  uint8_t *p_start;


  p_start = (uint8_t *)&config + sizeof(config.header);

  crc = drv_crc32_with_padding( p_start,sizeof(config_t)-sizeof(config.header));
  
  config.header.magicNum = CONFIG_MAGIC;
  config.header.crc = crc;
  config.header.version = get_app_version(0,0,0,0); 

  drv_fram_write(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config)); 
}

//FRAM을 0xff로 지우고 펌웨어가 정상 동작하는지 테스트해본다.
void erase_fram(void)
{
  uint8_t buff[512];
  int quot;
  int rem;
  int i=0;

  quot = 8192/512;
  rem = 8192%512;

  memset(buff,0xff,sizeof(buff));
  for(int i = 0 ;i< quot; i++)
  {
    drv_fram_write(i * sizeof(buff), buff, sizeof(buff));
  }
  
  if(rem)
  {
    drv_fram_write(i * sizeof(buff), buff, rem);
  }
}


void check_unused_field(uint32_t start_address,uint32_t end_address)
{
  uint8_t buff[512];

  uint32_t unused_memory_size;
  int quot;
  int rem;
  int i=0;

  unused_memory_size = end_address - start_address + 1;

  quot = unused_memory_size / sizeof(buff);
  rem = unused_memory_size  % sizeof(buff);

  for( i = 0 ; i < quot; i++)
  {
    drv_fram_read(start_address +i*sizeof(buff), buff, sizeof(buff));
    for(int j = 0 ; j < sizeof(buff);j++)
    {
      if(buff[j])
      {
        memset(buff,0,sizeof(buff));
        drv_fram_write(start_address + i * sizeof(buff), buff, sizeof(buff));
        io_printf("start_address:%X 512\r\n", start_address + i * sizeof(buff));
        break;
      }
    }
  }

  if(rem)
  {
    drv_fram_read(start_address + i * sizeof(buff), buff,rem);
    for (int j = 0; j < rem; j++)
    {
      if (buff[j])
      {
        memset(buff, 0, sizeof(buff));
        drv_fram_write(start_address + i * sizeof(buff), buff, rem);
        io_printf("start_address:%X %d\r\n", start_address + i * sizeof(buff),rem);
        break;
      }
    }
  }
}

void load_config_app(void)
{

 
#if 0 // CRC 미사용(test 필요)

  uint8_t *p_start;
  uint32_t crc;

  drv_fram_read(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config_t));

  p_start = (uint8_t *)&config + sizeof(config.header);

  if (config.header.magicNum == CONFIG_MAGIC)
  {
    crc = drv_crc32_with_padding(p_start, sizeof(config_t) - sizeof(config.header));
    if (crc != config.header.crc)
    {
      config = config_app_default;
    }
  }

#else
      drv_fram_read(CONFIG_START_ADDRESS, (uint8_t *)&config, sizeof(config));
#endif
  check_config_app();

  if (g_config_app_change_count)
  {
    save_config_app();
  }

  // 프로그램 실행 중 설정값 변경되어도 영향 없도록 측정 Task는 설정값 복사본으로 동작
  memcpy(g_sensor_config_bk, config.sensor, sizeof(g_sensor_config_bk));

  check_unused_field(CONFIG_START_ADDRESS + sizeof(config_t), CONFIG_SENSOR_START_ADDRESS);
  
  io_printf("sizeof(config_t):%d\r\n",sizeof(config_t));
            
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
  uint8_t *p_buffer;
  config = config_app_default;

  memset(config.sensor, 0, sizeof(config.sensor));
  p_buffer = pvPortMalloc(CONFIG_MEMORY_SIZE);

  if(p_buffer)
  {
    memset(p_buffer, 0, CONFIG_MEMORY_SIZE);
    drv_fram_write(CONFIG_START_ADDRESS, p_buffer, CONFIG_MEMORY_SIZE);
    vPortFree(p_buffer);
  }
 
}

//센서 설정 부분만 초기화 한다.
void config_app_sensor_reset(void)
{
  uint8_t *p_sensor = (uint8_t *)config.sensor;

  memset(p_sensor,0,sizeof(config.sensor));
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
      drv_fram_write(CONFIG_START_ADDRESS + (uint32_t)offset, (uint8_t *)config.sensor,member_size);
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

#define PATH_CONFIG_APP_BIN "0:back_up/config_app.bin"
void backup_config_app(void)
{
  FRESULT f_ret;
    make_path(PATH_CONFIG_APP_BIN);
  f_ret = write_file(PATH_CONFIG_APP_BIN,(uint8_t *)&config,sizeof(config),0);
  if(f_ret == FR_OK)
  {
    io_printf("%s에 저장되었습니다\r\n",PATH_CONFIG_APP_BIN);
  }
}

void restore_config_app(void)
{
  uint8_t *p_start;

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
  
    p_start = (uint8_t*)p_config + sizeof(p_config->header);

    if (p_config->header.magicNum == CONFIG_MAGIC)
      {
        crc = drv_crc32_with_padding(p_start, sizeof(config_t) - sizeof(p_config->header));
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


uint16_t get_lcd_off_time(void)
{
  uint16_t lcd_off_time;

  switch (config.lcd_off_time_index)
  {
  case eLCD_OFF_10SEC:
    lcd_off_time = 10;
    break;
  case eLCD_OFF_600SEC:
    lcd_off_time = 600;
    break;
  default:
  lcd_off_time = 10;
  break;
  }

  return lcd_off_time;
}




eUART_BAUD_t uart_baud_to_config_index(uint32_t baud)
{
  eUART_BAUD_t baud_index;
   switch (baud)
  {
    case 1200:
      baud_index = eBAUD_1200;
      break;
    case 9600:
      baud_index = eBAUD_9600;
      break;
    case 19200:
      baud_index = eBAUD_19200;
      break;
    case 38400:
      baud_index = eBAUD_38400;
      break;
    case 57600:
      baud_index = eBAUD_57600;
      break;
    case 115200:
      baud_index = eBAUD_115200;
      break;
      default:
        baud_index = eBAUD_19200;
        break;
    }

    return baud_index;
}

uint32_t config_index_to_uart_baud(eUART_BAUD_t index)
{
  uint32_t baud;
  switch (index)
  {
  case eBAUD_1200:
    baud = 1200;
    break;
  case eBAUD_9600:
    baud =9600 ;
    break;
  case eBAUD_19200:
    baud = 19200;
    break;
  case eBAUD_38400:
    baud = 38400;
    break;
  case eBAUD_57600:
    baud = 57600;
    break;
  case eBAUD_115200:
    baud = 115200;
    break;
  default:
  baud = 19200;
  break;
  }

  return baud;
}


