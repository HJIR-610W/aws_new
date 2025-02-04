
#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os.h"
#include "dev_io.h"
#include "Sensors\rain\rain.h"
#include "task_isrEvent.h"
#include "pcb_define.h"
#include "driver_di.h"

static uint16_t g_rainPulse;

void rainReedCallBack(void *arg)
{
  os_send_isrEvent(eRAIN_REED_INT,0);
}

void rainHallCallBack(void *arg)
{
  os_send_isrEvent(eRAIN_REED_INT,0);
}

void rain_init(sensor_t *sensor)
{
  driver_t *rain_pulse;
  di_isr_set_cfg_t isr_cfg;

  switch (sensor->type)
  {
  case S_T_GENERAL_232:
  /* code */
  break;
  case S_T_RAIN_REED_05MM:
  case S_T_RAIN_REED_1MM:
  case S_T_RAIN_HALL_05MM:
  case S_T_RAIN_HALL_1MM:
    rain_pulse = driver_di_open(DI_RAIN_REED,0);

    isr_cfg.call    = rainReedCallBack;
    isr_cfg.name    = "rain_pulse";
    isr_cfg.trigger = eDI_FALLING;
    isr_cfg.prio    = 5;

    driver_di_set(rain_pulse,DI_SET_INTERRUPT,&isr_cfg);

  break;
  default:
    break;
  }
}



int32_t read_rain(dev_io_t *dev,uint8_t *err)
{
  return 0;
}

int32_t read_rain_rs232(dev_io_t *dev,uint8_t *err)
{
  char frame[10];
  uint16_t len;
  int32_t data=0;
  static uint8_t cnt=0;
  int32_t recvCnt;
  char *endptr;
 len= snprintf(frame,sizeof(frame),"Q_RAIN:%d\r\n",cnt);

  dev_io_write(dev,(uint8_t *)frame,len,0);
    
  len = dev_io_read(dev,(uint8_t*)frame,sizeof(frame),0,(void *)5);

  if(len)
  {
    frame[len]=0;

    recvCnt = strtol(&frame[1],&endptr,10);
   // if(frame[0]=='A'&&recvCnt==cnt)
    {
      *err = 0;
      data = 1;
    }

  }
  cnt++;
  return data;

}


int32_t read_sensor_rain(sensor_t *sensor,uint8_t *err)
{
  driver_t *rain_pulse;
  di_isr_set_cfg_t isr_cfg;
  rs232_config_t *rs232_config;
  int32_t data;
    dev_io_t dev_io;
  void *cfg;

  cfg = get_sensor_config(sensor);

  if(cfg == NULL)
  {
    *err = 2;

    return 0;
  }

  rs232_config = cfg;
  switch (sensor->type)
  {
  case S_T_GENERAL_232:

    dev_io.io = eRS232_IO;
    dev_io.handle = (void *)rs232_config->port;

  data =read_rain_rs232(&dev_io,err);
  break;
  case S_T_RAIN_REED_05MM:
  case S_T_RAIN_REED_1MM:
  case S_T_RAIN_HALL_05MM:
  case S_T_RAIN_HALL_1MM:

  break;
  default:
    break;
  }
  
  return 0;
}