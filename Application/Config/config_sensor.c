
#include "config_sensor.h"

#include <string.h>

#include "app_file.h"
#include "app_version.h"
#include "crc.h"
#include "dev_io.h"
#include "user_heap.h"
config_sensor_t g_config_sensor;

const config_sensor_t g_config_sensor_default;

bool g_config_sensor_dirty_flag=false;

void limit_rs232(void)
{
  for (int i = 0; i < _countof(g_config_sensor.rs232); i++)
  {
    if (g_config_sensor.rs232[i].baud < 9600 || g_config_sensor.rs232[i].baud > 115200)
    {
      g_config_sensor.rs232[i].baud = 9600;
      g_config_sensor_dirty_flag = true;
    }

    if (g_config_sensor.rs232[i].port >= eRS232_MAX)
    {
      g_config_sensor.rs232[i].port = 0;
      g_config_sensor_dirty_flag = true;
    }

    if (g_config_sensor.rs232[i].parityIdx >= 2)
    {
      g_config_sensor.rs232[i].parityIdx = 0;
      g_config_sensor_dirty_flag = true;
    }
  }
}

void limit_rs485(void)
{
  for (int i = 0; i < _countof(g_config_sensor.rs485); i++)
  {
    if (g_config_sensor.rs485[i].baud < 9600 || g_config_sensor.rs485[i].baud > 115200)
    {
      g_config_sensor.rs485[i].baud = 9600;
      g_config_sensor_dirty_flag = true;
    }

    if (g_config_sensor.rs485[i].port >= eAPP_RS485_MAX)
    {
      g_config_sensor.rs485[i].port = 0;
      g_config_sensor_dirty_flag = true;
    }

    if (g_config_sensor.rs485[i].parityIdx >= 2)
    {
      g_config_sensor.rs485[i].parityIdx = 0;
      g_config_sensor_dirty_flag = true;
    }
  }
}

void limit_adc(void)
{
  for (int i = 0; i < _countof(g_config_sensor.adc); i++)
  {
    if (g_config_sensor.adc[i].channel > 17)
    {
      g_config_sensor.adc[i].channel = 0;
      g_config_sensor_dirty_flag = true;
    }
  }
}

void limit_hjwind(void)
{
  for (int i = 0; i < _countof(g_config_sensor.hjwind); i++)
  {
    if (g_config_sensor.hjwind[i].rs485_port > eAPP_RS485_MAX)
    {
      g_config_sensor.hjwind[i].rs485_port = 0;
      g_config_sensor_dirty_flag = true;
    }
  }
}

void limit_hjtemp(void)
{
  for (int i = 0; i < _countof(g_config_sensor.hjtemp); i++)
  {
    if (g_config_sensor.hjtemp[i].port > eAPP_RS485_MAX)
    {
      g_config_sensor.hjtemp[i].port = 0;
      g_config_sensor_dirty_flag = true;
    }
  }
}


void save_config_sensor(void)
{
  uint32_t crc;
  uint8_t temp;

  g_config_sensor.start = 0;
  crc = crc32_hw_with_padding(&g_config_sensor.start,
                              sizeof(config_sensor_t) - sizeof(g_config_sensor.header));

  g_config_sensor.header.magicNum = CONFIG_MAGIC;
  g_config_sensor.header.crc = crc;
  g_config_sensor.header.version = get_app_version();
  fram_write(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)&g_config_sensor, sizeof(g_config_sensor));
}

void load_config_sensor(void)
{


#if 0 
  //체크 
  crc_result = false;
  
  config_sensor_t *p_config = aws_malloc(sizeof(config_sensor_t));

  fram_read(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)p_config, sizeof(config_sensor_t));

  if (p_config->header.magicNum == CONFIG_MAGIC)
  {
    crc = crc32_hw_with_padding(&p_config->single,
                                sizeof(config_sensor_t) - sizeof(p_config->header));
    if (crc == p_config->header.crc)
    {
      memcpy(g_config_sensor, p_config, sizeof(config_sensor_t));
      crc_result = true;
    }
  }

  if (crc_result == false)
  {

  }
  aws_free(p_config);
#endif
  fram_read(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)&g_config_sensor, sizeof(g_config_sensor));

  limit_adc();
  limit_rs232();
  limit_rs485();
  limit_hjwind();
  limit_hjtemp();

  if (g_config_sensor_dirty_flag)
  {
    save_config_sensor();
  }
}

config_sensor_t *get_config_sensor(void)
{
  // 필요시 적절한 조치 처리
  return &g_config_sensor;
}

//task 실행 안되게 하고 업데이트 후 장비 재시작
void config_sensor_reset(void)
{
  memset(&g_config_sensor,0,sizeof(g_config_sensor));


}




#define PATH_CONFIG_SENSOR_BIN "0:config_sensor.bin"
void backup_config_sensor(void)
{
  FRESULT f_ret;

  f_ret = write_file(PATH_CONFIG_SENSOR_BIN, (uint8_t *)&g_config_sensor, sizeof(g_config_sensor), 0);
  if (f_ret == FR_OK)
  {
    debug_printf("0:config_sensor.bin 저장되었습니다.\r\n");
  }
}

void restore_config_sensor(void)
{
  config_sensor_t *p_config;
  bool crc_result= false;
  uint32_t crc;
  FRESULT f_ret;
  
  
  p_config = (config_sensor_t*)aws_malloc(sizeof(config_sensor_t));

  if(p_config)
  {
    f_ret = read_file(PATH_CONFIG_SENSOR_BIN,(uint8_t*)p_config,sizeof(config_sensor_t),0);
    
    if(f_ret != FR_OK)
    {
      debug_printf("파일 읽기 오류  %d\r\n",f_ret);
      aws_free(p_config);
      return ;
    }
      if (p_config->header.magicNum == CONFIG_MAGIC)
      {
        crc = crc32_hw_with_padding(&p_config->start, sizeof(config_sensor_t) - sizeof(p_config->header));
        if (crc == p_config->header.crc)
        {
          memcpy(&g_config_sensor, p_config, sizeof(config_sensor_t));
          crc_result = true;
          debug_printf("0:config_sensor.bin 복구되었습니다.\r\n");
        }
      }
    
      if (crc_result == false)
      {
        debug_printf("체크섬 오류\r\n");
      }
 
 

    aws_free(p_config);
  }
}