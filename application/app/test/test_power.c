#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cli_input.h"
#include "cli_key_code.h"
#include "console_utile.h"
#include "debug_io.h"
#include "drv_do.h"

#include "drv_power.h"

void test_power_signal(void)
{

  dbg_printf("파워 신호 제어 테스트 시작\r\n");
  dbg_printf("입력 예: cdma,on  또는  24v,off  또는  btm,on\r\n");
  dbg_printf("CTRL+C 입력 시 종료\r\n");

  while (1)
  {
  static  char signal[20] = {0};
  static   char cmd[10] = {0};

    dbg_printf("입력 대기 (cdma/24v/btm/heater/raind,on/off) > ");
    int ret = cli_scanf_s("%19[^,],%9s", signal, sizeof(signal), cmd, sizeof(cmd));

    if (ret == CLI_KEYCODE_CTRL_C)
    {
      dbg_printf("\r\nCTRL+C 감지: 테스트 종료\r\n");
      break;
    }
    else if (ret == 2)
    {
      // 소문자로 변환 (대소문자 구분 없이)
      for (int i = 0; signal[i]; i++) signal[i] = (char)tolower(signal[i]);
      for (int i = 0; cmd[i]; i++) cmd[i] = (char)tolower(cmd[i]);

      if (strcmp(signal, "cdma") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          drv_power_on(DRV_POWER_CDMA);  
          dbg_printf("CDMA: ON (Low)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          drv_power_off(DRV_POWER_CDMA);
          dbg_printf("CDMA: OFF (High)\r\n");
        }
        else
        {
          dbg_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else if (strcmp(signal, "24v") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          drv_power_on(DRV_POWER_HART_24V);  // ACTIVE_H → on=High
          dbg_printf("24V: ON (High)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          drv_power_off(DRV_POWER_HART_24V);  // ACTIVE_H → off=Low
          dbg_printf("24V: OFF (Low)\r\n");
        }
        else
        {
          dbg_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else if (strcmp(signal, "btm") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          drv_power_on(DRV_POWER_LCD);  
          dbg_printf("BTM: ON (Low)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          drv_power_off(DRV_POWER_LCD);  
          dbg_printf("BTM: OFF (High)\r\n");
        }
        else
        {
          dbg_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else if (strcmp(signal, "heater") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          drv_power_on(DRV_POWER_RAIN_DECT_ANALOG);  // ACTIVE_H → on=High
          dbg_printf("rain: ON (High)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          drv_power_off(DRV_POWER_RAIN_DECT_ANALOG);  // ACTIVE_H → off=Low
          dbg_printf("rain: OFF (Low)\r\n");
        }
        else
        {
          dbg_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else if (strcmp(signal, "raind") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          drv_power_on(DRV_POWER_RAIN_DECT_DIGITAL);  // ACTIVE_H → on=High
          dbg_printf("raind: ON (High)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          drv_power_off(DRV_POWER_RAIN_DECT_DIGITAL);  // ACTIVE_H → off=Low
          dbg_printf("raind: OFF (Low)\r\n");
        }
        else
        {
          dbg_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else
      {
        dbg_printf("알 수 없는 신호명입니다. cdma, 24v, btm,rain만 허용\r\n");
      }
    }
    else
    {
      dbg_printf("입력 형식 오류. 예: cdma,on 또는 24v,off\r\n");
    }
  }

  // 필요시 닫기 (생략 가능)
  // driver_do_close(do_cdma);
  // driver_do_close(DRV_POWER_HART_24V);
  // driver_do_close(RV_POWER_LCD_RESET);
}
