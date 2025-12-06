#include "console_utile.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IO\debug_io.h"
#include "adc_calibration.h"
#include "app_adc.h"
#include "config_adc.h"
#include "console_define.h"
#include "drv_adc.h"
#include "fsl_shell.h"
#include "bsp.h"
#include "bsp_delay.h"
#include "util_filter.h"
#include "util_time.h"
#include "vt100_command.h"
#include "cli_input.h"
#include "cli_key_code.h"
#include "util_stdio.h"
#include "fsl_debug_console.h"
#include "os_user_def.h"

const char *g_unknown = "unknown";

const char* enableList[] = {"비활성", "활성"};



uint8_t recv_key(uint32_t timeout_ms)
{
  uint8_t ch=0;

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

int32_t console_scanf_s(const char* fmt, ...)
{
  va_list args;
  int ret;

  va_start(args, fmt);
  ret = cli_vscanf_s(fmt, args);
  va_end(args);

  if (ret == CLI_KEYCODE_CTRL_C)
  {
    ret = MENU_BACK;
  }
  else if (ret == CLI_KEYCODE_CTRL_Q)
  {
    ret = MENU_ABORT;
  }

  return ret;
}

int input_decimal_prompt(const char* prompt, int* value, int min_val, int max_val)
{
  int ret_scan;
  int ret = MENU_ABORT;
  int input_value=0;
  
  while(1)
  {
    debug_printf("%s (%d ~ %d): ", prompt, min_val, max_val);
    ret_scan = cli_scanf_s("%d", &input_value);
    if (ret_scan == CLI_KEYCODE_CTRL_Q)
    {
      ret = MENU_ABORT;
      break;
    }
    else if (ret_scan == CLI_KEYCODE_CTRL_C)
    {
      ret = MENU_BACK;
      break;
    }
    if (ret_scan == 1 && input_value >= min_val && input_value <= max_val)
    {
      *value = input_value;
      ret = MENU_OK;
      break;
    }
    debug_printf("%s\r\n", STRING_INPUT_ERR);
  }
  return ret;
}




//utf8 전용
int print_menu(int width, const char* title, char** menu_list, int cnt)
{
  char buff[50];
  int len;
  int total_width = width + 8;  // 좌우 여백 및 메뉴 번호 고려

  // 타이틀 가운데 정렬
  int title_len = utf8_strlen(title);
  int title_padding = (total_width - 2 - title_len) / 2;
  int title_padding_right = total_width - 2 - title_len - title_padding;

  // CTRL 문구 가운데 정렬
  const char* ctrl_msg = "CTRL+C 이전, CTRL+Q 종료";
  int ctrl_len = utf8_strlen(ctrl_msg);
  int ctrl_padding = (total_width - 2 - ctrl_len) / 2;
  int ctrl_padding_right = total_width - 2 - ctrl_len - ctrl_padding;

  // 상단 라인
  debug_printf("+");
  for (int i = 0; i < total_width - 2; i++) debug_printf("-");
  debug_printf("+\r\n");

  // 타이틀 출력
  debug_printf("|");
  for (int i = 0; i < title_padding; i++) debug_printf(" ");
  debug_printf("%s", title);
  for (int i = 0; i < title_padding_right; i++) debug_printf(" ");
  debug_printf("|\r\n");

  // 중간 라인
  debug_printf("+");
  for (int i = 0; i < total_width - 2; i++) debug_printf("-");
  debug_printf("+\r\n");

  // 메뉴 리스트 출력
  for (int i = 0; i < cnt; i++)
  {
    len = snprintf(buff, sizeof(buff), "|  %2d. %-s", i+1, menu_list[i]);
    debug_printf(buff);
    len = total_width - utf8_strlen(buff) - 1;
    for (int j = 0; j < len; j++) debug_printf(" ");
    debug_printf("|\r\n");
  }

  // CTRL 문구
  debug_printf("|");
  for (int i = 0; i < ctrl_padding; i++) debug_printf(" ");
  debug_printf("%s", ctrl_msg);
  for (int i = 0; i < ctrl_padding_right; i++) debug_printf(" ");
  debug_printf("|\r\n");

  // 하단 라인
  debug_printf("+");
  for (int i = 0; i < total_width - 2; i++) debug_printf("-");
  debug_printf("+\r\n");

  return cnt;
}
int32_t choice_menu(int width, const char* title, char** menu_list, int cnt,int32_t *choice)
{
  int32_t max_number;
  int status;
  
  while(1)
  {

  max_number = print_menu(width, title, menu_list,cnt);

  status = input_decimal_prompt("선택", choice, 1, max_number);
  if (status == MENU_ABORT || status == MENU_BACK)
    return status;
  if (status == MENU_OK)
    {
      return status;
    }
  }
}



int32_t select_index_from_table(const char* list[], int32_t (*func)(), uint16_t listCnt, bool number,int32_t *choice)
{

  int index = 0;
  int funcCnt = 0;
  int indexMax;
  int status=0;

  do
  {
    if (list)
    {
      for (int i = 0; i < listCnt; i++)
      {
        if (number == true)
        {
          debug_printf("%d.%s\r\n", i, list[i]);
        }
        else
        {
          debug_printf("%s\r\n", list[i]);
        }
      }
      indexMax = listCnt;
    }
    if (func)
    {
      funcCnt = func();
      indexMax = funcCnt;
    }

    if (func == NULL && list == NULL)
    {
      indexMax = listCnt;
    }

    status = input_decimal_prompt("번호를 선택해주세요",&index,0,indexMax-1);
    if(status!=MENU_OK)
    break;

      *choice = index;
      status = MENU_OK;
      break;
  } while (1);

  return status;
}



int32_t choice_enable(uint8_t *enable)
{
  const char *menu[]={"미사용","사용"};
  int32_t status;
  int32_t choice;

  status = choice_menu(20,"사용 선택",(char **)menu,_countof(menu),&choice);

  if(status ==MENU_OK)
  {
    *enable = choice-1;
  }

  return status;

}


bool wait_break(uint32_t timeoutms)
{
  int32_t ch;
  osDelay(timeoutms);
  ch = DbgConsole_GetcharNonBlocking();
  if (ch == -1)
  {
    return true;
  }

  return false;
}

int input_float_prompt(const char* prompt, float min, float max, float* value)
{
  int ret_scan;
  int ret;

  while (1)
  {
    debug_printf("%s: ", prompt);
    ret_scan = cli_scanf_s("%f", value);

    if (ret_scan == CLI_KEYCODE_CTRL_C)
    {
      ret = MENU_BACK;
      break;
    }
    else if (ret_scan == CLI_KEYCODE_CTRL_Q)
    {
      ret = MENU_ABORT;
      break;
    }
    else if (ret_scan == 1)
    {
      if(min==0&&max==0)
      {
        ret = MENU_OK;
        break;
      }
      else if(*value >= min && *value <= max)
      {
        ret = MENU_OK;
        break;
      }
      else
      {
        debug_printf("입력 범위: %.2f ~ %.2f\r\n", min, max);
      }
    }
    else
    {
      debug_printf("%s\r\n", STRING_INPUT_ERR);
    }
  }

  return ret;
}

int check_pass(const char* title, char* password_str,int *ok)
{
  char input[16] = {0};
  int len;
  int status;

  debug_printf("%s\r\n", title);
  debug_printf(": ");

  while(1)
  {
    status = cli_scanf_s("%s", input,sizeof(input));

    if(status == CLI_KEYCODE_CTRL_C)
    {
      status = MENU_BACK;
      break;
    }
    else if (status == CLI_KEYCODE_CTRL_Q)
    {
      status = MENU_ABORT;
      break;
    }
    else if(status >0)
    {
      len = strlen(password_str);

      if (strncmp(input, password_str, len) == 0)
      {
        status = MENU_OK;
        *ok = 1;
        break;
      }
      else
      {
        status = MENU_OK;
        *ok = 0;
        break;
      }
    }

    debug_printf("%s\r\n", STRING_INPUT_ERR);
 }

 return status;
}

int confirm_continue(const char *title,int32_t* ok)
{
  char input[16] = {0};
  int status;
  int len;
  while (1)
  {
    debug_printf("%s(yes/no)\r\n",title);
    debug_printf("입력:");
    status = cli_scanf_s("%s", input,sizeof(input));

    if (status == CLI_KEYCODE_CTRL_C)
    {
      status = MENU_BACK;
      break;
    }
    else if (status == CLI_KEYCODE_CTRL_Q)
    {
      status = MENU_ABORT;
      break;
    }
    else
    {
      len = strlen(input);
      if (len==3 && strncmp(input, "yes", 3) == 0)
      {
        *ok = 1;
        status = MENU_OK;
        break;
      }

      if (len==2 && strncmp(input, "no", 2) == 0)
      {
        *ok = 0;
        status = MENU_OK;
        break;
      }
    }
    debug_printf("%s\r\n", STRING_INPUT_ERR);
  }
  return status;
}

