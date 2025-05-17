

#include "panel_aws_std.h"
#include "panel_mooju.h"
#include "panel_hansung.h"

#include "driver_uart.h"
#include "config_app.h"

driver_t *g_panel_uart;

void panel_init(void)
{

  uart_config_t uart_config;
  
  uart_config.baud = 115200;
  uart_config.dataLen = UART_DATA_LEN_8;
  uart_config.parityIdx = 0;
  uart_config.stop_bit = 0;

  g_panel_uart = driver_uart_open(UART_3_EXT_B,&uart_config);
}


void send_panel(void)
{
  uint16_t len;
  uint8_t frame[100];
  switch (get_config_app()->panel_model)
  {
    case ePANEL_STD:
    send_panel_std(g_panel_uart);
    break;
  case ePANEL_HANSUNG:
  	send_panel_hansung(g_panel_uart);
    break;
  case ePANEL_MUJU:
    send_panel_muju(g_panel_uart);
  break;

  }


}