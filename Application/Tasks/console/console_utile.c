
#include "console_utile.h"

#include <math.h>    // For NAN, isnan, fabsf
#include <stdarg.h>  // For va_list in io_printf stub
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>  // For atoi, atof (대안 입력 파싱 시)
#include <string.h>  // For memcpy, strcmp (필요시)

#include "IO\dev_io.h"
#include "adc_calibration.h"
#include "app_adc.h"
#include "config_adc.h"
#include "console_define.h"
#include "console_scanf.h"
#include "driver_adc.h"
#include "fsl_shell.h"
#include "mcu_utile.h"
#include "usDelay.h"
#include "util_filter.h"
#include "util_time.h"
#include "vt100_command.h"
const char *g_unknown = "unknown";

const char* enableList[] = {"미사용", "사용"};


char recv_key(uint32_t timeout_ms)
{
  //char key;
  char ch=0;

  while(1)
  {
    if(io_recv(&ch, 1, timeout_ms))
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
  int input_value=0;
  while (3)
  {
    io_printf("%s (%d ~ %d): ", prompt, min_val, max_val);
    ret_scan = console_scanf("%d", &input_value);
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
    if (ret_scan == 1 && input_value >= min_val && input_value <= max_val)
    {
      *value = input_value;
      ret = MENU_OK;
      break;
    }
    io_printf("오류: 잘못된 입력입니다. 다시 시도하세요.\r\n");
  }
  return ret;
}


int print_menu(int width, const char* title, char** menu_list, int cnt)
{
  int total_width = width + 8;  // 좌우 여백 및 메뉴 번호 고려

  // 타이틀 가운데 정렬
  int title_len = strlen(title);
  int title_padding = (total_width - 2 - title_len) / 2;
  int title_padding_right = total_width - 2 - title_len - title_padding;

  // CTRL 문구 가운데 정렬
  const char* ctrl_msg = "CTRL+C 이전, CTRL+Q 종료";
  int ctrl_len = strlen(ctrl_msg);
  int ctrl_padding = (total_width - 2 - ctrl_len) / 2;
  int ctrl_padding_right = total_width - 2 - ctrl_len - ctrl_padding;

  // 상단 라인
  io_printf("+");
  for (int i = 0; i < total_width - 2; i++) io_printf("-");
  io_printf("+\r\n");

  // 타이틀 출력
  io_printf("|");
  for (int i = 0; i < title_padding; i++) io_printf(" ");
  io_printf("%s", title);
  for (int i = 0; i < title_padding_right; i++) io_printf(" ");
  io_printf("|\r\n");

  // 중간 라인
  io_printf("+");
  for (int i = 0; i < total_width - 2; i++) io_printf("-");
  io_printf("+\r\n");

  // 메뉴 리스트 출력
  for (int i = 0; i < cnt; i++)
  {
    io_printf("|  %2d. %-*s|\r\n", i + 1, width, menu_list[i]);
  }

  // CTRL 문구
  io_printf("|");
  for (int i = 0; i < ctrl_padding; i++) io_printf(" ");
  io_printf("%s", ctrl_msg);
  for (int i = 0; i < ctrl_padding_right; i++) io_printf(" ");
  io_printf("|\r\n");

  // 하단 라인
  io_printf("+");
  for (int i = 0; i < total_width - 2; i++) io_printf("-");
  io_printf("+\r\n");

  return cnt;
}

int32_t choice_menu(int width, const char* title, char** menu_list, int cnt,int32_t *choice)
{
  int32_t max_number;
  int status;
  
  while(1)
  {

  max_number = print_menu(width, title, menu_list,cnt);

  status = get_int_input("선택", choice, 1, max_number);
  if (status == MENU_ABORT || status == MENU_BACK)
    return status;
  if (status == MENU_OK)
    {
      return status;
    }
  }
}

int32_t user_decimal(const char *title,int min,int max, int *val)
{

  int status;


  while (1)
  {
    io_printf("%s\r\n",title);
    status = get_int_input("입력", val, min, max);
    if (status == MENU_ABORT || status == MENU_BACK)
      return status;
    if (status == MENU_OK)
    {
      return status;
    }
  }
}



/**
 * @retval 0보다 크면 사용자 입력이 있음
 */
int32_t select_indexFromList(const char* list[], int32_t (*func)(), uint16_t listCnt, bool number)
{
  int cnt;
  int index = 0;
  int funcCnt = 0;
  int indexMax;
  do
  {
    if (list)
    {
      for (int i = 0; i < listCnt; i++)
      {
        if (number == true)
        {
          io_printf("%d.%s\r\n", i, list[i]);
        }
        else
        {
          io_printf("%s\r\n", list[i]);
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

    vt100_printfColor(GREEN, "번호를 선택해 주세요:");
    cnt = console_scanf("%d", &index);
    io_printf("\r\n");
    if (cnt == 1)
    {
      if (index < indexMax)
        break;
    }
    else if (cnt == EXIT_BACK)
    {
      return EXIT_BACK;
    }
    else if (cnt == EXIT_PROGRAM)
    {
      return EXIT_PROGRAM;
    }
    vt100_printfColor(RED, "유효한 번호가 아닙니다\r\n");
  } while (1);

  return (index + 1);
}

int input_decimal(int32_t start, int32_t stop, int32_t* dec)
{
  int32_t cnt;

  io_printf("범위:%d~%d\r\n", start, stop);
  vt100_printfColor(GREEN, "값을 입력해 주세요:");
  cnt = console_scanf("%d", dec);
  if (cnt == 1)
  {
    if (*dec >= start && *dec <= stop)
    {
      return 1;
    }
    else
    {
      vt100_printfColor(RED, "입력값의 범위를 확인해 주세요\r\n");
      return 0;
    }
  }

  return cnt;
}

int32_t input_use( uint8_t* en)
{
  int32_t cnt;
  int32_t dec;
  io_printf("0:미사용\r\n");
  io_printf("1:사용\r\n");
  vt100_printfColor(GREEN, "번호를 선택해 주세요:");
  cnt = console_scanf("%d", &dec);
  if (cnt == 1)
  {
    if (dec >= 0 && dec <= 1)
    {
      *en = (uint8_t)dec;
      return 1;
    }
    else
    {
      io_printf("입력 범위를 확인해주세요\r\n");
      return 0;
    }
  }

  return cnt;
}

int32_t choice_enable(uint8_t *enable)
{
  const char *menu[]={"미사용","사용"};
  int32_t status;
  int32_t choice;

  status = choice_menu(20,"사용 여부",(char **)menu,_countof(menu),&choice);

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
