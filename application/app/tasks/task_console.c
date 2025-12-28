
#include <stdio.h>

#include "pcb_define.h"
#include "app_console.h"
#include "app_console_test.h"
#include "app_alarm_logging.h"
#include "app_version.h"
#include "boot_version.h"
#include "cmsis_os.h"
#include "shell.h"

#include "console_menu.h"
#include "console_login.h"
#include "drv_rs232.h"
#include "debug_io.h"
#include "task_event.h"
#include "util_time.h"


#include "console_test.h"
#include "system_err.h"
#include "task_logging.h"
#include "crypto_key.h"
#include "task_logger.h"
#include "user_heap.h"
#include "app_screen.h"

int32_t console_uart_num = -1;

static osThreadId_t s_console_task_id;
const osThreadAttr_t consoleTask_attributes = {
  .name = "console",
  .stack_size = TASK_STACK(TASK_CONSOLE_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_CONSOLE_DEF),
};


static const shell_command_context_t developCmd = {"develop", "\r\n\"develop\":develop\r\n", menu_develop,
                                                 0};

static const shell_command_context_t testCmd = {"test", "\r\n\"test\":test\r\n", test_pcb, 0};

#if 0 
void print_signature(void)
{
  uint8_t major;
  uint8_t minor;
  uint8_t fix;
  uint8_t rel;
  DATE_TIME_BUF ct;
  
  
  debug_printf(ES_CURSOR_HOME);
  //debug_printf(EC_SCROLL_REGION(20,30));
 // debug_printf(ES_CLEAR_SCREEN_BELOW);
     
    
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

  //debug_printf(EC_CURSOR_POS(20,1));   
}
#endif
void print_signature(void)
{
    uint8_t major, minor, fix, rel;
    DATE_TIME_BUF ct_app, ct_boot;
    char *buffer;
    int len = 0;
    int max_len = 2048;

    buffer = user_malloc(max_len);
    if (buffer == NULL)
    {
        return;
    }

    // 데이터 가져오기 (버퍼 구성 전 미리 획득)
    get_app_version(&major, &minor, &fix, &rel);
    get_app_build(&ct_app);
    
    // 임시 변수 재사용을 위해 boot 정보는 따로 저장하거나 출력 시점에 맞춰 호출
    uint8_t b_major, b_minor, b_fix, b_rel;
    get_boot_version(&b_major, &b_minor, &b_fix, &b_rel);
    get_boot_build(&ct_boot);

    // snprintf를 이용해 buffer에 문자열 조립
    // len += snprintf 형식을 사용하여 이전 문자열 끝에 이어서 붙임
    len += snprintf(buffer + len, max_len - len, "\r\n\r\n");
    len += snprintf(buffer + len, max_len - len, "┌──────────────────────────────────────────────┐\r\n");
    len += snprintf(buffer + len, max_len - len, "│ HWAJIN T&I CO.,LTD.                          │\r\n");
    len += snprintf(buffer + len, max_len - len, "├──────────────────────────────────────────────┤\r\n");
    len += snprintf(buffer + len, max_len - len, "│ AWS                                          │\r\n");

    len += snprintf(buffer + len, max_len - len, 
                    "│ App  %3d.%3d.%3d.%3d, %04d-%02d-%02d %02d:%02d:%02d    │\r\n",
                    major, minor, fix, rel, ct_app.Year, ct_app.Month, ct_app.Day,
                    ct_app.Hour, ct_app.Min, ct_app.Sec);

    len += snprintf(buffer + len, max_len - len, 
                    "│ Boot %3d.%3d.%3d.%3d, %04d-%02d-%02d %02d:%02d:%02d    │\r\n",
                    b_major, b_minor, b_fix, b_rel, ct_boot.Year, ct_boot.Month, ct_boot.Day,
                    ct_boot.Hour, ct_boot.Min, ct_boot.Sec);

    len += snprintf(buffer + len, max_len - len, "└──────────────────────────────────────────────┘\r\n");

    // 버퍼를 한 번에 출력
    debug_printf("%s", buffer);

    // 할당된 메모리 해제
    user_free(buffer); 
}
void dev_shell(int mode)
{


}


extern void inject_key(uint8_t data);

void consoleTask(void *arg)
{
  char buffer[50];
  char login_key[10];

  print_signature();
  
  read_password(login_key);
  while(check_login(login_key)==false);

  if(read_last_error(buffer,sizeof(buffer)))
  {
    debug_printf("Last error:%s\r\n",buffer);
  }
  
  debug_printf("Alarm log count:%d\r\n",alarm_get_log_count());

  shell_scanf_init();

  aws_menu();
  

}


void testColsoleTask(void *arg)
{
  shell_context_struct user_context;

  log_write(L_ERROR,"testColsoleTask");

  shell_scanf_init();
    
    
  shell_init(&user_context,   debug_printf,(char *)"\x1B[32mAWS_TEST>> \x1B[37m");
  shell_register_command(&testCmd);
  shell_register_command(&developCmd);
  shell_loop(&user_context);

  debug_puts("Debug menu exited\r\n");

  
}


//디버깅 포트 초기화 안된 상태에서 실행되면 recv가 계속 리턴되는 문제 발생
void start_console(void *arg)
{

  debug_init();

  if(s_console_task_id == NULL)
  {
    s_console_task_id = osThreadNew(consoleTask, arg, &consoleTask_attributes);
  }


}

void consoleTask_init(void *arg)
{

 
       debug_init() ;
  if((int)arg == 1)//버튼 눌린상태로진입 test 모드 실행
  {
    osThreadNew(testColsoleTask, arg, &consoleTask_attributes);
  }
  else
  { 
    s_console_task_id = osThreadNew(consoleTask, arg, &consoleTask_attributes);
  }
  
}


