
#include "config_sensor.h"

#include <string.h>

#include "app_file.h"
#include "app_version.h"
#include "drv_crc.h"
#include "dev_io.h"
#include "user_heap.h"

const config_sensor_t g_sensor_att_default =
    {
        .temp.hj = {.modbus_id = 1,
                   .ofset = 0,
                   .physical_layer = ePHYSICAL_RS485,
                   .rs485_port = eAPP_RS485_RS232_B},
        .humi.hj= {.modbus_id = 1,
                   .ofset = 0,
                   .physical_layer = ePHYSICAL_RS485,
                   .rs485_port = eAPP_RS485_RS232_B}
        };


config_sensor_t g_config_sensor;
const config_sensor_t g_config_sensor_default;

bool g_config_sensor_dirty_flag=false;



void limit_temp(void)
{
    if (g_config_sensor.temp.adc.single_channel >= 18)
    {
      g_config_sensor.temp.adc.single_channel = 0;
      g_config_sensor_dirty_flag = true;
    }

    if (g_config_sensor.temp.adc.diff_channel >= 8)
    {
      g_config_sensor.temp.adc.diff_channel = 0;
      g_config_sensor_dirty_flag = true;
    }

    if (g_config_sensor.temp.adc.mode > ADC_FG_MODE_DIFF)
    {
      g_config_sensor.temp.adc.mode = ADC_CFG_MODE_SE;
      g_config_sensor_dirty_flag = true;
    }
  

  if (g_config_sensor.temp.hj.physical_layer > ePHYSICAL_RS485)
  {
    g_config_sensor.temp.hj.physical_layer = ePHYSICAL_RS485;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.temp.hj.physical_layer == ePHYSICAL_RS485)
  {
    if (g_config_sensor.temp.hj.rs485_port > eAPP_RS485_MAX)
    {
      g_config_sensor.temp.hj.rs485_port = eAPP_RS485_RS232_B;
      g_config_sensor_dirty_flag = true;
    }
  }

  if (g_config_sensor.temp.hj.physical_layer == ePHYSICAL_RS232)
  {
    if (g_config_sensor.temp.hj.rs232_port > eRS232_MAX)
    {
      g_config_sensor.temp.hj.rs232_port = eRS232_RS485_B;
      g_config_sensor_dirty_flag = true;
    }
  }

  if(g_config_sensor.temp.pt100.channel >2)
  {
    g_config_sensor.temp.pt100.channel = 0;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_wind_speed(void)
{
  if (g_config_sensor.wind_speed.hj.rs485_port > eAPP_RS485_MAX)
  {
    g_config_sensor.wind_speed.hj.rs485_port = eAPP_RS485_C;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.wind_speed.hj_modbus.rs485_port > eAPP_RS485_MAX)
  {
    g_config_sensor.wind_speed.hj_modbus.rs485_port = eAPP_RS485_C;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.wind_speed.rmyoung_05103v.frequency_channel > 1)
  {
    g_config_sensor.wind_speed.rmyoung_05103v.frequency_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.wind_speed.frequency.channel >= 2)
  {
    g_config_sensor.wind_speed.frequency.channel = 0;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_wind_direction(void)
{
  if (g_config_sensor.wind_direction.hj.rs485_port > eAPP_RS485_MAX)
  {
    g_config_sensor.wind_direction.hj.rs485_port = eAPP_RS485_C;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.wind_direction.hj_modbus.rs485_port > eAPP_RS485_MAX)
  {
    g_config_sensor.wind_direction.hj_modbus.rs485_port = eAPP_RS485_C;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.wind_direction.rmyoung_05103v.adc_channel > 15)
  {
    g_config_sensor.wind_direction.rmyoung_05103v.adc_channel = 0;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_rain(void)
{
  // rain.reed와 rain.hall은 eRAIN_MM_t enum 타입이므로 범위 체크
  if (g_config_sensor.rain.reed.mm > eRAIN_1MM)
  {
    g_config_sensor.rain.reed.mm = eRAIN_05MM;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.rain.hall.mm > eRAIN_1MM)
  {
    g_config_sensor.rain.hall.mm = eRAIN_05MM;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_barometer(void)
{
  if (g_config_sensor.baromater.jinsung_sjgp215.rs232_port > eRS232_MAX)
  {
    g_config_sensor.baromater.jinsung_sjgp215.rs232_port = eRS232_RS485_B;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.baromater.rmyoung_61402v_barometer.adc_channel > 15)
  {
    g_config_sensor.baromater.rmyoung_61402v_barometer.adc_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.baromater.adc.single_channel >= 18)
  {
    g_config_sensor.baromater.adc.single_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.baromater.adc.diff_channel >= 8)
  {
    g_config_sensor.baromater.adc.diff_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.baromater.adc.mode > ADC_FG_MODE_DIFF)
  {
    g_config_sensor.baromater.adc.mode = ADC_CFG_MODE_SE;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_rain_present(void)
{
  if (g_config_sensor.rain_present.off_delay_sec > 3600)
  {
    g_config_sensor.rain_present.off_delay_sec = 60;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_snow(void)
{
  if (g_config_sensor.snow.hj.physical_layer > ePHYSICAL_RS485)
  {
    g_config_sensor.snow.hj.physical_layer = ePHYSICAL_RS485;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.snow.hj.physical_layer == ePHYSICAL_RS485)
  {
    if (g_config_sensor.snow.hj.rs485_port > eAPP_RS485_MAX)
    {
      g_config_sensor.snow.hj.rs485_port = eAPP_RS485_RS232_B;
      g_config_sensor_dirty_flag = true;
    }
  }

  if (g_config_sensor.snow.hj.physical_layer == ePHYSICAL_RS232)
  {
    if (g_config_sensor.snow.hj.rs232_port > eRS232_MAX)
    {
      g_config_sensor.snow.hj.rs232_port = eRS232_RS485_B;
      g_config_sensor_dirty_flag = true;
    }
  }
}

void limit_humi(void)
{
  if (g_config_sensor.humi.hj.physical_layer > ePHYSICAL_RS485)
  {
    g_config_sensor.humi.hj.physical_layer = ePHYSICAL_RS485;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.humi.hj.physical_layer == ePHYSICAL_RS485)
  {
    if (g_config_sensor.humi.hj.rs485_port > eAPP_RS485_MAX)
    {
      g_config_sensor.humi.hj.rs485_port = eAPP_RS485_RS232_B;
      g_config_sensor_dirty_flag = true;
    }
  }

  if (g_config_sensor.humi.hj.physical_layer == ePHYSICAL_RS232)
  {
    if (g_config_sensor.humi.hj.rs232_port > eRS232_MAX)
    {
      g_config_sensor.humi.hj.rs232_port = eRS232_RS485_B;
      g_config_sensor_dirty_flag = true;
    }
  }

  if (g_config_sensor.humi.adc.single_channel >= 18)
  {
    g_config_sensor.humi.adc.single_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.humi.adc.diff_channel >= 8)
  {
    g_config_sensor.humi.adc.diff_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.humi.adc.mode > ADC_FG_MODE_DIFF)
  {
    g_config_sensor.humi.adc.mode = ADC_CFG_MODE_SE;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_solar_radiation(void)
{
  if (g_config_sensor.solar_radication.ott_smp3.rs485_port > eAPP_RS485_MAX)
  {
    g_config_sensor.solar_radication.ott_smp3.rs485_port = eAPP_RS485_C;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.solar_radication.ott_smp3.modbus_id == 0 ||
      g_config_sensor.solar_radication.ott_smp3.modbus_id > 247)
  {
    g_config_sensor.solar_radication.ott_smp3.modbus_id = 1;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.solar_radication.adc.single_channel >= 18)
  {
    g_config_sensor.solar_radication.adc.single_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.solar_radication.adc.diff_channel >= 8)
  {
    g_config_sensor.solar_radication.adc.diff_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.solar_radication.adc.mode > ADC_FG_MODE_DIFF)
  {
    g_config_sensor.solar_radication.adc.mode = ADC_CFG_MODE_SE;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_sunshine(void)
{
  if (g_config_sensor.sunshine.solar_duration_csd3.adc_channel > 15)
  {
    g_config_sensor.sunshine.solar_duration_csd3.adc_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.sunshine.adc.single_channel >= 18)
  {
    g_config_sensor.sunshine.adc.single_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.sunshine.adc.diff_channel >= 8)
  {
    g_config_sensor.sunshine.adc.diff_channel = 0;
    g_config_sensor_dirty_flag = true;
  }

  if (g_config_sensor.sunshine.adc.mode > ADC_FG_MODE_DIFF)
  {
    g_config_sensor.sunshine.adc.mode = ADC_CFG_MODE_SE;
    g_config_sensor_dirty_flag = true;
  }
}

void limit_soil_temp(void)
{
  for (int i = 0; i < _countof(g_config_sensor.soil_temp); i++)
  {
    if (g_config_sensor.soil_temp[i].adc.single_channel >= 18)
    {
      g_config_sensor.soil_temp[i].adc.single_channel = 0;
      g_config_sensor_dirty_flag = true;
    }

    if (g_config_sensor.soil_temp[i].adc.diff_channel >= 8)
    {
      g_config_sensor.soil_temp[i].adc.diff_channel = 0;
      g_config_sensor_dirty_flag = true;
    }

    if (g_config_sensor.soil_temp[i].adc.mode > ADC_FG_MODE_DIFF)
    {
      g_config_sensor.soil_temp[i].adc.mode = ADC_CFG_MODE_SE;
      g_config_sensor_dirty_flag = true;
    }
  }
}

void save_config_sensor(void)
{
  uint32_t crc;
  uint8_t *p_start;

  p_start = (uint8_t *)&g_config_sensor + sizeof(g_config_sensor.header);

  crc = drv_crc32_with_padding(p_start,sizeof(config_sensor_t) - sizeof(g_config_sensor.header));

  g_config_sensor.header.magicNum = CONFIG_MAGIC;
  g_config_sensor.header.crc = crc;
  g_config_sensor.header.version = get_app_version(0,0,0,0);
  drv_fram_write(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)&g_config_sensor, sizeof(g_config_sensor));
}

void load_config_sensor(void)
{


#if 0
  uint32_t crc;
  uint8_t *p_start;


  drv_fram_read(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)&g_config_sensor, sizeof(config_sensor_t));

  p_start = (uint8_t *)&g_config_sensor + sizeof(g_config_sensor.header);

  if (g_config_sensor.header.magicNum == CONFIG_MAGIC)
  {
    crc = drv_crc32_with_padding(p_start, sizeof(config_sensor_t) - sizeof(g_config_sensor.header));
    if (crc != g_config_sensor.header.crc)
    {
      memset(&g_config_sensor, 0xff, sizeof(g_config_sensor));
    }
  }

  

#else
  drv_fram_read(CONFIG_SENSOR_START_ADDRESS, (uint8_t *)&g_config_sensor, sizeof(g_config_sensor));
#endif

  limit_temp();
  limit_wind_speed();
  limit_wind_direction();
  limit_rain();
  limit_barometer();
  limit_rain_present();
  limit_snow();
  limit_humi();
  limit_solar_radiation();
  limit_sunshine();
  limit_soil_temp();

  if (g_config_sensor_dirty_flag)
  {
    save_config_sensor();
    backup_config_sensor();
  }
  
    io_printf("sizeof(config_sensor_t):%d\r\n",sizeof(config_sensor_t));
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




#define PATH_CONFIG_SENSOR_BIN "0:back_up/config_sensor.bin"
void backup_config_sensor(void)
{
  FRESULT f_ret;

   make_path(PATH_CONFIG_SENSOR_BIN);
      
  f_ret = write_file(PATH_CONFIG_SENSOR_BIN, (uint8_t *)&g_config_sensor, sizeof(g_config_sensor), 0);
  if (f_ret == FR_OK)
  {
    io_printf("%s에 저장되었습니다\r\n",PATH_CONFIG_SENSOR_BIN);
  }
}

void restore_config_sensor(void)
{
  bool crc_result = false;
  uint8_t *p_start;
  uint32_t crc;
  config_sensor_t *p_config;
  FRESULT f_ret;

  
  p_config = (config_sensor_t *)user_malloc(sizeof(config_sensor_t));

  p_start = (uint8_t *)p_config + sizeof(p_config->header);

  if (p_config)
  {
    f_ret = read_file(PATH_CONFIG_SENSOR_BIN, (uint8_t *)p_config, sizeof(config_sensor_t), 0);

    if (f_ret != FR_OK)
    {
      io_printf("파일 읽기 오류  %d\r\n", f_ret);
      user_free(p_config);
      return;
    }
    if (p_config->header.magicNum == CONFIG_MAGIC)
    {
      crc = drv_crc32_with_padding(p_start, sizeof(config_sensor_t) - sizeof(p_config->header));
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