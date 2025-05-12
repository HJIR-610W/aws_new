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
  driver_t *count_c;
  float freq_b,freq_c;



  debug_printf("최소 주파수 약 15.26 Hz 이상부터 측정가능\r\n");
  debug_printf("아무키나 입력하면 측정 시작");
  get_key(0xFFFFFFFF);

  count_b = driver_freq_open(FREQ_MEAURE_B);
  count_c = driver_freq_open(FREQ_MEAURE_C);

  while(1)
  {
    freq_b = driver_freq_read(count_b);
    freq_c = driver_freq_read_duty(count_b);

    debug_printf("freq:%f,duty:%f\r\n", freq_b, freq_c);

    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      debug_printf("테스트 종료 (CTRL+Q 감지)\r\n");
      break;
    }
  }
}