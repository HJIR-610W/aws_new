#include "console_utile.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IO\dev_io.h"
#include "adc_calibration.h"
#include "app_adc.h"
#include "config_adc.h"
#include "console_define.h"
#include "driver_adc.h"
#include "fsl_shell.h"
#include "mcu_utile.h"
#include "usDelay.h"
#include "util_filter.h"
#include "util_time.h"
#include "vt100_command.h"
#include "cli_input.h"
#include "cli_key_code.h"

const char *g_unknown = "unknown";

const char* enableList[] = {"비활성", "활성"};

static int get_visual_width(const char* str)
{
  int width = 0;
  int i = 0;
  
  while (str[i] != '\0') 
  {
    unsigned char c = (unsigned char)str[i];
    
    if (c < 0x80) 
    {
      width++;
      i++;
    }
    else if ((c & 0xE0) == 0xC0) 
    {
      width++;
      i += 2;
    }
    else if ((c & 0xF0) == 0xE0) 
    {
      width += 2;
      i += 3;
    }
    else if ((c & 0xF8) == 0xF0) 
    {
      width += 2;
      i += 4;
    }
    else 
    {
      width++;
      i++;
    }
  }
  
  return width;
}

char recv_key(uint32_t timeout_ms)
{
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
    io_printf("%s (%d ~ %d): ", prompt, min_val, max_val);
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
    io_printf("%s\r\n", STRING_INPUT_ERR);
  }
  return ret;
}


int print_menu(int width, const char* title, char** menu_list, int cnt)
{
  int total_width = width + 8;
  char line_buffer[256];

  int title_len = get_visual_width(title);
  int title_padding = (total_width - 2 - title_len) / 2;
  int title_padding_right = total_width - 2 - title_len - title_padding;

  const char* ctrl_msg = "CTRL+C 뒤로, CTRL+Q 종료";
  int ctrl_len = get_visual_width(ctrl_msg);
  int ctrl_padding = (total_width - 2 - ctrl_len) / 2;
  int ctrl_padding_right = total_width - 2 - ctrl_len - ctrl_padding;

  strcpy(line_buffer, "+");
  for (int i = 0; i < total_width - 2; i++) strcat(line_buffer, "-");
  strcat(line_buffer, "+\r\n");
  io_printf("%s", line_buffer);

  strcpy(line_buffer, "|");
  for (int i = 0; i < title_padding; i++) strcat(line_buffer, " ");
  strcat(line_buffer, title);
  for (int i = 0; i < title_padding_right; i++) strcat(line_buffer, " ");
  strcat(line_buffer, "|\r\n");
  io_printf("%s", line_buffer);

  strcpy(line_buffer, "+");
  for (int i = 0; i < total_width - 2; i++) strcat(line_buffer, "-");
  strcat(line_buffer, "+\r\n");
  io_printf("%s", line_buffer);

  for (int i = 0; i < cnt; i++)
  {
    int menu_visual_width = get_visual_width(menu_list[i]);
    int padding_needed = width - menu_visual_width;
    
    strcpy(line_buffer, "|  ");
    char num_str[8];
    snprintf(num_str, sizeof(num_str), "%2d. ", i + 1);
    strcat(line_buffer, num_str);
    strcat(line_buffer, menu_list[i]);
    
    for (int j = 0; j < padding_needed; j++) {
      strcat(line_buffer, " ");
    }
    strcat(line_buffer, "|\r\n");
    io_printf("%s", line_buffer);
  }

  strcpy(line_buffer, "|");
  for (int i = 0; i < ctrl_padding; i++) strcat(line_buffer, " ");
  strcat(line_buffer, ctrl_msg);
  for (int i = 0; i < ctrl_padding_right; i++) strcat(line_buffer, " ");
  strcat(line_buffer, "|\r\n");
  io_printf("%s", line_buffer);

  strcpy(line_buffer, "+");
  for (int i = 0; i < total_width - 2; i++) strcat(line_buffer, "-");
  strcat(line_buffer, "+\r\n");
  io_printf("%s", line_buffer);

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



int32_t select_indexFromList(const char* list[], int32_t (*func)(), uint16_t listCnt, bool number,int32_t *choice)
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
    io_printf("%s: ", prompt);
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
        io_printf("입력 범위: %.2f ~ %.2f\r\n", min, max);
      }
    }
    else
    {
      io_printf("%s\r\n", STRING_INPUT_ERR);
    }
  }

  return ret;
}

int check_pass(const char* title, char* password_str,int *ok)
{
  char input[16] = {0};
  int len;
  int status;

  io_printf("%s\r\n", title);
  io_printf(": ");

  while(1)
  {
    status = cli_scanf_s("%15s", input);

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

    io_printf("%s\r\n", STRING_INPUT_ERR);
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
    io_printf("%s(yes/no)\r\n",title);
    io_printf("입력:");
    status = cli_scanf_s("%15s", input);

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
    io_printf("%s\r\n", STRING_INPUT_ERR);
  }
  return status;
}

