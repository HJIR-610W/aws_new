

#include "cli_key_code.h"
#include "dev_io.h"
#include "driver_di.h"
#include "Sensors\rain_present\rain_present.h"
bool g_reed_rain=false;

void test_rain_reed_callBack(void *arg)
{
  g_reed_rain = true;
}

bool g_hall_rain = false;

void test_rain_hall_callBack(void *arg)
{
  g_hall_rain = true;
}

void test_rain(void)
{
  di_isr_set_cfg_t isr_cfg;
  driver_t *rain_reed;
  driver_t *rain_hall;
  driver_t *rain_hall_err;
  driver_t *rain_present;
  uint8_t err=0;
  int hall_status;
  int prev_hall_status;
  int rain_present_status = -1;
  int prev_rain_present_status=-1;
  int once=1;

  rain_present = rainPresent_open(RAIN_PRESENT_DI,0);

  rain_hall_err = driver_di_open(DI_RAIN_HALL_ERR, 0);
  rain_hall = driver_di_open(DI_RAIN_HALL, 0);

  isr_cfg.call = test_rain_hall_callBack;
  isr_cfg.name = "rain_hall";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio = 5;
  driver_di_set(rain_hall, DI_SET_INTERRUPT, &isr_cfg);

  rain_reed = driver_di_open(DI_RAIN_REED, 0);

  isr_cfg.call = test_rain_reed_callBack ;
  isr_cfg.name = "rain_reed";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio = 5;
  driver_di_set(rain_reed, DI_SET_INTERRUPT, &isr_cfg);


  debug_printf("우량을 1초 간격으로 입력해주세요\r\n");

  hall_status = driver_di_read(rain_hall_err);

  
  prev_hall_status = hall_status;

  while (1)
  {
    hall_status = driver_di_read(rain_hall_err);
    if (once ||hall_status != prev_hall_status)
    {
      if(once==1)
      {
        once = 0;
      }
      if(hall_status == 1)
      {
        debug_printf("홀센서 정상\r\n");
      }
      else
      {
        debug_printf("홀센서 에러\r\n");
      }
      prev_hall_status = hall_status;
    }


    if(g_reed_rain)
    {
      g_reed_rain = false;
      debug_printf("리드 우량\r\n");
    }

    if (g_hall_rain)
    {
      g_hall_rain = false;
      debug_printf("홀 우량\r\n");
    }

    if (read_sensor_rainPresent(rain_present,&err))
    {
      rain_present_status = 1;
    }
    else
    {
      rain_present_status = 0;
    }
    if (rain_present_status != prev_rain_present_status)
    {
      prev_rain_present_status = rain_present_status;
       if (rain_present_status == 1)
      {
        debug_printf("우량 감지\r\n");
      }
      else{
        debug_printf("우량 감지 해제\r\n");
      }
    }


    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      return;
    }
  }
}