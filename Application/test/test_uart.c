#include <stdio.h>
#include <string.h>

#include "cli_key_code.h"
#include "console_utile.h"
#include "dev_io.h"
#include "driver_uart.h"

#define UART_PORT_MAX 7

void test_uart(void)
{
  uart_config_t uart_config;
  driver_t *port[UART_PORT_MAX];
  char buff[30];
  char rx_buff[10];
  const char *rs232_port_name[UART_PORT_MAX] = {"VHF", "TTL", "A", "B", "C", "D","CDMA"};
  int baud;
  int len;

  debug_printf("RS232 CDMA,TTL,A,B,C,D,CDMA 테스트\r\n");
  debug_printf("주의:RS232 A,B는 하드웨어점퍼 설정 필요\r\n");
  debug_printf("기능:1초마다 각 포트이름 전송되며 1초 대기,입력 에코처리함\r\n");
  debug_printf("통신 속도를 입력해주세요\r\n");

  if (get_int_input("통신 속도를 입력해주세요", &baud, 1200, 115200) != MENU_OK)
  {
    baud = 57600;
    debug_printf("기본 속도로 설정합니다.%d\r\n", baud);
  }
  debug_printf("이제 테스트 진행하세요 CTRL+Q 종료\r\n");


  uart_config.baud = baud;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  port[0] = driver_uart_open(UART_0_D_SUB_0, &uart_config);
  port[1] = driver_uart_open(UART_1_TTL, &uart_config);
  port[2] = driver_uart_open(UART_2_EXT_A, &uart_config);
  port[3] = driver_uart_open(UART_3_EXT_B, &uart_config);
  port[4] = driver_uart_open(UART_4_EXT_C, &uart_config);
  port[5] = driver_uart_open(UART_5_EXT_D, &uart_config);
  port[6] = driver_uart_open(UART_8_CDMA, &uart_config);

  while(1)
  {
    for (int i = 0; i < UART_PORT_MAX;i++)
    {
      snprintf(buff, sizeof(buff), "RS232 %s\r\n", rs232_port_name[i]);
      len = driver_uart_send(port[i],buff,strlen(buff));
      
      if(len < 0)
      {
        snprintf(buff, sizeof(buff), "RS232 %s error\r\n", rs232_port_name[i]);
        debug_printf(buff);
      }
      
      len = driver_uart_recv(port[i], rx_buff, sizeof(rx_buff), 1000);
      if(len)
      {
        driver_uart_send(port[i],rx_buff,len);
      }
      if (get_key(100) == KEY_CODE_CTRL_Q)
      {
        return;
      }
    }

  }
}