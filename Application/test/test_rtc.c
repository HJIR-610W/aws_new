#include <stdio.h>
#include <string.h>

#include "cli_key_code.h"
#include "console_utile.h"
#include "dev_io.h"
#include "driver_rtc.h"



void test_rtc(void)
{
  DATE_TIME_BUF Date_Time;
  driver_t *rtc;
  io_printf("RTC 테스트 시작 (1초마다 현재 시간 출력)\r\n");
  io_printf("CTRL+Q 입력 시 종료\r\n");

  // RTC 드라이버 오픈
  rtc = driver_rtc_open(RTC_RV8803, 0);
  if (rtc == NULL)
  {
    io_printf("RTC 오픈 실패!\r\n");
    return;
  }
  else
  {
    io_printf("RTC 오픈 성공\r\n");
  }

  while (1)
  {
    // RTC 읽기
    if (driver_rtc_read(rtc, &Date_Time) == 0)
    {
      // 시간 출력
      io_printf("현재 시간: %04d-%02d-%02d %02d:%02d:%02d\r\n", Date_Time.Year,
                   Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
    }
    else
    {
      io_printf("RTC 읽기 실패\r\n");
    }

    // 1초 대기 및 키 체크
    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      io_printf("테스트 종료 (CTRL+Q 감지)\r\n");
      break;
    }
  }

}
