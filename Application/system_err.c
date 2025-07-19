
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dev_io.h"
#include "system_err.h"
#include "pcb_define.h"
#include "util_time.h"
#include "vt100_command.h"


void Error_Handler(const char *file,const int32_t line)
{
  io_printf("%s,%d\r\n",file,line);

}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  Error_Handler((const char *)file,line);
}
#endif /* USE_FULL_ASSERT */


#define RST_LOG_MAX 100
//2022-11-11 11:00:00,test
typedef struct no_init_s
{
  uint32_t key;
  char rstLog[RST_LOG_MAX];
}no_init_t;
__no_init volatile no_init_t noInitData @ 0x20000004; //이 주소에 할당되도록 한다 IAR 전용



void reset_system(const char * pFmt, ...)
{
  char buff[RST_LOG_MAX];
  uint32_t len=0;
  va_list ap;

    __disable_irq();;//TODO 인터럽트 비활성 코드 삽입

    snprintf(&buff[len], sizeof(buff), "RST,%04d-%02d-%02d %02d:%02d:%02d,", Date_Time.Year,
               Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);

    len = strlen(buff);

    va_start(ap, pFmt);
    vsnprintf(&buff[len], sizeof(buff)-len,pFmt, ap);
    va_end(ap);

    strcpy((char *)noInitData.rstLog,  buff);//리셋 원인 기록

    noInitData.key = 0x5a5a5a5a;
    HAL_NVIC_SystemReset();

}

bool restore_error(char *p_out, int32_t out_size)
{ 
  if(noInitData.key ==0x5a5a5a5a)
  {
    snprintf(p_out, out_size,"%s",noInitData.rstLog);
  return true;
  }

  return false;
}

void error_print(const char *pFmt, ...)
{
  va_list args;
  va_start(args, pFmt);
  io_vprintf(pFmt, args);
  va_end(args);
}

static osTimerId_t s_reset_timer_id;


// 타이머 콜백 함수
void rtu_reset_callback(void *argument)
{
  HAL_NVIC_SystemReset();
}

void reset_system_delay(uint32_t delay_seconds)
{
  // 타이머 속성 설정
  osTimerAttr_t timer_attr = {.name = "DelayTimer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0};

  // 원샷 타이머 생성 (한 번만 실행)
  s_reset_timer_id = osTimerNew(rtu_reset_callback, osTimerOnce, NULL, &timer_attr);

  if (s_reset_timer_id != NULL)
  {
    // 타이머 시작 (delay_seconds를 틱 단위로 변환)
    osStatus_t status = osTimerStart(s_reset_timer_id, delay_seconds * osKernelGetTickFreq());

    if (status != osOK)
    {
      task_printf("타이머 시작 실패\n");
    }
  }
  else
  {
    task_printf("타이머 생성 실패\n");
  }
}
