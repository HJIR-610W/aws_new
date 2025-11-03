
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
#include "drv_rs232.h"
#include "dev_io.h"
#include "task_isrEvent.h"
#include "util_time.h"
#include "cli_input.h"
#include "vt100_command.h"
#include "console_test.h"
#include "system_err.h"
#include "dev_io.h"

int32_t console_uart_num = -1;

 osThreadId_t s_console_task_id;
const osThreadAttr_t consoleTask_attributes = {
  .name = "consoleTask",
  .stack_size = TASK_STACK(TASK_CONSOLE_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_CONSOLE_DEF),
};


static const shell_command_context_t printCmd = { "menu",
                                                  "\r\n\"menu\":menu\r\n" ,
                                                   menu_root,0 };

static const shell_command_context_t developCmd = {"develop", "\r\n\"develop\":develop\r\n", menu_develop,
                                                 0};

static const shell_command_context_t testCmd = {"test", "\r\n\"test\":test\r\n", test_pcb, 0};

void print_signature(void)
{
  uint8_t major;
  uint8_t minor;
  uint8_t fix;
  uint8_t rel;
  DATE_TIME_BUF ct;

  get_app_version(&major,&minor,&fix,&rel);
  get_app_build(&ct);

  io_printf("\r\n");


io_printf("┌──────────────────────────────────────────────┐\r\n");
io_printf("│ HWAJIN T&I CO.,LTD.                          │\r\n");
io_printf("├──────────────────────────────────────────────┤\r\n");
io_printf("│ AWS                                          │\r\n"); 
io_printf("│ App  %3d.%3d.%3d.%3d, %04d-%02d-%02d %02d:%02d:%02d    │\r\n",
          major, minor, fix, rel, ct.Year, ct.Month, ct.Day,
          ct.Hour, ct.Min, ct.Sec);

get_boot_version(&major, &minor, &fix, &rel);
get_boot_build(&ct);

io_printf("│ Boot %3d.%3d.%3d.%3d, %04d-%02d-%02d %02d:%02d:%02d    │\r\n",
          major, minor, fix, rel, ct.Year, ct.Month, ct.Day,
          ct.Hour, ct.Min, ct.Sec);

io_printf("└──────────────────────────────────────────────┘\r\n");


}


void SHELL_SendDataCallback(uint8_t* buf, uint32_t len)
{
  io_send(buf,len);
}

void SHELL_ReceiveDataCallback(uint8_t* buf, uint32_t len)
{
    drv_uart_get_char(console_uart_num, buf, len);
}


char g_buffer[]={"321테스트중입니다.테스트중입니다.테스트중입니다.테스트중입니다.테스트중입니다.테스트중입니다.\r\n"};
void test_console(void)
{
  for(int i = 0;i< sizeof(g_buffer);i++)
  {
   // g_buffer[i] = i+'!';
  }
  while(1)
  {

    drv_uart_send(console_uart_num,(uint8_t *)g_buffer,strlen(g_buffer));
    osDelay(10);
    
  }
}


void consoleTask(void *arg)
{
  shell_context_struct user_context;
  uint8_t instance = 0;


  int mode = (int)arg;
  char buffer[100];
  const char *cli_aws = "\x1B[32mAWS>> \x1B[37m";
  const char *cli_test = "\x1B[32mAWS_TEST>> \x1B[37m";

 //test_console();
  osDelay(1000);
  io_printf("\r\n\r\n");

  if (restore_error(buffer,sizeof(buffer)))
  {
    io_printf("%s\r\n",buffer);
  }

    // io_printf(VT100_CLEAR_SCREEN);
    // io_printf(VT100_CURSOR_HOME);
    print_signature();

  DbgConsole_Init(instance, 0, DEBUG_CONSOLE_DEVICE_TYPE_RS232, 0);

  if(mode==0)
  {
    SHELL_Init(&user_context, SHELL_SendDataCallback, SHELL_ReceiveDataCallback, io_printf,
               (char *)cli_aws);
  }
  else
  {
    SHELL_Init(&user_context, SHELL_SendDataCallback, SHELL_ReceiveDataCallback, io_printf,
               (char *)cli_test);
  }
  console_scanf_init(&user_context);

  SHELL_RegisterCommand(&printCmd);
  SHELL_RegisterCommand(&testCmd);
  SHELL_RegisterCommand(&developCmd);
  SHELL_Main(&user_context);

  while(1)
  {
    drv_uart_send(console_uart_num,(uint8_t *)g_buffer,sizeof(g_buffer));
    osDelay(1000);
  }
}

void start_console(void *arg)
{
  if (s_console_task_id == NULL)
  {
    s_console_task_id = osThreadNew(consoleTask, arg, &consoleTask_attributes);
  }

  if(s_console_task_id)
  {
    set_debug_uart_handle(BSP_UART_10_CDC);
  }
}

void consoleTask_init(void *arg)
{
  int32_t result;
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  uart_config.baud = 115200;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;

  console_uart_num = BSP_UART_10_CDC  ;
  
    
  result = drv_uart_init(console_uart_num, &uart_config);
   
  if(result > 0)                                                                                
  {
    set_debug_uart_handle(console_uart_num);
    start_console(arg);
  }
}


void stop_console(void)
{
  set_debug_uart_handle(-1);
}

void consoleTask_stop(void)
{
  
}