
#include <stdio.h>

#include "pcb_define.h"
#include "app_console.h"
#include "app_console_test.h"
#include "app_alarm_logging.h"
#include "app_version.h"
#include "boot_version.h"
#include "cmsis_os.h"
#include "cli\shell.h"

 
#include "console_login.h"
#include "drv_rs232.h"
#include "debug_io.h"
#include "task_event.h"
#include "util_time.h"
 
 
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

  debug_printf("\r\n\r\n");

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



void consoleTask(void *arg)
{
  char buffer[100];
  const char *prompt=NULL;
  shell_context_struct user_context;
  uint8_t instance = 0;
  int mode = (int)arg;


  print_signature();
  check_login();
  
  if(read_last_error(buffer,sizeof(buffer)))
  {
    debug_printf("Last error:%s\r\n",buffer);
  }
  
  debug_printf("Alarm log count:%d\r\n",alarm_get_log_count());

  
  prompt = (mode==0)?"\x1B[32mAWS>> \x1B[37m":"\x1B[32mAWS_TEST>> \x1B[37m";

  SHELL_Init(&user_context, debug_send, debug_recv, debug_printf,(char *)prompt);

  shell_scanf_init();

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