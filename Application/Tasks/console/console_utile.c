
#include <math.h>    // For NAN, isnan, fabsf
#include <stdarg.h>  // For va_list in debug_printf stub
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>  // For atoi, atof (대안 입력 파싱 시)
#include <string.h>  // For memcpy, strcmp (필요시)

#include "IO\dev_io.h"
#include "adc_calibration.h"
#include "app_adc.h"
#include "config_adc.h"
#include "console_scanf.h"
#include "fsl_shell.h"
#include "mcu_utile.h"
#include "usDelay.h"
#include "utile_time.h"
#include "vt100_command.h"

#include "utile_filter.h"
#include "driver_adc.h"
#include "console_define.h"



#include "console_define.h"
const char *g_unknown = "unknown";

char recv_key(uint32_t timeout_ms)
{
  char key;
  char ch=0;

  while(1)
  {
    if(debug_recv(&ch, 1, timeout_ms))
    {
      if(ch==0x1B || ch==0x5B)
      {
        continue;
      }
      break;
    }
    break;
  }
return ch;

}

int get_int_input(const char* prompt, int* value, int min_val, int max_val)
{
  int ret_scan;
  int ret = MENU_ABORT;
  while (3)
  {
    debug_printf("%s (%d ~ %d): ", prompt, min_val, max_val);
    ret_scan = console_scanf("%d", value);
    if (ret_scan == -3)
    {
      ret = MENU_ABORT;
      break;
    }
    else if (ret_scan == -1)
    {
      ret = MENU_BACK;
      break;
    }
    if (ret_scan == 1 && *value >= min_val && *value <= max_val)
    {
      ret = MENU_OK;
      break;
    }
    debug_printf("오류: 잘못된 입력입니다. 다시 시도하세요.\r\n");
  }
  return ret;
}
