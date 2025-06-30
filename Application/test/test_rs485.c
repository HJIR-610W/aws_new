#include <stdio.h>
#include <string.h>

#include "cli_key_code.h"
#include "dev_io.h"
#include "driver_485.h"
#include "console_utile.h"

#define RS485_PORT_MAX 4

void test_rs485(void)
{
  uart_config_t uart_config;
  int len;
  driver_t *port[RS485_PORT_MAX];
  char buff[30];
  char rx_buff[50];
  int baud=57600;
  const char *rs485_port_name[RS485_PORT_MAX]={"A","B","C","D"};


  io_printf("RS485 A,B,C,D 테스트\r\n");
  io_printf("주의:RS485 C,D는 하드웨어점퍼 설정 필요\r\n");
  io_printf("기능:1초마다 각 포트이름 전송되며 1초 대기,입력 에코처리함\r\n");

  io_printf("통신 속도를 입력해주세요\r\n");

  if(input_decimal_prompt("통신 속도를 입력해주세요", &baud, 1200, 115200)!= MENU_OK)
  {
    baud=57600;
    io_printf("기본 속도로 설정합니다.%d\r\n", baud);
  }
  io_printf("이제 테스트 진행하세요 CTRL+Q 종료\r\n");

  uart_config.baud = baud;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  port[0] = driver_rs485_open(RS485_A, &uart_config);
  port[1] = driver_rs485_open(RS485_B, &uart_config);
  port[2] = driver_rs485_open(RS485_C, &uart_config);
  port[3] = driver_rs485_open(RS485_D, &uart_config);
  
  while(1)
  {
    for (int i = 0; i < RS485_PORT_MAX; i++)
    {
      snprintf(buff,sizeof(buff),"RS485 %s\r\n",rs485_port_name[i]);
      len = driver_rs485_send(port[i], (uint8_t*)buff, strlen(buff));
      if(len<0)
      {
        snprintf(buff, sizeof(buff), "RS485 %s failed\r\n", rs485_port_name[i]);
        io_printf(buff);
      }
      len = driver_rs485_recv(port[i], (uint8_t*)rx_buff, sizeof(rx_buff), 2000);
      if(len)
      {
        driver_rs485_send(port[i], (uint8_t*)rx_buff, len);
      }
      if (get_key(100) == KEY_CODE_CTRL_Q)
      {
        return;
      }
    }

  }
}