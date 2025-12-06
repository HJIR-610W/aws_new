#include "app_console_test.h"

#include <stdlib.h>
#include <string.h>


#include "drv_rs485.h"
#include "app_sensor.h"
#include "drv_rs232.h"
#include "debug_io.h"
#include "drv_di.h"
#include "drv_do.h"
#include "pcb_define.h"
#include "util_memory.h"
 
#include "console_test.h"



void print_gpio_states_in_table_old()
{
  GPIO_TypeDef *ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
                           GPIOF, GPIOG, GPIOH, GPIOI};
  const char *port_names[] = {"GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE",
                              "GPIOF", "GPIOG", "GPIOH", "GPIOI"};
  uint8_t num_ports = sizeof(ports) / sizeof(ports[0]);
  uint8_t max_pins = 16;  // GPIO 핀은 최대 16개

  // 1. 첫 번째 행: 포트 이름 출력
  for (uint8_t i = 0; i < num_ports; i++)
  {
    debug_printf("%-8s ", port_names[i]);  // 포트 이름 간격 정렬
  }
  debug_printf("\r\n");

  // 2. 각 핀 상태를 행 단위로 출력
  for (uint8_t pin = 0; pin < max_pins; pin++)
  {
    for (uint8_t i = 0; i < num_ports; i++)
    {
      GPIO_TypeDef *port = ports[i];

      // 핀이 입력 모드인지 확인
      if (port->MODER & (0x3 << (pin * 2)))  // 입력 모드가 아닌 경우
      {
        debug_printf("%2d:%-5s ", pin, "-");  // 출력 모드가 아닌 핀 표시
      }
      else
      {
        uint8_t pin_state =
            (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        debug_printf("%2d:%-5s ", pin,
                     pin_state ? "HIGH" : "LOW");  // 핀 번호와 상태 출력
      }
    }
    debug_printf("\r\n");
  }
}

void print_gpio_states_in_table()
{
  GPIO_TypeDef *ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
                           GPIOF, GPIOG, GPIOH, GPIOI};
  const char *port_names[] = {"GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE",
                              "GPIOF", "GPIOG", "GPIOH", "GPIOI"};
  uint8_t num_ports = sizeof(ports) / sizeof(ports[0]);
  uint8_t max_pins = 16;  // GPIO 핀은 최대 16개

  // 1. 첫 번째 행: 포트 이름 출력
  for (uint8_t i = 0; i < num_ports; i++)
  {
    debug_printf("%-8s ", port_names[i]);  // 포트 이름 간격 정렬
  }
  debug_printf("\r\n");

  // 2. 각 핀 상태를 행 단위로 출력
  for (uint8_t pin = 0; pin < max_pins; pin++)
  {
    for (uint8_t i = 0; i < num_ports; i++)
    {
      GPIO_TypeDef *port = ports[i];

      // 핀이 입력 모드인지 확인
      if (port->MODER & (0x3 << (pin * 2)))  // 입력 모드가 아닌 경우
      {
        debug_printf("%2d:%-5s ", pin, "-");  // 출력 모드가 아닌 핀 표시
      }
      else
      {
        uint8_t pin_state =
            (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        debug_printf("%2d:%-5s ", pin,
                     pin_state ? "HIGH" : "LOW");  // 핀 번호와 상태 출력
      }
    }
    debug_printf("\r\n");
  }
}

#include <stdio.h>

#include "stm32f4xx.h"  // HAL 라이브러리 헤더 포함

#define printf debug_printf  // 사용자 스타일에 맞춘 printf 매크로 정의

void print_gpio_output_states()
{
  GPIO_TypeDef *ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
                           GPIOF, GPIOG, GPIOH, GPIOI};
  const char *port_names[] = {"GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE",
                              "GPIOF", "GPIOG", "GPIOH", "GPIOI"};
  uint8_t num_ports = sizeof(ports) / sizeof(ports[0]);
  uint8_t max_pins = 16;  // GPIO 핀은 최대 16개

  // 1. 첫 번째 행: 포트 이름 출력
  for (uint8_t i = 0; i < num_ports; i++)
  {
    debug_printf("%-8s ", port_names[i]);  // 포트 이름 간격 정렬
  }
  debug_printf("\r\n");

  // 2. 각 핀 상태를 행 단위로 출력
  for (uint8_t pin = 0; pin < max_pins; pin++)
  {
    for (uint8_t i = 0; i < num_ports; i++)
    {
      GPIO_TypeDef *port = ports[i];

      // 핀이 출력 모드인지 확인
      if ((port->MODER & (0x3 << (pin * 2))) !=
          (0x1 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        debug_printf("%2d:%-5s ", pin, "-");  // 출력 모드가 아닌 핀 표시
      }
      else
      {
        uint8_t pin_state =
            (port->ODR & (1 << pin)) ? 1 : 0;  // ODR에서 핀 상태 읽기
        debug_printf("%2d:%-5s ", pin,
                     pin_state ? "HIGH" : "LOW");  // 핀 번호와 상태 출력
      }
    }
    debug_printf("\r\n");
  }
}

