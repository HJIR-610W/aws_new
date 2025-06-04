#include "app_bsp.h"
#include <math.h>
#include "driver_adc.h"
#include "driver_led.h"
#include "driver_do.h"
#include "driver_di.h"


driver_t *g_status_led;

driver_t *g_port_mode;

void status_led_init(void);

void set_portd_hart_mode(void)
{
  driver_do_high(g_port_mode);
}

void set_portd_rs232_mode(void)
{
  driver_do_low(g_port_mode);
}

void app_bsp_init(void)
{

  status_led_init();

  g_port_mode = driver_do_open(DO_HART_SEL, 0);



  set_portd_rs232_mode();
}





void status_led_init(void)
{
  led_freq_cfg_t cfg = {.freq = 5, .highDuty = 10};

  g_status_led = driver_led_open(LED_SYS_RUN);

  driver_led_set(g_status_led, LED_CMD_SET_TOGGLE_FREQ, &cfg);
  driver_led_set(g_status_led, LED_CMD_START, NULL);
}

void status_led_on(void)
{
  driver_led_set(g_status_led, LED_CMD_START, NULL);
}

void status_led_off(void)
{
  driver_led_set(g_status_led, LED_CMD_STOP, NULL);
}



void status_led_set(int mode)
{
  switch (mode)
  {
    case LED_BLINK:
      {
       led_freq_cfg_t cfg = {.freq = 1, .highDuty = 10};
      driver_led_set(g_status_led, LED_CMD_SET_TOGGLE_FREQ, &cfg);
      }
      break;
    case LED_ON:
    break;
    default:
      break;
  }
}