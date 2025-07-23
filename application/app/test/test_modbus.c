#include <stdio.h>
#include <string.h>

#include "cli_key_code.h"
#include "dev_io.h"
#include "drv_rs485.h"
#include "console_utile.h"
#include "modbus_master.h"
#include "bsp_rs485.h"
#include "drv_rs232.h"
#define RS485_PORT_MAX 4

#define READ_REG_CNT 15






#if 0
void test_modbus_task(void *arg)
{
  uint16_t reg[READ_REG_CNT];
  uart_config_t uart_config;
  int len;
  int32_t selected_port;
  int port_index;
  char buff[100];
  char rx_buff[50];
  int baud = 57600;
  const char *rs485_port_name[RS485_PORT_MAX] = {"A", "B", "C", "D"};
  int32_t port_list[RS485_PORT_MAX] = {RS485_A, RS485_B, RS485_RS232_C, RS485_RS232_D};
  modbus_h_t modbus;
  int32_t ret;
  int32_t reg_number=0x3100;

     io_printf("MODBUS RS485 RTU테스트\r\n");
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

  if (input_decimal_prompt("모드버스  주소", &reg_number, 0, 65535) != MENU_OK)
  {
    reg_number = 0;
    io_printf("주소 오류 종료\r\n");
    return ;
  }

  selected_port = port_list[port_index];
  
  uart_config.baud = baud;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  drv_rs485_init(selected_port, &uart_config);

  modbus.id = 1;
  modbus.modebus_type = eMODBUS_RS485;
  modbus.name ="test";
  modbus.port_num = selected_port;
  modbus.sem = 0;

  io_printf("모드버스 RS485 포트 %s, 속도 %d로 테스트 시작 (CTRL+Q 종료)\r\n",
                rs485_port_name[port_index], baud);

  while(1)
  {
    snprintf(buff, sizeof(buff), "RS485 %s\r\n", rs485_port_name[port_index]);

    ret = modbus_read_input_reg(&modbus, 1, reg_number, reg, READ_REG_CNT);

    if (ret)
    {
      io_printf("modbus err:%d\r\n",ret);
    }
    else
    {
        io_printf("수신 및 에코 (%d bytes): ", len);
        for (int i = 0; i < READ_REG_CNT; i++)
        {
          io_printf("%04X ", reg[i]);
        }
        io_printf("\r\n");

    }
    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      io_printf("테스트 종료\r\n");
      return;
    }
  }
}
#include "cmsis_os2.h"
const osThreadAttr_t ktestmodbusTask_attributes = {
    .name = "modbustestTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};
#endif

#define MODBUS_TEMP 0
#define MODBUS_CHARGER 1
void test_modbus_task(void *arg)
{
  uint16_t reg[READ_REG_CNT];
  uart_config_t uart_config;
  int len;
  int32_t selected_port;
  int port_index;
  char buff[100];
  char rx_buff[50];
  int baud = 57600;
  const char *rs485_port_name[RS485_PORT_MAX] = {"A", "B", "C", "D"};
  int32_t port_list[RS485_PORT_MAX] = {BSP_RS485_A, BSP_RS485_B, BSP_RS485_RS232_C, BSP_RS485_RS232_D};
  modbus_h_t modbus;
  int32_t ret;
  int32_t reg_number = 0x3100;
  int32_t channel = (int)arg;
  int32_t read_cnt;

  switch (channel)
  {
  case MODBUS_CHARGER:
    selected_port = BSP_RS485_B;
    baud =115200;
    reg_number = 0x3100;
    read_cnt = 15;
    break;
  case MODBUS_TEMP:
    selected_port = BSP_RS485_RS232_D ;
    baud = 9600;
    reg_number = 0x0;
    read_cnt =1;
    break;
  default:
    break;
  }



  uart_config.baud = baud;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  drv_rs485_init(selected_port, &uart_config);

  modbus.id = 1;
  modbus.modebus_type = eMODBUS_RS485;
  modbus.name = "test";
  modbus.port_num = selected_port;
  modbus.sem = 0;

  while (1)
  {

    if(channel==MODBUS_CHARGER)
    {
      io_printf("charger\r\n");

      ret = modbus_read_input_reg(&modbus, reg_number, reg, read_cnt);
    }
    else{
      io_printf("temperature\r\n");
      ret = modbus_read_hold_reg(&modbus,  reg_number, reg, read_cnt);
    }




    if (ret)
    {
      io_printf("modbus err:%d\r\n", ret);
    }
    else
    {
      io_printf("수신 및 에코 (%d bytes): ", read_cnt);
      for (int i = 0; i < read_cnt; i++)
      {
        io_printf("%04X ", reg[i]);
      }
      io_printf("\r\n");
    }
    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      io_printf("테스트 종료\r\n");
      return;
    }
  }
}
#include "cmsis_os2.h"
const osThreadAttr_t ktestmodbusTask_attributes = {
    .name = "modbustestTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};
void test_modbus(void)
{

  osThreadNew(test_modbus_task, (void *)MODBUS_TEMP, &ktestmodbusTask_attributes);
  //osThreadNew(test_modbus_task, (void *)MODBUS_CHARGER, &ktestmodbusTask_attributes);
  while(1);
}