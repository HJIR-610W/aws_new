#include "app_bsp.h"
#include "app_charger.h"
#include "app_di.h"
#include "app_do.h"
#include "app_rtc.h"
#include "cmsis_os2.h"
#include "driver_di.h"
#include "driver_do.h"
#include "driver_modbus.h"
#include "task_isrEvent.h"
#include "driver_uart.h"
driver_t *g_test_do;

const osThreadAttr_t kSystemTask_attributes = {
    .name = "systemTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

void userBtnCallBack(void *arg) { os_send_isrEvent(eUSER_BTN_INT, 0); }

void userBtn_init(void)
{
  driver_t *user_btn;
  di_isr_set_cfg_t isr_cfg;

  user_btn = driver_di_open(DI_USER_BTN, 0);

  isr_cfg.call = userBtnCallBack;
  isr_cfg.name = "user_btn";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio = 5;

  driver_di_set(user_btn, DI_SET_INTERRUPT, &isr_cfg);
}

void systemTask(void *arg)
{
  modbus_init_t m_init;
  uint16_t reg[10];
  driver_t *m_master;
  m_init.baud = 9600;
  m_init.parityIdx = 0;
  m_init.port_num = UART_2_EXT_A;
  m_init.stop = 1;

 //m_master = driver_modbus_master_open(DRIVER_MODBUS_MSTER_RTU_OVER_232, &m_init);
  while (1)
  {
   // driver_modbus_m_read_multi_reg(m_master, 0, 0, reg, 2);

    rtc_update();
    update_charger();

    for (int i = 0; i < 6; i++)
    {
      if (IS_DI_PRESSED(i))
      {
        write_do(i, 0);
      }
      else
      {
        write_do(i, 1);
      }
    }
    osDelay(500);
  }
}

void test_do(int out)
{
  if (out)
  {
    driver_do_high(g_test_do);
  }
  else
  {
    driver_do_low(g_test_do);
  }
}

void test_do_toggle(void)
{
  static int i = 0;

  i ^= 1;
  if (i)
  {
    driver_do_high(g_test_do);
  }
  else
  {
    driver_do_low(g_test_do);
  }
}

void systemTask_init(void)
{
  g_test_do = driver_do_open(DO_EXT_0, 0);

  app_bsp_init();

  userBtn_init();

  charger_init(APP_CHARGER_HJ);

  di_init();

  do_init();

  osThreadNew(systemTask, NULL, &kSystemTask_attributes);
}