int32_t mcu_pin(p_shell_context_t ctx, int32_t argc, char **argv)
{
  uint8_t ch;

  if (strncmp(argv[1], "di", 2) == 0)
  {
    debug_printf(ES_CLEAR_SCREEN);
    debug_printf(ES_CURSOR_OFF);
    do
    {
      debug_printf(ES_CURSOR_HOME_ALT);
      print_gpio_states_in_table();

      debug_recv(&ch, 1, 100);
    } while (ch != ASCII_CODE_CTRL_Q);

    debug_printf(ES_CURSOR_ON);
  }
  else if (strncmp(argv[1], "do", 2) == 0)
  {
    print_gpio_output_states();
  }
  return 0;
}

int32_t pcb_pin(void)
{
  GPIO_TypeDef *ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
                           GPIOF, GPIOG, GPIOH, GPIOI};

  debug_printf("GREEN[OUT],WHITE[IN],YELLOW[AF]\r\n");
  for (uint32_t i = 0; i < _countof(pcbPortNameList) / 2; i++)
  {
    debug_printf("%-23s ", pcbPortNameList[i]);
  }
  debug_printf("\r\n");

  for (uint32_t pin = 0; pin < 16; pin++)
  {
    for (uint32_t j = 0; j < _countof(pcbPortNameList) / 2; j++)
    {
      GPIO_TypeDef *port = ports[j];
      if ((port->MODER & (0x3 << (pin * 2))) ==
          (0x1 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        uint8_t pin_state = (port->ODR & (1 << pin)) ? 1 : 0;  // ODR 출력력
        debug_printf_color((int32_t)GREEN, "%-20s[%d] ", pcbPinNameList[j][pin],
                          pin_state);  // 출력핀이 아닌경우
      }
      else if ((port->MODER & (0x3 << (pin * 2))) == 0)  // 입력력
      {
        uint8_t pin_state =
            (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        debug_printf_color((int32_t)WHITE, "%-20s[%d] ", pcbPinNameList[j][pin],
                          pin_state);  // 출력핀이 아닌경우
      }
      else if ((port->MODER & (0x3 << (pin * 2))) ==
               (0x11 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        debug_printf_color((int32_t)YELLOW, "%-20s[A] ", pcbPinNameList[j][pin]);
      }
      else
      {
        debug_printf_color((int32_t)YELLOW, "%-20s[F] ", pcbPinNameList[j][pin]);
      }
    }
    debug_printf("\r\n");
  }
  debug_printf("\r\n");
  for (uint32_t i = _countof(pcbPortNameList) / 2; i < _countof(pcbPortNameList);
       i++)
  {
    debug_printf("%-23s ", pcbPortNameList[i]);
  }
  debug_printf("\r\n");

  for (uint32_t pin = 0; pin < 16; pin++)
  {
    for (uint32_t j = _countof(pcbPortNameList) / 2; j < _countof(pcbPortNameList);
         j++)
    {
      GPIO_TypeDef *port = ports[j];
      if ((port->MODER & (0x3 << (pin * 2))) ==
          (0x1 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        uint8_t pin_state =
            (port->ODR & (1 << pin)) ? 1 : 0;  // ODR에서 핀 상태 읽기
        debug_printf_color((int32_t)GREEN, "%-20s[%d] ", pcbPinNameList[j][pin],
                          pin_state);  // 출력핀이 아닌경우
      }
      else if ((port->MODER & (0x3 << (pin * 2))) == 0)  // 입력력
      {
        uint8_t pin_state =
            (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        debug_printf_color((int32_t)WHITE, "%-20s[%d] ", pcbPinNameList[j][pin],
                          pin_state);  // 출력핀이 아닌경우
      }
      else if ((port->MODER & (0x3 << (pin * 2))) ==
               (0x11 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        debug_printf_color((int32_t)YELLOW, "%-20s[A] ", pcbPinNameList[j][pin]);
      }
      else
      {
        debug_printf_color((int32_t)YELLOW, "%-20s[F] ", pcbPinNameList[j][pin]);
      }
    }
    debug_printf("\r\n");
  }

  return 0;
}

int32_t print_di(p_shell_context_t ctx, int32_t argc, char **argv)
{

  int32_t input;
  int di_list[8];

  di_list[0] = DRV_DI_0;
  di_list[1] = DRV_DI_1;
  di_list[2] = DRV_DI_2;
  di_list[3] = DRV_DI_3;
  di_list[4] = DRV_DI_4;
  di_list[5] = DRV_DI_RAIN_DETECT_DIGITAL;


  for (int i = 0; i < 6; i++)
  {
    input = drv_di_read(di_list[i]);
    debug_printf("EXT_%d:%d\r\n", i, input);

  }
  return 0;
}



int32_t test_pcb(p_shell_context_t ctx, int32_t argc, char **argv)
{
  run_test_root();
  
  return 0;
}