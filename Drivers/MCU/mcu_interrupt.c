
#include <stdint.h>
#include "stm32f4xx_hal.h"

#include "utile.h"


typedef struct mcu_interrupt_list_s
{
  int init;
  int num;
  void (*call)(void *);
  void *handle;
}mcu_interrupt_list_t;

mcu_interrupt_list_t g_mcu_isr_list[16];



void exti_register(uint16_t pin,void *handle,void (*call)(void *))
{
  int pos;
  pin = getPinNumber(pin);

  if(pos !=-1)
  {
    g_mcu_isr_list[pin].handle = handle;
    g_mcu_isr_list[pin].call = call;
  }
}

// 인터럽트 핸들러 구현
void EXTI15_10_IRQHandler(void)
{
  void *handle=NULL;;
void (*call)(void *)=NULL;


    // PA10 ~ PA15의 인터럽트 확인 및 클리어
    for (uint16_t pin = GPIO_PIN_10; pin <= GPIO_PIN_15; pin <<= 1)
    {
        if (__HAL_GPIO_EXTI_GET_IT(pin) != RESET)
        {
            __HAL_GPIO_EXTI_CLEAR_IT(pin);  // 인터럽트 플래그 클리어

            // 인터럽트 발생 시 핀별로 수행할 작업
            if (pin == GPIO_PIN_10)
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            }
            else if (pin == GPIO_PIN_11)
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            } 
            else if (pin == GPIO_PIN_12)
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_13)
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_14) 
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_15) 
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            }
        }


    }
    
      /* USER CODE END EXTI15_10_IRQn 0 */
 // HAL_GPIO_EXTI_IRQHandler(IN_EX_UART_INT_7_Pin);
  //HAL_GPIO_EXTI_IRQHandler(IN_EX_UART_INT_8_Pin);
  //HAL_GPIO_EXTI_IRQHandler(INT_RTC_Pin);
}