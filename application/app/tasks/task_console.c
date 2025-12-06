
#include <stdio.h>

#include "pcb_define.h"
#include "app_console.h"
#include "app_console_test.h"
#include "app_alarm_logging.h"
#include "app_version.h"
#include "boot_version.h"
#include "cmsis_os.h"
#include "cli\fsl_shell.h"
#include "cli\fsl_debug_console.h"
#include "cli\console_scanf.h"
#include "console_login.h"
#include "drv_rs232.h"
#include "debug_io.h"
#include "task_event.h"
#include "util_time.h"
#include "cli_input.h"
#include "vt100_command.h"
#include "console_test.h"
#include "system_err.h"


int32_t console_uart_num = -1;

static osThreadId_t s_console_task_id;
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

  debug_printf("\r\n");

  debug_printf("┌──────────────────────────────────────────────┐\r\n");
  debug_printf("│ HWAJIN T&I CO.,LTD.                          │\r\n");
  debug_printf("├──────────────────────────────────────────────┤\r\n");
  debug_printf("│ AWS                                          │\r\n"); 

  get_app_version(&major,&minor,&fix,&rel);
  get_app_build(&ct);

  debug_printf("│ App  %3d.%3d.%3d.%3d, %04d-%02d-%02d %02d:%02d:%02d    │\r\n",
          major, minor, fix, rel, ct.Year, ct.Month, ct.Day,
          ct.Hour, ct.Min, ct.Sec);

  get_boot_version(&major, &minor, &fix, &rel);
  get_boot_build(&ct);

  debug_printf("│ Boot %3d.%3d.%3d.%3d, %04d-%02d-%02d %02d:%02d:%02d    │\r\n",
          major, minor, fix, rel, ct.Year, ct.Month, ct.Day,
          ct.Hour, ct.Min, ct.Sec);

  debug_printf("└──────────────────────────────────────────────┘\r\n");


}


void SHELL_SendDataCallback(uint8_t* buf, uint32_t len)
{
  debug_send(buf,len);
}

void SHELL_ReceiveDataCallback(uint8_t* buffer, uint32_t len)
{
  debug_recv(buffer,len,osWaitForever);
}


void consoleTask(void *arg)
{
  shell_context_struct user_context;
  uint8_t instance = 0;
  int mode = (int)arg;
  char buffer[100];
  const char *cli_aws = "\x1B[32mAWS>> \x1B[37m";
  const char *cli_test = "\x1B[32mAWS_TEST>> \x1B[37m";

  osDelay(1000);

  debug_printf("\r\n\r\n");
  // debug_printf(VT100_CLEAR_SCREEN);
  // debug_printf(VT100_CURSOR_HOME);
  print_signature();

  check_login();
  

  if (read_last_error(buffer,sizeof(buffer)))
  {
    debug_printf("%s\r\n",buffer);
  }
  
  debug_printf("alarm log count:%d\r\n",alarm_get_log_count());
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
  SHELL_RegisterCommand(&testCmd);
  SHELL_RegisterCommand(&developCmd);
  SHELL_Main(&user_context);


  while(1)
  {
    debug_puts("Debug menu exited\r\n");
    osDelay(1000);
  }
}

void start_console(void *arg)
{
  if (s_console_task_id == NULL)
  {
    s_console_task_id = osThreadNew(consoleTask, arg, &consoleTask_attributes);
  }


}

void consoleTask_init(void *arg)
{
  debug_init();

  s_console_task_id = osThreadNew(consoleTask, arg, &consoleTask_attributes);
}



void consoleTask_stop(void)
{
  
}