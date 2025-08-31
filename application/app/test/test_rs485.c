#include <stdio.h>
#include <string.h>

#include "cli_key_code.h"
#include "dev_io.h"
#include "bsp_rs485.h"
#include "console_utile.h"

#define RS485_PORT_MAX 4

void test_rs485(void)
{
  uart_config_t uart_config;
  int len;
  int32_t selected_port;
  int port_index;
  char buff[100];
  char rx_buff[50];
  int baud = 57600;
  const char *rs485_port_name[RS485_PORT_MAX] = {"A", "B", "C", "D"};
  int32_t port_list[RS485_PORT_MAX] = {BSP_RS485_A, BSP_RS485_B, BSP_RS485_RS232_C, BSP_RS485_RS232_D};

  io_printf("RS485 포트별 테스트\r\n");
  io_printf("주의: RS485 C,D는 하드웨어점퍼 설정 필요\r\n");
  io_printf("기능: 포트이름 전송 후 1초간 수신 데이터 에코 및 HEX 출력\r\n\r\n");

  if(input_decimal_prompt("테스트할 포트를 선택하세요 (0:A, 1:B, 2:C, 3:D)", &port_index, 0, 3) != MENU_OK)
  {
    port_index = 0;
    io_printf("기본 포트 A로 설정합니다.\r\n");
  }

  if(input_decimal_prompt("통신 속도를 입력해주세요", &baud, 1200, 115200) != MENU_OK)
  {
    baud = 57600;
    io_printf("기본 속도 %d로 설정합니다.\r\n", baud);
  }

  selected_port = port_list[port_index];
  
  uart_config.baud = baud;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  bsp_rs485_init(selected_port, &uart_config);
  
  io_printf("RS485 포트 %s, 속도 %d로 테스트 시작 (CTRL+Q 종료)\r\n", 
            rs485_port_name[port_index], baud);

  while(1)
  {
    snprintf(buff, sizeof(buff), "RS485 %s\r\n", rs485_port_name[port_index]);
    len = bsp_rs485_send(selected_port, (uint8_t*)buff, strlen(buff));
    
    if(len < 0)
    {
      io_printf("RS485 %s 전송 실패\r\n", rs485_port_name[port_index]);
    }
    else
    {
      io_printf("전송: %s", buff);
    }

    len = bsp_rs485_recv(selected_port, (uint8_t*)rx_buff, sizeof(rx_buff), 1000);
    
    if(len > 0)
    {
      bsp_rs485_send(selected_port, (uint8_t*)rx_buff, len);
      
      io_printf("수신 및 에코 (%d bytes): ", len);
      for(int i = 0; i < len; i++)
      {
        io_printf("%c", rx_buff[i]);
      }
      io_printf("\r\n");
    }

    if (get_key(10) == KEY_CODE_CTRL_C)
    {
      io_printf("테스트 종료\r\n");
      return;
    }
  }
}