
#include "app_console.h"
#include "app_version.h"
#include "boot_version.h"
#include "cmsis_os.h"
#include "cli\fsl_shell.h"
#include "cli\fsl_debug_console.h"
#include "cli\console_scanf.h"
#include "driver_uart.h"
#include "dev_io.h"
#include "task_isrEvent.h"
#include "utile_time.h"


driver_t *console_uart;

const osThreadAttr_t consoleTask_attributes = {
  .name = "consoleTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityLow,
};


static const shell_command_context_t printCmd = { "menu",
                                                  "\r\n\"menu\"\r\n" ,
                                                   menu_root,0 };






void print_signature(void)
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    DATE_TIME_BUF ct;

    get_appVer(&a,&b,&c,&d);
    get_appBuild(&ct);

    debug_printf("\r\n");
    debug_printf("(0lqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqk(B\r\n");
    debug_printf("(0x(B HWAJIN T&I CO.,LTD.                      (0x(B\r\n");
    debug_printf("(0tqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqu(B\r\n");
    debug_printf("(0x(B AWS                                      (0x(B\r\n"); //1111-11-11 11:11:11
    debug_printf("(0x(B App  %3d.%3d.%3d.%3d,%04d-%02d-%02d %02d:%02d:%02d (0x(B\r\n",a,b,c,d,ct.Year,ct.Month,ct.Day,
                                                    ct.Hour,ct.Min,ct.Sec);//os »ç¿ëÀü¿¡´Â Á÷Á¢ È£Ãâ
    get_bootVer(&a,&b,&c,&d);
    get_bootBuild(&ct);
    debug_printf("(0x(B Boot %3d.%3d.%3d.%3d,%04d-%02d-%02d %02d:%02d:%02d (0x(B\r\n",a,b,c,d,ct.Year,ct.Month,ct.Day,
                                                    ct.Hour,ct.Min,ct.Sec);//os »ç¿ëÀü¿¡´Â Á÷Á¢ È£Ãâ

    debug_printf("(0mqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqj(B\r\n");

}


void SHELL_SendDataCallback(uint8_t* buf, uint32_t len)
{
  driver_uart_send(console_uart, buf, len);
}

void SHELL_ReceiveDataCallback(uint8_t* buf, uint32_t len)
{
    driver_uart_get_char(console_uart, buf, len);
}

void sonsoleTask(void *arg)
{

  
  shell_context_struct user_context;
   uint8_t instance = 0;
    
  console_uart = driver_uart_open(UART_STM32_3);

  set_debug_uart_handle(console_uart);


  print_signature();


  DbgConsole_Init(instance, 0, DEBUG_CONSOLE_DEVICE_TYPE_RS232, 0);

  SHELL_Init(&user_context, SHELL_SendDataCallback, SHELL_ReceiveDataCallback, debug_printf, "AWS>> ");
  console_scanf_init(&user_context);

  SHELL_RegisterCommand(&printCmd);
  SHELL_Main(&user_context);

  while(1)
  {
    osDelay(1000);
  }
}

void consoleTask_init(void)
{
  osThreadNew(sonsoleTask, NULL, &consoleTask_attributes);
}