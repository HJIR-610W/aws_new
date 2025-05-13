#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cli_input.h"
#include "cli_key_code.h"
#include "console_utile.h"
#include "dev_io.h"
#include "driver_do.h"
void test_power_signal(void)
{
  driver_t *do_cdma;
  driver_t *do_24v;
  driver_t *do_btm;
  driver_t *do_rain_heater;
  driver_t *do_rain_det;
  driver_t *do_rain_det_power;
  // DO 오픈
  do_cdma = driver_do_open(DO_PWR_CDMA, 0);
  do_24v = driver_do_open(DO_POWER_HART_24V_ACTIVE_H, 0);
  do_btm = driver_do_open(DO_BTM_PWCTRL, 0);
  do_rain_heater = driver_do_open(DO_CON_PWR_RAIN_ACTIVE_H,0);
  do_rain_det = driver_do_open(DO_CON_PWR_RAIN_DECT_ACTIVE_H, 0);


  debug_printf("파워 신호 제어 테스트 시작\r\n");
  debug_printf("입력 예: cdma,on  또는  24v,off  또는  btm,on\r\n");
  debug_printf("CTRL+C 입력 시 종료\r\n");

  while (1)
  {
  static  char signal[20] = {0};
  static   char cmd[10] = {0};

    debug_printf("입력 대기 (cdma/24v/btm/heater/raind,on/off) > ");
    int ret = cli_scanf_s("%19[^,],%9s", signal, sizeof(signal), cmd, sizeof(cmd));

    if (ret == CLI_KEYCODE_CTRL_C)
    {
      debug_printf("\r\nCTRL+C 감지: 테스트 종료\r\n");
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
          driver_do_high(do_cdma);  
          debug_printf("CDMA: ON (Low)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          driver_do_low(do_cdma);  
          debug_printf("CDMA: OFF (High)\r\n");
        }
        else
        {
          debug_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else if (strcmp(signal, "24v") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          driver_do_high(do_24v);  // ACTIVE_H → on=High
          debug_printf("24V: ON (High)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          driver_do_low(do_24v);  // ACTIVE_H → off=Low
          debug_printf("24V: OFF (Low)\r\n");
        }
        else
        {
          debug_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else if (strcmp(signal, "btm") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          driver_do_high(do_btm);  
          debug_printf("BTM: ON (Low)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          driver_do_low(do_btm);  
          debug_printf("BTM: OFF (High)\r\n");
        }
        else
        {
          debug_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else if (strcmp(signal, "heater") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          driver_do_high(do_rain_heater);  // ACTIVE_H → on=High
          debug_printf("rain: ON (High)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          driver_do_low(do_rain_heater);  // ACTIVE_H → off=Low
          debug_printf("rain: OFF (Low)\r\n");
        }
        else
        {
          debug_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else if (strcmp(signal, "raind") == 0)
      {
        if (strcmp(cmd, "on") == 0)
        {
          driver_do_high(do_rain_det);  // ACTIVE_H → on=High
          debug_printf("raind: ON (High)\r\n");
        }
        else if (strcmp(cmd, "off") == 0)
        {
          driver_do_low(do_rain_det);  // ACTIVE_H → off=Low
          debug_printf("raind: OFF (Low)\r\n");
        }
        else
        {
          debug_printf("명령어는 on 또는 off만 허용\r\n");
        }
      }
      else
      {
        debug_printf("알 수 없는 신호명입니다. cdma, 24v, btm,rain만 허용\r\n");
      }
    }
    else
    {
      debug_printf("입력 형식 오류. 예: cdma,on 또는 24v,off\r\n");
    }
  }

  // 필요시 닫기 (생략 가능)
  // driver_do_close(do_cdma);
  // driver_do_close(do_24v);
  // driver_do_close(do_btm);
}
