
#include "Sensors\temperature\temperature.h"
#include "app_adc.h"
#include "app_rtc.h"
#include "aws_data.h"
#include "config.h"
#include "cmsis_os.h"

#include "task_measure.h"

const osThreadAttr_t measureTask_attributes = {
  .name = "measureTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityHigh,
};




void measureTask(void *arg)
{
  int32_t data;
  uint8_t err;
  sensor_t *sensor;
  sensor_data_t *psensor_data;
  adc_init();



  while(1)
  {
    osDelay(250);
    rtc_update();
   
   sensor       = &config.sensor[A1_TEMPERATURE]; 
   psensor_data = &sensor_data[A1_TEMPERATURE];

   if(sensor->type)
   {
      psensor_data->data = read_sensor_temperature(sensor,&err);
   }
    
  }
  
}

void measureTask_init(void)
{
  osThreadNew(measureTask, NULL, &measureTask_attributes);
}