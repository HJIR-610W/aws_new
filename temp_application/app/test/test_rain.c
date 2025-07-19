

#include "cli_key_code.h"
#include "dev_io.h"
#include "drv_di.h"
#include "drv_do.h"
#include "Sensors\rain_present\rain_present.h"
#include "drv_power.h"
bool g_reed_rain=false;

void test_rain_reed_callBack(int32_t arg)
{
  g_reed_rain = true;
}

bool g_hall_rain = false;

void test_rain_hall_callBack(int32_t arg)
{
  g_hall_rain = true;
}

void test_rain(void)
{
  di_isr_set_cfg_t isr_cfg;

  uint8_t err=0;
  int hall_status;
  int prev_hall_status;
  int rain_present_status = -1;
  int prev_rain_present_status=-1;
  int once=1;



  drv_power_on(DRV_POWER_RAIN_DECT_DIGITAL);



  isr_cfg.call = test_rain_hall_callBack;
  isr_cfg.name = "rain_hall";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio = 5;
  drv_di_set_interrupt(DRV_DI_RAIN_HALL, &isr_cfg);


  isr_cfg.call = test_rain_reed_callBack ;
  isr_cfg.name = "rain_reed";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio = 5;
  drv_di_set_interrupt(DRV_DI_RAIN_REED, &isr_cfg);

  io_printf("우량을 1초 간격으로 입력해주세요\r\n");
  io_printf("우량감지(디지털 주파수형)\r\n");
  hall_status = drv_di_read(DRV_DI_RAIN_HALL_ERR);

    prev_hall_status = hall_status;

  while (1)
  {
    hall_status = drv_di_read(DRV_DI_RAIN_HALL_ERR);
    if (once ||hall_status != prev_hall_status)
    {
      if(once==1)
      {
        once = 0;
      }
      if(hall_status == 1)
      {
        io_printf("홀센서 정상\r\n");
      }
      else
      {
        io_printf("홀센서 에러\r\n");
      }
      prev_hall_status = hall_status;
    }


    if(g_reed_rain)
    {
      g_reed_rain = false;
      io_printf("리드 우량\r\n");
    }

    if (g_hall_rain)
    {
      g_hall_rain = false;
      io_printf("홀 우량\r\n");
    }

    if (read_sensor_rainPresent(0,&err))
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
        io_printf("우량 감지\r\n");
      }
      else{
        io_printf("우량 감지 해제\r\n");
      }
    }


    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      return;
    }
  }
}