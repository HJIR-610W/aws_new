
#include "config_sensor.h"

#include <string.h>

#include "app_file.h"
#include "app_version.h"
#include "drv_crc.h"
#include "dev_io.h"
#include "user_heap.h"
config_sensor_t g_config_sensor;

const config_sensor_t g_sensor_att_default =
    {
        .hjhumi = {.modbus_id = 1,
                   .ofset = 0,
                   .physical_layer = ePHYSICAL_RS485,
                   .rs485_port = eAPP_RS485_RS232_D},
        .hjtemp = {.modbus_id = 1,
                   .ofset = 0,
                   .physical_layer = ePHYSICAL_RS485,
                   .rs485_port = eAPP_RS485_RS232_D}
        };


const config_sensor_t g_config_sensor_default;

bool g_config_sensor_dirty_flag=false;





void limit_adc(void)
{
  for (int i = 0; i < _countof(g_config_sensor.adc); i++)
  {
    if (g_config_sensor.adc[i].single_channel > 17)
    {
      g_config_sensor.adc[i].single_channel = 0;
      g_config_sensor_dirty_flag = true;
    }
  }
}

void limit_hjwind(void)
{
  if (g_config_sensor.hjwind_speed.rs485_port > eAPP_RS485_MAX)
  {
    g_config_sensor.hjwind_speed.rs485_port = eAPP_RS485_A;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.hjwindDir.rs485_port > eAPP_RS485_MAX)
  {
    g_config_sensor.hjwindDir.rs485_port = eAPP_RS485_A;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_hjtemp(void)
{
  if (g_config_sensor.hjtemp.physical_layer > ePHYSICAL_RS485)
  {
    g_config_sensor.hjtemp.physical_layer = ePHYSICAL_RS485;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.hjtemp.physical_layer == ePHYSICAL_RS485)
  {
    if (g_config_sensor.hjtemp.rs485_port > eAPP_RS485_MAX)
    {
      g_config_sensor.hjtemp.rs485_port = eAPP_RS485_RS232_D;
      g_config_sensor_dirty_flag = true;
    }
  }
  if (g_config_sensor.hjtemp.physical_layer == ePHYSICAL_RS232)
  {
    if (g_config_sensor.hjtemp.rs232_port > eRS232_MAX)
    {
      g_config_sensor.hjtemp.rs232_port = eRS232_RS485_B;
      g_config_sensor_dirty_flag = true;
    }
  }
}
void limit_hjhumi(void)
{
  if (g_config_sensor.hjhumi.physical_layer > ePHYSICAL_RS485)
  {
    g_config_sensor.hjhumi.physical_layer = ePHYSICAL_RS485;
    g_config_sensor_dirty_flag = true;
  }
    if (g_config_sensor.hjhumi.physical_layer == ePHYSICAL_RS485)
    {
      if (g_config_sensor.hjhumi.rs485_port > eAPP_RS485_MAX)
      {
        g_config_sensor.hjhumi.rs485_port = eAPP_RS485_RS232_D;
        g_config_sensor_dirty_flag = true;
      }
    }
    else if (g_config_sensor.hjhumi.physical_layer == ePHYSICAL_RS232)
    {
      if (g_config_sensor.hjhumi.rs232_port > eRS232_MAX)
      {
        g_config_sensor.hjhumi.rs232_port = eRS232_RS485_B;
        g_config_sensor_dirty_flag = true;
      }
    }
  }

void save_config_sensor(void)
{
  uint32_t crc;


  g_config_sensor.start = 0;
  crc = drv_crc32_with_padding(&g_config_sensor.start,
                              sizeof(config_sensor_t) - sizeof(g_config_sensor.header));

  g_config_sensor.header.magicNum = CONFIG_MAGIC;
  g_config_sensor.header.crc = crc;
  g_config_sensor.header.version = get_app_version(0,0,0,0);
  drv_fram_write(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)&g_config_sensor, sizeof(g_config_sensor));
}

void load_config_sensor(void)
{


#if 0 
  //체크 
  crc_result = false;
  
  config_sensor_t *p_config = user_malloc(sizeof(config_sensor_t));

  drv_fram_read(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)p_config, sizeof(config_sensor_t));

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
  user_free(p_config);
#endif
  drv_fram_read(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)&g_config_sensor, sizeof(g_config_sensor));

  limit_adc();

  limit_hjwind();
  limit_hjhumi();
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
    io_printf("0:config_sensor.bin 저장되었습니다.\r\n");
  }
}

void restore_config_sensor(void)
{
  config_sensor_t *p_config;
  bool crc_result= false;
  uint32_t crc;
  FRESULT f_ret;
  
  
  p_config = (config_sensor_t*)user_malloc(sizeof(config_sensor_t));

  if(p_config)
  {
    f_ret = read_file(PATH_CONFIG_SENSOR_BIN,(uint8_t*)p_config,sizeof(config_sensor_t),0);
    
    if(f_ret != FR_OK)
    {
      io_printf("파일 읽기 오류  %d\r\n",f_ret);
      user_free(p_config);
      return ;
    }
      if (p_config->header.magicNum == CONFIG_MAGIC)
      {
        crc = drv_crc32_with_padding(&p_config->start, sizeof(config_sensor_t) - sizeof(p_config->header));
        if (crc == p_config->header.crc)
        {
          memcpy(&g_config_sensor, p_config, sizeof(config_sensor_t));
          crc_result = true;
          io_printf("0:config_sensor.bin 복구되었습니다.\r\n");
        }
      }
    
      if (crc_result == false)
      {
        io_printf("체크섬 오류\r\n");
      }
 
 

    user_free(p_config);
  }
}