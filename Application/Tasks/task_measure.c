#include <string.h>

#include "Sensors\temperature\temperature.h"
#include "Sensors\wind_speed\wind_speed.h"
#include "Sensors\wind_direction\wind_direction.h"
#include "Sensors\snow\snow.h"
#include "Sensors\rain\rain.h"
#include "Sensors\humidity\humidity.h"

#include "app_adc.h"
#include "app_rtc.h"
#include "app_file.h"
#include "aws_data.h"
#include "app_dataLogging.h"
#include "config.h"
#include "cmsis_os.h"
#include "task_logging.h"
#include "task_measure.h"
#include "mcu_delay.h"
#include "utile_time.h"

const osThreadAttr_t measureTask_attributes = {
  .name = "measureTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityHigh,
};



  uint32_t start_time;
  uint32_t elased_time;

void sensor_init(void)
{
    sensor_t *sensor;
      sensor = config.sensor;

  if(sensor[A9_SNOW_DEPTH].type)
  {
    snow_init(&sensor[A9_SNOW_DEPTH]);
  }

  if(sensor[A6_RAINFALL_DOT5_1MM].type)
  {
    rain_init(&sensor[A6_RAINFALL_DOT5_1MM]);
  }

}

extern void file_test(void);
void measureTask(void *arg)
{
  DATE_TIME_BUF ct;
  DATE_TIME_BUF ot;
  uint8_t err;
  uint8_t data[100];
  sensor_t *sensor;

  adc_init();

  sensor_init();
  sensorData_init();

  sensor = config.sensor;
  os_logging_printf("measure task");

  ct = Date_Time;
    
  memset(data,0xff,sizeof(data));
  while(1)
  {
    ct.Sec = Date_Time.Sec;
    ct.Min = Date_Time.Min;
    
    if(ct.Sec != ot.Sec)
    {
      if(ct.Min != ot.Min)
      {
        os_write_sensorData(&ct,data, sizeof(data),0,1);
        ot.Min = ct.Min;
      }
      ot.Sec = ct.Sec;
    }


    osDelay(250);
    
    start_time = mcu_get_clk();
   
    if(sensor[A1_TEMPERATURE].type)
    {
      sensor_data[A1_TEMPERATURE].data.f = read_sensor_temperature(&sensor[A1_TEMPERATURE],&err);
    }
    
    if(sensor[A2_WIND_DIRECTION].type)
    {
      sensor_data[A2_WIND_DIRECTION].data.f = read_sensor_windDirection(&sensor[A2_WIND_DIRECTION],&err);
    }
    if(sensor[A3_WIND_SPEED].type)
    {
      sensor_data[A3_WIND_SPEED].data.f = read_sensor_windDirection(&sensor[A3_WIND_SPEED],&err);
    }
    
    if(sensor[A9_SNOW_DEPTH].type)
    {
      sensor_data[A9_SNOW_DEPTH].data.i = read_sensor_snow(&sensor[A9_SNOW_DEPTH],&err);
    }

    if(sensor[A6_RAINFALL_DOT5_1MM].type)
    {
      sensor_data[A6_RAINFALL_DOT5_1MM].data.i = read_sensor_rain(&sensor[A6_RAINFALL_DOT5_1MM],&err);
    }

    if(sensor[A10_RELATIVE_HUMIDITY].type)
    {
      sensor_data[A10_RELATIVE_HUMIDITY].data.f = read_sensor_humidity(&sensor[A10_RELATIVE_HUMIDITY],&err);
    }

   elased_time =mcu_cal_elapse_us(start_time);

  }
  
}

void measureTask_init(void)
{
  osThreadNew(measureTask, NULL, &measureTask_attributes);
}