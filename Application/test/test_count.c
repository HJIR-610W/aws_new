#include <stddef.h>
#include <string.h>

#include "cli_input.h"
#include "cli_key_code.h"
#include "console_utile.h"
#include "dev_io.h"
#include "driver_di.h"
#include "driver_do.h"
#include "test_dinOut.h"
#include  "driver_freqInput.h"


void test_freq(void)
{
  driver_t *count_b;
 // driver_t *count_c;
  float freq_b,duty_b;
  uint8_t err=0;


  io_printf("최소 주파수 약 15.26 Hz 이상부터 측정가능\r\n");
  io_printf("아무키나 입력하면 측정 시작");
  get_key(0xFFFFFFFF);

  count_b = driver_freq_open(FREQ_MEAURE_B);
 // count_c = driver_freq_open(FREQ_MEAURE_C);

  while(1)
  {
    freq_b = driver_freq_read(count_b,&err);
    duty_b = driver_freq_read_duty(count_b,&err);

    io_printf("freq b:%f,duty:%f\r\n", freq_b, duty_b);

    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      io_printf("테스트 종료 (CTRL+Q 감지)\r\n");
      break;
    }
  }
}