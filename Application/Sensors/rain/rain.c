
#include <stdio.h>
#include <stdlib.h>


#include "os_define.h"
#include "dev_io.h"
#include "Sensors\rain\rain.h"
#include "task_isrEvent.h"
#include "pcb_define.h"
#include "driver_di.h"
#include "utile_time.h"


rain_data_t rain_data;

driver_t *g_hallStatusDriver;

static uint16_t g_rainPulse;
static osSemaphoreId_t g_rainSemId = NULL;

volatile uint32_t g_last_pulse_time;


void increase_rain(void)
{
  if(OS_SEM_PEND(g_rainSemId, osWaitForever) == osOK)
  {
    g_rainPulse += 1;
    OS_SEM_POST(g_rainSemId);
  }
}

uint16_t peek_rain(void)
{
	uint16_t ret=0;
  if(OS_SEM_PEND(g_rainSemId, osWaitForever) == osOK)
	{
		ret = g_rainPulse;
    OS_SEM_POST(g_rainSemId);
	}
	return ret;
}

uint16_t get_rain(uint16_t cnt)
{
  uint16_t ret=0;
  if(OS_SEM_PEND(g_rainSemId, osWaitForever) == osOK)
  {
    if(cnt>= g_rainPulse)
      {
        g_rainPulse -= cnt;
              ret = cnt;
      }
      OS_SEM_POST(g_rainSemId);
    }	
  return ret;
}


void rainReedCallBack(void *arg)
{
  uint32_t current_time = OS_GET_TICK();

  if((current_time - g_last_pulse_time)>= 1000)
  {
    os_send_isrEvent(eRAIN_REED_INT,0);
    g_last_pulse_time = OS_GET_TICK();
  }
}

void rainHallCallBack(void *arg)
{
  uint32_t current_time = OS_GET_TICK();

  if((current_time - g_last_pulse_time)>= 1000)
  {
  os_send_isrEvent(eRAIN_HALL_INT,0);
  g_last_pulse_time = OS_GET_TICK();
}
}

void rain_init(sensor_t *sensor)
{
  driver_t *rain_pulse;
  di_isr_set_cfg_t isr_cfg;


  g_rainSemId = osSemaphoreNew(1, 1, NULL); 
  g_hallStatusDriver  = driver_di_open(DI_RAIN_HALL_ERR,0);

  switch (sensor->type)
  {
  case S_T_GENERAL_232:
  /* code */
  break;
  case S_T_RAIN_REED_05MM:
  case S_T_RAIN_REED_1MM:
  rain_pulse = driver_di_open(DI_RAIN_REED,0);

  isr_cfg.call    = rainReedCallBack;
  isr_cfg.name    = "rain_pulse";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio    = 5;
  driver_di_set(rain_pulse,DI_SET_INTERRUPT,&isr_cfg);
  break;
  
  case S_T_RAIN_HALL_05MM:
  case S_T_RAIN_HALL_1MM:
    rain_pulse = driver_di_open(DI_RAIN_HALL,0);

    isr_cfg.call    = rainHallCallBack;
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

/**
 * @brief 1mm 10, 0.5mm 5
 */
int32_t read_sensor_rain(sensor_t *sensor,uint8_t *err)
{
  uint16_t rain=0;
  uint16_t scale=1;
  int32_t data=0;

  
  switch (sensor->type)
  {
  case S_T_RAIN_REED_05MM:
  scale = 5;
  break;
  case S_T_RAIN_REED_1MM:
  scale = 10;
  break;
  case S_T_RAIN_HALL_05MM:
  scale = 5;
  break;
  case S_T_RAIN_HALL_1MM:
  scale = 10;
  break;
  }
  
  rain =  peek_rain();
  if(rain)
  {
   data = get_rain(rain);
  }
  return data*scale;
}


int32_t read_rainHallErr(void)
{
  if(driver_di_read(g_hallStatusDriver))
  {
    return 1;
  }

  return 0;
}
uint16_t calculate_yearRain(DATE_TIME_BUF *ct)
{

}


uint16_t calculate_monthRain(DATE_TIME_BUF *ct)
{

}