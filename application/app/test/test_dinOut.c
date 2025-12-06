#include "test_dinOut.h"

#include <stddef.h>
#include <string.h>
#include "cli_key_code.h"
#include "console_utile.h"
#include "debug_io.h"
#include "drv_di.h"
#include "drv_do.h"
#include "drv_power.h"
#include "cli_input.h"

#define DI_COUNT 6
#define DO_COUNT 6

void test_di(void)
{

  const uint32_t di_nums[DI_COUNT] = {DRV_DI_0, DRV_DI_1, DRV_DI_2, DRV_DI_3, DRV_DI_4, DRV_DI_RAIN_DETECT_DIGITAL};
  const char *di_names[DI_COUNT] = {"DRV_DI_0", "DRV_DI_1", "DRV_DI_20",
                                    "DRV_DI_3", "DRV_DI_4", "DRV_DI_5"};

  int32_t prev_state[DI_COUNT] = {-1, -1, -1, -1, -1, -1};  

  debug_printf("DI_EXT_0 ~ DI_EXT_5 상태 모니터링 시작\r\n");
  debug_printf("1초마다 상태를 읽어 변경 시 출력됩니다. CTRL+Q로 종료\r\n");


  while (1)
  {
    for (int i = 0; i < DI_COUNT; i++)
    {
      int32_t state = drv_di_read(di_nums[i]);
      if (state >= 0 && state != prev_state[i])
      {
        debug_printf("%s 상태 변경: %s\r\n", di_names[i], (state == 1) ? "High" : "Low");
        prev_state[i] = state;
      }
        else if (state < 0)
        {
          debug_printf("%s read 에러: %d\r\n", di_names[i], state);
        }

    }

    if (get_key(100) == KEY_CODE_CTRL_C)
    {
      debug_printf("테스트 종료\r\n");
      break;
    }
  }


}

void test_do(void)
{

  const uint32_t do_nums[DO_COUNT] = {DRV_DO_EXT_0, DRV_DO_EXT_1, DRV_DO_EXT_2, DRV_DO_EXT_3, DRV_DO_EXT_4, DRV_DO_EXT_5};
  const char *do_names[DO_COUNT] = {"DRV_DO_EXT_0", "DRV_DO_EXT_1", "DRV_DO_EXT_2",
                                    "DRV_DO_EXT_3", "DRV_DO_EXT_4", "DRV_DO_EXT_5"};



  debug_printf("DO_EXT_0 ~ DO_EXT_5 인터랙티브 테스트 시작\r\n");
  debug_printf("입력 예: 0,low  또는  3,high (번호,상태)\r\n");
  debug_printf("CTRL+C 입력 시 종료\r\n");

  drv_power_on(DRV_POWER_RAIN_DECT_DIGITAL);
  while (1)
  {
    int num;
    char state_str[10] = {0};

    debug_printf("출력 제어 입력 대기 (번호,상태) > ");
    int ret = cli_scanf_s("%d,%9s", &num, state_str,sizeof(state_str));

    if (ret == CLI_KEYCODE_CTRL_C)
    {
      debug_printf("\r\nCTRL+C 감지: 테스트 종료\r\n");
      break;
    }
    else if (ret == 2)
    {
      if (num < 0 || num >= DO_COUNT)
      {
        debug_printf("잘못된 번호입니다. 0 ~ %d 범위만 허용\r\n", DO_COUNT - 1);
        continue;
      }

       // 출력 제어
      if (strcasecmp(state_str, "low") == 0 || strcmp(state_str, "0") == 0)
      {
        drv_do_low(do_nums[num]);
        debug_printf("%s 출력: Low\r\n", do_names[num]);
      }
      else if (strcasecmp(state_str, "high") == 0 || strcmp(state_str, "1") == 0)
      {
        drv_do_high(do_nums[num]);
        debug_printf("%s 출력: High\r\n", do_names[num]);
      }
      else
      {
        debug_printf("상태는 low 또는 high 만 허용\r\n");
      }
    }
    else
    {
      debug_printf("입력 형식 오류. 예: 2,low 또는 3,high\r\n");
    }
  }

  
}