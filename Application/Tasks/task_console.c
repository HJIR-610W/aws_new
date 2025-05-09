
#include <stdio.h>

#include "pcb_define.h"
#include "app_console.h"
#include "app_console_test.h"
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
#include "cli_input.h"
#include "vt100_command.h"
#include "console_test.h"
driver_t *console_uart;

const osThreadAttr_t consoleTask_attributes = {
  .name = "consoleTask",
  .stack_size = 2048+1024,
  .priority = (osPriority_t) osPriorityBelowNormal,
};


static const shell_command_context_t printCmd = { "menu",
                                                  "\r\n\"menu\"\r\n" ,
                                                   menu_root,0 };



 const shell_command_context_t ioCmd = {"io",
                                        "\r\n\"io arg1 arg2 arg3 arg4\"\r\n"
                                        "arg1: write|read|wr\r\n"
                                        "arg2: rs232|rs485|do|di|eth\r\n"
                                        "arg3: dest\r\n"
                                        "arg4: data\r\n",
                                         io_test, 4};

static const shell_command_context_t diCmd = {"di",
                                              "\r\n\"di\"\r\n",
                                              print_di, 0};

static const shell_command_context_t pcbCmd = { "pcb",
                                                  "\r\n\"pcb\"\r\n" ,
                                                   pcb_pin,0 };
static const shell_command_context_t doutCmd = {"do",
                                        "\r\n\"do arg1 arg2\"\r\n"
                                        "arg1: pin\r\n"
                                        "arg2: 0|1\r\n",
                                         ctrl_do, 2};

static const shell_command_context_t testCmd = {"test", "\r\n\"test\"\r\n", test_pcb, 0};

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
  int a;
  int ret;
  int mode = (int)arg;

  const char *cli_aws = "\x1B[32mAWS>> \x1B[37m";
  const char *cli_test = "\x1B[32mAWS_TEST>> \x1B[37m";

  osDelay(1000);
  debug_printf("\r\n");
  debug_printf(VT100_CLEAR_SCREEN);
  debug_printf(VT100_CURSOR_HOME);
  print_signature();

  DbgConsole_Init(instance, 0, DEBUG_CONSOLE_DEVICE_TYPE_RS232, 0);

  if(mode==0)
  {
    SHELL_Init(&user_context, SHELL_SendDataCallback, SHELL_ReceiveDataCallback, debug_printf,
               (char *)cli_aws);
  }
  else
  {
    SHELL_Init(&user_context, SHELL_SendDataCallback, SHELL_ReceiveDataCallback, debug_printf,
               (char *)cli_test);
  }
  console_scanf_init(&user_context);

  SHELL_RegisterCommand(&printCmd);
  SHELL_RegisterCommand(&ioCmd);
  SHELL_RegisterCommand(&diCmd);
  SHELL_RegisterCommand(&pcbCmd);
  SHELL_RegisterCommand(&doutCmd);
  SHELL_RegisterCommand(&testCmd);
  SHELL_Main(&user_context);

  while(1)
  {
    osDelay(1000);
  }
}



void consoleTask_init(void *arg)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  uart_config.baud = 115200;
  uart_config.parityIdx = 0;
  uart_config.stop_bit = 0;

  console_uart = driver_uart_open(UART_10_CDC,&uart_config);
  //console_uart = driver_uart_open(UART_0_D_SUB_0,&uart_config);

  osDelay(100);

  set_debug_uart_handle(console_uart);
  osThreadNew(sonsoleTask, arg, &consoleTask_attributes);
}