


#include "driver_led.h"

#include "main.h"

typedef struct gpio_api_s
{
  void (*on)(void);
  void (*off)(void);
}gpio_api_t;


static const gpio_api_t g_gpio;

void led_on(void)
{
    HAL_GPIO_WritePin(GPIOH,OUT_SYS_RUN_Pin,GPIO_PIN_RESET);
}


void led_off(void)
{
    HAL_GPIO_WritePin(GPIOH,OUT_SYS_RUN_Pin,GPIO_PIN_SET);
}






void driver_led_init(driver_led_t *led,uint32_t num)
{
  switch(num)
  {
    case LED_SYS_RUN:
        led->api = (void *)&g_gpio;
        led->num = num;
    break;
  }

}


void driver_led_on(driver_led_t *led)
{
  ((gpio_api_t *)led->api)->on();
  
}

void driver_led_off(driver_led_t *led)
{
  ((gpio_api_t *)led->api)->off();
}


static const gpio_api_t g_gpio={.on = led_on,
                   .off = led_off
};
