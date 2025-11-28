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
  uart_config.parity_index = PARITY_NONE;
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
    if (get_key(1000) == KEY_CODE_CTRL_C)
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
void test_modbus_task(void)
{
  uint16_t reg[READ_REG_CNT];
  uart_config_t uart_config;
  int32_t selected_port;
  int port_index;
  int interface_type;
  int baud = 57600;
  int parity = PARITY_NONE;
  const char *rs485_port_name[RS485_PORT_MAX] = {"A", "B", "C", "D"};
  int32_t rs485_port_list[RS485_PORT_MAX] = {BSP_RS485_C, BSP_RS485_D, BSP_RS485_RS232_A, BSP_RS485_RS232_B};
  const char *rs232_port_name[4] = {"A", "B", "C", "D"};
  int32_t rs232_port_list[4] = {DRV_UART_2_EXT_A, DRV_UART_3_EXT_B, DRV_UART_4_EXT_C, DRV_UART_5_EXT_D};
  const char *parity_name[] = {"NONE", "ODD", "EVEN"};
  const char *interface_name[] = {"RS232", "RS485"};
  modbus_h_t modbus;
  int32_t ret;
  int32_t reg_number;
  int32_t read_cnt;
  int32_t modbus_id = 1;
  int32_t reg_type;

  io_printf("MODBUS RTU 테스트\r\n");
  io_printf("기능: Modbus Read Registers\r\n\r\n");

  if(input_decimal_prompt("인터페이스를 선택하세요 (0:RS232, 1:RS485)", &interface_type, 0, 1) != MENU_OK)
  {
    interface_type = 1;
    io_printf("기본 RS485로 설정합니다.\r\n");
  }

  if(interface_type == 0)
  {
    io_printf("RS232 포트 선택 (사용자 포트만 지원)\r\n");
    if(input_decimal_prompt("테스트할 포트를 선택하세요 (0:A, 1:B, 2:C, 3:D)", &port_index, 0, 3) != MENU_OK)
    {
      port_index = 0;
      io_printf("기본 포트 A로 설정합니다.\r\n");
    }
    selected_port = rs232_port_list[port_index];
  }
  else
  {
    io_printf("RS485 포트 선택\r\n");
    io_printf("주의: RS485 C,D는 하드웨어 점퍼 설정 필요\r\n");
    if(input_decimal_prompt("테스트할 포트를 선택하세요 (0:A, 1:B, 2:C, 3:D)", &port_index, 0, 3) != MENU_OK)
    {
      port_index = 0;
      io_printf("기본 포트 A로 설정합니다.\r\n");
    }
    selected_port = rs485_port_list[port_index];
  }

  if(input_decimal_prompt("통신 속도를 입력해주세요 (1200-115200)", &baud, 1200, 115200) != MENU_OK)
  {
    baud = 57600;
    io_printf("기본 속도 %d로 설정합니다.\r\n", baud);
  }

  if(input_decimal_prompt("패리티를 선택하세요 (0:NONE, 1:ODD, 2:EVEN)", &parity, 0, 2) != MENU_OK)
  {
    parity = PARITY_NONE;
    io_printf("기본 패리티 NONE으로 설정합니다.\r\n");
  }

  if(input_decimal_prompt("Modbus 슬레이브 ID를 입력하세요 (1-247)", &modbus_id, 1, 247) != MENU_OK)
  {
    modbus_id = 1;
    io_printf("기본 ID 1로 설정합니다.\r\n");
  }


  uart_config.baud = baud;
  uart_config.parity_index = parity;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  if(interface_type == 0)
  {
    drv_uart_init(selected_port, &uart_config,"Modbus");
    modbus.modebus_type = eMODBUS_RS232;
  }
  else
  {
    drv_rs485_init(selected_port, &uart_config,"Modbus");
    modbus.modebus_type = eMODBUS_RS485;
  }

  modbus.id = modbus_id;
  modbus.name = "test";
  modbus.port_num = selected_port;
  modbus.sem = 0;

  io_printf("Modbus %s 설정:\r\n", interface_name[interface_type]);
  if(interface_type == 0)
    io_printf("  포트: %s (사용자%d)\r\n", rs232_port_name[port_index], port_index);
  else
    io_printf("  포트: %s\r\n", rs485_port_name[port_index]);
  io_printf("  속도: %d bps\r\n", baud);
  io_printf("  패리티: %s\r\n", parity_name[parity]);
  io_printf("  슬레이브 ID: %d\r\n", modbus_id);
  io_printf("테스트 시작 (CTRL+Q로 종료)\r\n\r\n");

  while (1)
  {
    io_printf("Modbus 기능 선택:\r\n");
    io_printf("  0: Read Coils (0x01)\r\n");
    io_printf("  1: Read Discrete Inputs (0x02)\r\n");
    io_printf("  2: Read Input Registers (0x04)\r\n");
    io_printf("  3: Read Holding Registers (0x03)\r\n");
    
    if (input_decimal_prompt("기능을 선택하세요", &reg_type, 0, 3) != MENU_OK)
    {
      io_printf("테스트 종료\r\n");
      return;
    }

    if (input_decimal_prompt("Modbus 시작 주소 (0-65535)", &reg_number, 0, 65535) != MENU_OK)
    {
      io_printf("테스트 종료\r\n");
      return;
    }

    if(reg_type == 0)
    {
      uint16_t coil_result;
      io_printf("Single Coil 읽기 (0x01) - 주소: 0x%04X (%d)\r\n", reg_number, reg_number);
      ret = modbus_read_single_coil(&modbus, reg_number, &coil_result);
      
      if (ret)
      {
        io_printf("Modbus 오류: %d\r\n", ret);
      }
      else
      {
        io_printf("Coil 읽기 성공:\r\n");
        io_printf("  주소 0x%04X (%d): %s (%d)\r\n", 
                  reg_number, reg_number, coil_result ? "ON" : "OFF", coil_result);
      }
    }
    else if(reg_type == 1)
    {
      if (input_decimal_prompt("읽을 Discrete Input 개수 (1-15)", &read_cnt, 1, 15) != MENU_OK)
      {
        io_printf("테스트 종료\r\n");
        return;
      }
      
      io_printf("Discrete Inputs 읽기 (0x02) - 주소: 0x%04X (%d), 개수: %d\r\n", reg_number, reg_number, read_cnt);
      ret = modbus_read_discrete_inputs(&modbus, reg_number, reg, read_cnt);
      
      if (ret)
      {
        io_printf("Modbus 오류: %d\r\n", ret);
      }
      else
      {
        io_printf("Discrete Inputs 읽기 성공:\r\n");
        for (int i = 0; i < read_cnt; i++)
        {
          io_printf("  주소 0x%04X (%d): %s (0x%04X)\r\n", 
                    reg_number + i, reg_number + i, (reg[i] & 0x01) ? "ON" : "OFF", reg[i]);
        }
      }
    }
    else if(reg_type == 2)
    {
      if (input_decimal_prompt("읽을 Input Register 개수 (1-15)", &read_cnt, 1, 15) != MENU_OK)
      {
        io_printf("테스트 종료\r\n");
        return;
      }
      
      io_printf("Input Registers 읽기 (0x04) - 주소: 0x%04X (%d), 개수: %d\r\n", reg_number, reg_number, read_cnt);
      ret = modbus_read_input_reg(&modbus, reg_number, reg, read_cnt);
      
      if (ret)
      {
        io_printf("Modbus 오류: %d\r\n", ret);
      }
      else
      {
        io_printf("Input 레지스터 읽기 성공:\r\n");
        for (int i = 0; i < read_cnt; i++)
        {
          io_printf("  주소 0x%04X (%d): 0x%04X (%d)\r\n", 
                    reg_number + i, reg_number + i, reg[i], reg[i]);
        }
      }
    }
    else
    {
      if (input_decimal_prompt("읽을 Holding Register 개수 (1-15)", &read_cnt, 1, 15) != MENU_OK)
      {
        io_printf("테스트 종료\r\n");
        return;
      }
      
      io_printf("Holding Registers 읽기 (0x03) - 주소: 0x%04X (%d), 개수: %d\r\n", reg_number, reg_number, read_cnt);
      ret = modbus_read_hold_reg(&modbus, reg_number, reg, read_cnt);
      
      if (ret)
      {
        io_printf("Modbus 오류: %d\r\n", ret);
      }
      else
      {
        io_printf("Holding 레지스터 읽기 성공:\r\n");
        for (int i = 0; i < read_cnt; i++)
        {
          io_printf("  주소 0x%04X (%d): 0x%04X (%d)\r\n", 
                    reg_number + i, reg_number + i, reg[i], reg[i]);
        }
      }
    }
    
    io_printf("\r\n");
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
  test_modbus_task();

 // osThreadNew(test_modbus_task, (void *)MODBUS_TEMP, &ktestmodbusTask_attributes);
  //osThreadNew(test_modbus_task, (void *)MODBUS_CHARGER, &ktestmodbusTask_attributes);
//  while(1);
}