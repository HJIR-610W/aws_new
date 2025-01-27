#include <string.h>
#include <stdlib.h>
#include "pcb_define.h"
#include "app_sensor.h"
#include "app_console_test.h"
#include "app_rs232.h"
#include "dev_io.h"
#include "driver_digitalOut.h"
#include "utile.h"
#include "vt100_command.h"

 int32_t io_test(p_shell_context_t ctx, int32_t argc, char** argv)
 {
  char ch;
  char buff[10];
  const char *portList[10];
  uint16_t cnt;
  char *endptr;
  int pin;
  int data;
  driver_t *driver;
  if(strncmp(argv[1],"write",4)==0)
  {
    if(strncmp(argv[2],"rs232",5)==0)
    {
      cnt= rs232_get_portList(portList,_countof(portList));
      for(int i = 0 ; i < cnt;i++)
      {
        if(strncmp(argv[3],portList[i],strlen(portList[i]))==0)
        {
          if(rs232_is_opened(i)==false)
          {
            rs232_open((eRS232_PORT_t)i);

          }
          rs232_send(i,argv[4],strlen(argv[4]));
          break;
        }
      }
    }
    else if(strncmp(argv[2],"do",2)==0)
    {
      pin  = strtol(argv[3],&endptr,10);
      data = strtol(argv[4],&endptr,10);
      driver = driver_do_open(pin);
    }
  }
  else if(strncmp(argv[1],"read",4)==0)
  {
    if(strncmp(argv[2],"rs232",5)==0)
    {
      cnt= rs232_get_portList(portList,_countof(portList));
      for(int i = 0 ; i < cnt;i++)
      {
        if(strncmp(argv[3],portList[i],strlen(portList[i]))==0)
        {
          if(rs232_is_opened(i)==false)
          {
            rs232_open((eRS232_PORT_t)i);

          }
          do{
            if(rs232_recv(i,buff,1,100))
            {
              debug_printf("%c",buff[0]);
            }

             debug_recv(&ch,1,0);
          }while(ch !=ASCII_CODE_CTRL_Q);
          break;
        }
      }
    }
  }
 
 }




void print_gpio_states_in_table()
{
  GPIO_TypeDef *ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI};
  const char *port_names[] = {"GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE", "GPIOF", "GPIOG", "GPIOH", "GPIOI"};
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
        debug_printf("%2d:%-5s ", pin,"-");  // 출력 모드가 아닌 핀 표시
      }
      else
      {
        uint8_t pin_state = (port->IDR & (1 << pin)) ? 1 : 0;  // IDR에서 핀 상태 읽기
        debug_printf("%2d:%-5s ", pin, pin_state ? "HIGH" : "LOW");   // 핀 번호와 상태 출력
      }
    }
    debug_printf("\r\n");
  }
}

#include "stm32f4xx.h"  // HAL 라이브러리 헤더 포함
#include <stdio.h>

#define printf debug_printf  // 사용자 스타일에 맞춘 printf 매크로 정의


void print_gpio_output_states()
{
  GPIO_TypeDef *ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI};
  const char *port_names[] = {"GPIOA", "GPIOB", "GPIOC", "GPIOD", "GPIOE", "GPIOF", "GPIOG", "GPIOH", "GPIOI"};
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
      if ((port->MODER & (0x3 << (pin * 2))) != (0x1 << (pin * 2)))  // 출력 모드 확인 (MODER = 01)
      {
        debug_printf("%2d:%-5s ", pin,"-");  // 출력 모드가 아닌 핀 표시
      }
      else
      {
        uint8_t pin_state = (port->ODR & (1 << pin)) ? 1 : 0;  // ODR에서 핀 상태 읽기
        debug_printf("%2d:%-5s ", pin, pin_state ? "HIGH" : "LOW");   // 핀 번호와 상태 출력
      }
    }
    debug_printf("\r\n");
  }
}

  int32_t mcu_pin(p_shell_context_t ctx, int32_t argc, char** argv)
 {
  char ch;

  if(strncmp(argv[1],"di",2)==0)
  {
    debug_printf(VT100_CLEAR_SCREEN);
    debug_printf(VT100_CURSOR_OFF);
    do
    {
          debug_printf(VT100_CURSOR_HOME);
      print_gpio_states_in_table();

    debug_recv(&ch,1,100);
    }while(ch !=ASCII_CODE_CTRL_Q);

    debug_printf(VT100_CURSOR_ON);
  }
  else if(strncmp(argv[1],"do",2)==0)
  {
    print_gpio_output_states();
  }
  return 0;
 }

int32_t pcb_pin(p_shell_context_t ctx, int32_t argc, char** argv)
{
 
  for (int i = 0; i < _countof(pcbPortNameList)/2; i++)
  {
    printf("%-20s ", pcbPortNameList[i]);
  }
  printf("\r\n");


  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      printf("%-20s ", pcbPinNameList[j][i]);
    }
    printf("\r\n");
  }

  for (int i = _countof(pcbPortNameList)/2; i < _countof(pcbPortNameList); i++)
  {
    printf("%-20s ", pcbPortNameList[i]);
  }
  printf("\r\n");


  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 1; j++)
    {
      printf("%-20s ", pcbPinNameList[j][i]);
    }
    printf("\r\n");
  }

  return 0;
}