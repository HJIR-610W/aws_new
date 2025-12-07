#include <stddef.h>
#include <string.h>

 
#include "cli_key_code.h"
#include "console_utile.h"
#include "debug_io.h"
#include "drv_di.h"
#include "drv_do.h"
#include  "drv_freqInput.h"


void test_freq(void)
{
  driver_t *count_b;
 driver_t *count_c;
  float freq_b,freq_c;
  uint8_t err=0;


   debug_printf("아무키나 입력하면 측정 시작");
  debug_get_key(0xFFFFFFFF);

  count_b = driver_freq_open(GENERAL_FREQ_1,"test");
  count_c = driver_freq_open(GENERAL_FREQ_2,"test");

  while(1)
  {
    freq_b = driver_freq_read(count_b,&err);
    freq_c = driver_freq_read(count_c, &err);

    debug_printf("freq b:%f,freq c:%f\r\n", freq_b, freq_c);

    if (debug_get_key(1000) == KEY_CODE_CTRL_C)
    {
      debug_printf("테스트 종료 (CTRL+Q 감지)\r\n");
      break;
    }
  }
}