#include <stdio.h>
#include <string.h>

#include "cli_key_code.h"
#include "console_utile.h"
#include "debug_io.h"
#include "drv_rtc.h"



void test_rtc(void)
{
  DATE_TIME_BUF Date_Time;

  debug_printf("RTC 테스트 시작 (1초마다 현재 시간 출력)\r\n");
  debug_printf("CTRL+Q 입력 시 종료\r\n");

  drv_rtc_init();

  while (1)
  {
    // RTC 읽기
    if (drv_rtc_read(&Date_Time) == 0)
    {
      // 시간 출력
      debug_printf("현재 시간: %04d-%02d-%02d %02d:%02d:%02d\r\n", Date_Time.Year,
                   Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
    }
    else
    {
      debug_printf("RTC 읽기 실패\r\n");
    }

    // 1초 대기 및 키 체크
    if (get_key(1000) == KEY_CODE_CTRL_C)
    {
      debug_printf("테스트 종료 (CTRL+Q 감지)\r\n");
      break;
    }
  }

}
