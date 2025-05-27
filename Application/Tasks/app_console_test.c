#include "app_console_test.h"

#include <stdlib.h>
#include <string.h>

#include "app_rs232.h"
#include "app_rs485.h"
#include "app_sensor.h"
#include "dev_io.h"
#include "driver_di.h"
#include "driver_do.h"
#include "pcb_define.h"
#include "utile.h"
#include "vt100_command.h"
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
    io_printf("%-8s ", port_names[i]);  // 포트 이름 간격 정렬
  }
  io_printf("\r\n");

  // 2. 각 핀 상태를 행 단위로 출력
  for (uint8_t pin = 0; pin < max_pins; pin++)
  {
    for (uint8_t i = 0; i < num_ports; i++)
    {
      GPIO_TypeDef *port = ports[i];

      // 핀이 입력 모드인지 확인
      if (port->MODER & (0x3 << (pin * 2)))  // 입력 모드가 아닌 경우
      {
        io_printf("%2d:%-5s ", pin, "-");  // 출력 모드가 아닌 핀 표시
      }
      else
      {
        uint8_t pin_state =
            (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        io_printf("%2d:%-5s ", pin,
                     pin_state ? "HIGH" : "LOW");  // 핀 번호와 상태 출력
      }
    }
    io_printf("\r\n");
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
    io_printf("%-8s ", port_names[i]);  // 포트 이름 간격 정렬
  }
  io_printf("\r\n");

  // 2. 각 핀 상태를 행 단위로 출력
  for (uint8_t pin = 0; pin < max_pins; pin++)
  {
    for (uint8_t i = 0; i < num_ports; i++)
    {
      GPIO_TypeDef *port = ports[i];

      // 핀이 입력 모드인지 확인
      if (port->MODER & (0x3 << (pin * 2)))  // 입력 모드가 아닌 경우
      {
        io_printf("%2d:%-5s ", pin, "-");  // 출력 모드가 아닌 핀 표시
      }
      else
      {
        uint8_t pin_state =
            (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        io_printf("%2d:%-5s ", pin,
                     pin_state ? "HIGH" : "LOW");  // 핀 번호와 상태 출력
      }
    }
    io_printf("\r\n");
  }
}

#include <stdio.h>

#include "stm32f4xx.h"  // HAL 라이브러리 헤더 포함

#define printf io_printf  // 사용자 스타일에 맞춘 printf 매크로 정의

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
    io_printf("%-8s ", port_names[i]);  // 포트 이름 간격 정렬
  }
  io_printf("\r\n");

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
        io_printf("%2d:%-5s ", pin, "-");  // 출력 모드가 아닌 핀 표시
      }
      else
      {
        uint8_t pin_state =
            (port->ODR & (1 << pin)) ? 1 : 0;  // ODR에서 핀 상태 읽기
        io_printf("%2d:%-5s ", pin,
                     pin_state ? "HIGH" : "LOW");  // 핀 번호와 상태 출력
      }
    }
    io_printf("\r\n");
  }
}

int32_t mcu_pin(p_shell_context_t ctx, int32_t argc, char **argv)
{
  char ch;

  if (strncmp(argv[1], "di", 2) == 0)
  {
    io_printf(VT100_CLEAR_SCREEN);
    io_printf(VT100_CURSOR_OFF);
    do
    {
      io_printf(VT100_CURSOR_HOME);
      print_gpio_states_in_table();

      debug_recv(&ch, 1, 100);
    } while (ch != ASCII_CODE_CTRL_Q);

    io_printf(VT100_CURSOR_ON);
  }
  else if (strncmp(argv[1], "do", 2) == 0)
  {
    print_gpio_output_states();
  }
  return 0;
}

