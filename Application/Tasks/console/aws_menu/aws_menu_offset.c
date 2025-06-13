#include "config_app.h"
#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"
#include "config_sensor.h"
#include "cli_input.h"

#include "Sensors\temperature\temperature.h"
#include "Sensors\barometer\barometer.h"
#include "driver_adc.h"
#include "app_adc.h"
#include "console_utile.h"


extern driver_t *get_sensor_driver(eSENSOR_LIST_t sensor);
extern int32_t get_driverNum(eSENSOR_MODEL_t type) ;


static uint8_t
    s_offset_sensor_index[SENSOR_LIST_MAX];

void inline_print_offset_sensor(uint8_t cnt,eSENSOR_LIST_t sensor)
{
  sensor_t *p_sensor;
  p_sensor = &get_config_app()->sensor[sensor];
  s_offset_sensor_index[cnt] = sensor;
  io_printf("%2d.%-14s 오프셋:%9.3f \r\n", cnt, sensor_name_list[sensor], p_sensor->offset);
  
}
int32_t print_offset_sensor(void)
{
  int32_t cnt = 0;
  io_printf("\r\n");


  inline_print_offset_sensor(cnt++, A1_TEMPERATURE);
  inline_print_offset_sensor(cnt++, A10_RELATIVE_HUMIDITY);
  inline_print_offset_sensor(cnt++, A7_PRESSURE);
  inline_print_offset_sensor(cnt++, B1_SOLAR_RADIATION);
  inline_print_offset_sensor(cnt++, B2_SUNSHINE_DURATION);
  inline_print_offset_sensor(cnt++, B3_GROUND_TEMPERATURE);
  inline_print_offset_sensor(cnt++, B4_SURFACE_TEMPERATURE);
  inline_print_offset_sensor(cnt++, B5_SOIL_TEMPERATURE_5CM);
  inline_print_offset_sensor(cnt++, B6_SOIL_TEMPERATURE_10CM);
  inline_print_offset_sensor(cnt++, B7_SOIL_TEMPERATURE_20CM);
  inline_print_offset_sensor(cnt++, B8_SOIL_TEMPERATURE_30CM);
  inline_print_offset_sensor(cnt++, B9_SOIL_TEMPERATURE_50CM);
  inline_print_offset_sensor(cnt++, B10_SOIL_TEMPERATURE_100CM);
  inline_print_offset_sensor(cnt++, B11_SOIL_TEMPERATURE_150CM);
  inline_print_offset_sensor(cnt++, B12_SOIL_TEMPERATURE_300CM);
  inline_print_offset_sensor(cnt++, B13_SOIL_TEMPERATURE_500CM);
  return cnt;
}


int32_t menu_offset_pressure(void)
{
  float temperature;
  driver_t *driver;
  uint8_t error;
  float local_temperature;
  int driver_num;
  adc_config_t *config;
  float voltage;
  float calibrated_voltage;
  int status;
  int ok;

  driver_num = get_driverNum(get_config_app()->sensor[A7_PRESSURE].type);

  if (driver_num != GENERAL_ADC)
  {
    io_printf("ADC가 아닙니다\r\n");
    return 0;
  }

  config = get_sensor_config(&get_config_app()->sensor[A7_PRESSURE]);
  driver = get_sensor_driver(A7_PRESSURE);
  temperature = read_sensor_barometer(driver, &error);

  io_printf("%s 장비 값:%fhpa\r\n", sensor_name_list[A7_PRESSURE], temperature);
  io_printf("현장 값 입력해주세요\r\n");
  io_printf("입력:");
  if(cli_scanf_s("%f",&local_temperature)>0)
  {
    voltage = adc_read_single_avg(config->channel,&error,10);
    io_printf("현재 ADC 싱글 %d 전압:%fv\r\n",config->channel,voltage);
    calibrated_voltage = cvt_data_to_voltage(config,local_temperature);
    io_printf("요구되는 전압:%f\r\n", calibrated_voltage);
    status  = confirm_continue("오프셋을 조정합니다",&ok);
    if(status != MENU_OK)
    if(ok)
    {
      float new_offset = calibrated_voltage - voltage;
      adc_set_offset_trim(eSINGLE_ADC, config->channel, new_offset);
      io_printf("현장센서에맞게 오프셋 %f 적용됩니다\n",new_offset);
    }

  }
  
  return 0;

}

int aws_menu_offset(void)
{
  int choice, status;
  int max_number;
  float offset;

  while (1)
  {
    max_number = print_offset_sensor();
    status = input_decimal_prompt("선택", &choice, 0, max_number-1);//수위  제외

    if (status != MENU_OK)
    {
      break;
    }

    io_printf("%s offset 을 입력해주세요\r\n", sensor_name_list[s_offset_sensor_index[choice]]);
    io_printf("입력:");
    if(cli_scanf_s("%f", &offset)>0)
    {
      config.sensor[s_offset_sensor_index[choice]].offset = offset;
      WRITE_CFG(sensor[s_offset_sensor_index[choice]].offset);
      io_printf("수정되었습니다\r\n");
    }

    switch (s_offset_sensor_index[choice])
    {
      case A7_PRESSURE:  
        menu_offset_pressure();
        break;

      default:
        break;
    }
  }

  return status;
}