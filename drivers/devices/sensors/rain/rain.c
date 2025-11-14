
#include <stdio.h>
#include <stdlib.h>


#include "os_user_def.h"
#include "dev_io.h"
#include "Sensors\rain\rain.h"
#include "task_isrEvent.h"
#include "pcb_define.h"
#include "drv_di.h"
#include "util_time.h"


rain_data_t rain_data;


static uint16_t g_rainPulse;
static osSemaphoreId_t g_rainSemId = NULL;

volatile uint32_t g_last_pulse_time;


void increase_rain(void)
{
  OS_PEND_SEM(g_rainSemId, osWaitForever);

    g_rainPulse += 1;
    OS_POST_SEM(g_rainSemId);

}

uint16_t peek_rain(void)
{
	uint16_t ret=0;
  OS_PEND_SEM(g_rainSemId, osWaitForever);

		ret = g_rainPulse;
    OS_POST_SEM(g_rainSemId);
	
	return ret;
}

uint16_t get_rain(uint16_t cnt)
{
  uint16_t ret=0;
  OS_PEND_SEM(g_rainSemId, osWaitForever);
 
    if(cnt>= g_rainPulse)
      {
        g_rainPulse -= cnt;
              ret = cnt;
      }
      OS_POST_SEM(g_rainSemId);

  return ret;
}


void rain_reed_callback(int32_t arg)
{
  uint32_t current_time = OS_GET_TICK();

  if((current_time - g_last_pulse_time)>= 1000)
  {
    isr_event_cmd_t event;

    event.cmd = eRAIN_REED_INT;
    os_send_event(&event, 0);


    g_last_pulse_time = OS_GET_TICK();
  }
}

void rain_hall_callback(int32_t arg)
{
  uint32_t current_time = OS_GET_TICK();

  if((current_time - g_last_pulse_time)>= 1000)
  {
    isr_event_cmd_t event;
    event.cmd = eRAIN_HALL_INT;
    os_send_event(&event, 0);

    g_last_pulse_time = OS_GET_TICK();
}
}



#define RAIN_REED_05MM 100
#define RAIN_REED_1MM  101
#define RAIN_HALL_05MM 102
#define RAIN_HALL_1MM  103

void rain_init(uint32_t num)
{

  di_isr_set_cfg_t isr_cfg;

  g_rainSemId = osSemaphoreNew(1, 1, NULL); 

  switch (num)
  {
  case RAIN_REED_05MM:
  case RAIN_REED_1MM:
    isr_cfg.call    = rain_reed_callback;
    isr_cfg.name    = "rain_pulse";
    isr_cfg.trigger = eDI_FALLING;
    isr_cfg.prio    = 5;
    drv_di_set_interrupt(DRV_DI_RAIN_REED, &isr_cfg);
  break;
  
  case RAIN_HALL_05MM:
  case RAIN_HALL_1MM:
    isr_cfg.call    = rain_hall_callback;
    isr_cfg.name    = "rain_hall";
    isr_cfg.trigger = eDI_FALLING;
    isr_cfg.prio    = 5;
    drv_di_set_interrupt(DRV_DI_RAIN_HALL, &isr_cfg);
    break;
  }
}



int32_t read_rainHallErr(void)
{
  if (drv_di_read(DRV_DI_RAIN_HALL_ERR))
  {
    return 0;
  }

  return 1; 
}



typedef struct rain_cfg_s
{
  float pulse;
  uint8_t type;
}rain_cfg_t;

driver_t rain_driver;
rain_cfg_t rain_cfg;


driver_t *rain_open(int32_t num,void *opt)
{

  if(rain_driver.opened)
  {
    return &rain_driver;
  }
  
  rain_init(num);
  switch (num)
  {
    case RAIN_REED_05MM:
    rain_cfg.pulse = 0.5;
    rain_cfg.type = RAIN_REED_05MM;
    break;
    case RAIN_REED_1MM:
    rain_cfg.pulse = 1;
    rain_cfg.type = RAIN_REED_1MM;
    break;
    case RAIN_HALL_05MM:
    rain_cfg.pulse = 0.5;
    rain_cfg.type = RAIN_HALL_05MM;
    break;
    case RAIN_HALL_1MM:
    rain_cfg.pulse = 1;
    rain_cfg.type = RAIN_HALL_1MM;
    break;
  }

  rain_driver.cfg = &rain_cfg;

  return &rain_driver;

}

float read_sensor_rain(driver_t *driver,uint8_t *err)
{
  int32_t rain=0;
  rain_cfg_t *cfg = driver->cfg;
  uint16_t data = 0;

  if (cfg->type == RAIN_HALL_05MM || cfg->type == RAIN_HALL_1MM)
  {
    *err = read_rainHallErr();
  }
  else
  {
    *err = 0;
  }

    rain =  peek_rain();
  if(rain)
  {
   data = get_rain(rain);
  }

  return (data*cfg->pulse);
}