int32_t pcb_pin(p_shell_context_t ctx, int32_t argc, char **argv)
{
  GPIO_TypeDef *ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
                           GPIOF, GPIOG, GPIOH, GPIOI};

  io_printf("GREEN[OUT],WHITE[IN],YELLOW[AF]\r\n");
  for (int i = 0; i < _countof(pcbPortNameList) / 2; i++)
  {
    io_printf("%-23s ", pcbPortNameList[i]);
  }
  io_printf("\r\n");

  for (int pin = 0; pin < 16; pin++)
  {
    for (int j = 0; j < _countof(pcbPortNameList) / 2; j++)
    {
      GPIO_TypeDef *port = ports[j];
      if ((port->MODER & (0x3 << (pin * 2))) ==
          (0x1 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        uint8_t pin_state = (port->ODR & (1 << pin)) ? 1 : 0;  // ODR 출력력
        vt100_printfColor(GREEN, "%-20s[%d] ", pcbPinNameList[j][pin],
                          pin_state);  // 출력핀이 아닌경우
      }
      else if ((port->MODER & (0x3 << (pin * 2))) == 0)  // 입력력
      {
        uint8_t pin_state =
            (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        vt100_printfColor(WHITE, "%-20s[%d] ", pcbPinNameList[j][pin],
                          pin_state);  // 출력핀이 아닌경우
      }
      else if ((port->MODER & (0x3 << (pin * 2))) ==
               (0x11 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        vt100_printfColor(YELLOW, "%-20s[A] ", pcbPinNameList[j][pin]);
      }
      else
      {
        vt100_printfColor(YELLOW, "%-20s[F] ", pcbPinNameList[j][pin]);
      }
    }
    io_printf("\r\n");
  }
  io_printf("\r\n");
  for (int i = _countof(pcbPortNameList) / 2; i < _countof(pcbPortNameList);
       i++)
  {
    io_printf("%-23s ", pcbPortNameList[i]);
  }
  io_printf("\r\n");

  for (int pin = 0; pin < 16; pin++)
  {
    for (int j = _countof(pcbPortNameList) / 2; j < _countof(pcbPortNameList);
         j++)
    {
      GPIO_TypeDef *port = ports[j];
      if ((port->MODER & (0x3 << (pin * 2))) ==
          (0x1 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        uint8_t pin_state =
            (port->ODR & (1 << pin)) ? 1 : 0;  // ODR에서 핀 상태 읽기
        vt100_printfColor(GREEN, "%-20s[%d] ", pcbPinNameList[j][pin],
                          pin_state);  // 출력핀이 아닌경우
      }
      else if ((port->MODER & (0x3 << (pin * 2))) == 0)  // 입력력
      {
        uint8_t pin_state =
            (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        vt100_printfColor(WHITE, "%-20s[%d] ", pcbPinNameList[j][pin],
                          pin_state);  // 출력핀이 아닌경우
      }
      else if ((port->MODER & (0x3 << (pin * 2))) ==
               (0x11 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        vt100_printfColor(YELLOW, "%-20s[A] ", pcbPinNameList[j][pin]);
      }
      else
      {
        vt100_printfColor(YELLOW, "%-20s[F] ", pcbPinNameList[j][pin]);
      }
    }
    io_printf("\r\n");
  }

  return 0;
}

int32_t print_di(p_shell_context_t ctx, int32_t argc, char **argv)
{
  driver_t *din;
  int32_t input;

  for (int i = 0; i < 8; i++)
  {
    din = driver_di_open(DI_EXT_0 + i, 0);
    if (din)
    {
      input = driver_di_read(din);
      io_printf("EXT_%d:%d\r\n", i, input);
    }
  }
  return 0;
}

int32_t ctrl_do(p_shell_context_t ctx, int32_t argc, char **argv)
{
  driver_t *gp;
  char *endptr;
  int32_t pin;
  int32_t pin_state;
  driver_t *dout;

  pin = strtol(argv[1], &endptr, 10);
  pin_state = strtol(argv[2], &endptr, 10);

  dout = driver_do_open(pin, 0);

  if (dout)
  {
    if (pin_state)
    {
      driver_do_high(dout);
    }
    else
    {
      driver_do_low(dout);
    }
  }

  return 0;
}

int32_t test_pcb(p_shell_context_t ctx, int32_t argc, char **argv)
{
  run_test_root();
}