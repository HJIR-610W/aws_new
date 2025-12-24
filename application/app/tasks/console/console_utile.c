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
#include "shell.h"
#include "bsp.h"
#include "bsp_delay.h"
#include "util_filter.h"
#include "util_time.h"
 
 
#include "cli_key_code.h"
#include "util_stdio.h"

#include "os_user_def.h"



#define CONSOLE_LINE_BUFFER_SIZE 256



uint8_t view_recv_key(uint32_t timeout_ms)

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





int view_input_decimal(const char* prompt, int* value, int min_val, int max_val)

{

  int ret_scan;

  int ret = MENU_ABORT;

  int input_value=0;

  

  while(1)

  {

    debug_printf("%s (%d ~ %d): ", prompt, min_val, max_val);

    //ret_scan = debug_scanf_s("%d", &input_value);

    ret_scan = debug_scanf_s("%d", &input_value);

    if (ret_scan == KEY_CODE_CTRL_Q)

    {

      ret = MENU_ABORT;

      break;

    }

    else if (ret_scan == KEY_CODE_CTRL_C)

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

int print_menu(int width, const char* title, const char** menu_list, int cnt)
{
  char buff[CONSOLE_LINE_BUFFER_SIZE]; // Use CONSOLE_LINE_BUFFER_SIZE
  char line_buf[CONSOLE_LINE_BUFFER_SIZE];
 // int len;
  int total_width = width + 8;  // 좌우 여백 및 메뉴 번호 고려
  int title_len = utf8_strlen(title);
  int title_padding = (total_width - 2 - title_len) / 2;
  int title_padding_right = total_width - 2 - title_len - title_padding;
  const char* ctrl_msg = "CTRL+C 이전, CTRL+Q 종료";
  int ctrl_len = utf8_strlen(ctrl_msg);
  int ctrl_padding = (total_width - 2 - ctrl_len) / 2;
  int ctrl_padding_right = total_width - 2 - ctrl_len - ctrl_padding;


  snprintf(line_buf, sizeof(line_buf), "┌");
  for (int i = 0; i < total_width - 2; i++) strncat(line_buf, "─", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "┐\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);

  snprintf(line_buf, sizeof(line_buf), "│");
  for (int i = 0; i < title_padding; i++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, title, sizeof(line_buf) - strlen(line_buf) - 1);
  for (int i = 0; i < title_padding_right; i++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "│\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);

  snprintf(line_buf, sizeof(line_buf), "├");
  for (int i = 0; i < total_width - 2; i++) strncat(line_buf, "─", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "┤\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);


  for (int i = 0; i < cnt; i++)
  {
    snprintf(line_buf, sizeof(line_buf), "│  %2d. %-s", i+1, menu_list[i]);
   // len = total_width - utf8_strlen(line_buf) - 1; // total_width is physical chars, utf8_strlen is logical chars
    int current_visible_width = utf8_strlen(line_buf); // visible width of current content
    int space_to_add = total_width - current_visible_width ; // calculate spaces based on visible width

    

    for (int j = 0; j < space_to_add; j++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
    strncat(line_buf, "│\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
    debug_printf("%s", line_buf);

  }

  // CTRL 문구
  snprintf(line_buf, sizeof(line_buf), "│");
  for (int i = 0; i < ctrl_padding; i++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, ctrl_msg, sizeof(line_buf) - strlen(line_buf) - 1);
  for (int i = 0; i < ctrl_padding_right; i++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "│\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);



  // 하단 라인
  snprintf(line_buf, sizeof(line_buf), "└");
  for (int i = 0; i < total_width - 2; i++) strncat(line_buf, "─", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "┘\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);

  return cnt;
}


int print_combobox(int width, const char* title, const char** menu_list, int cnt,int selected_index)
{
  char buff[CONSOLE_LINE_BUFFER_SIZE]; // Use CONSOLE_LINE_BUFFER_SIZE
  char line_buf[CONSOLE_LINE_BUFFER_SIZE];
 // int len;
  int total_width = width + 8;  // 좌우 여백 및 메뉴 번호 고려
  int title_len = utf8_strlen(title);
  int title_padding = (total_width - 2 - title_len) / 2;
  int title_padding_right = total_width - 2 - title_len - title_padding;
  const char* ctrl_msg = "CTRL+C 이전, CTRL+Q 종료";
  int ctrl_len = utf8_strlen(ctrl_msg);
  int ctrl_padding = (total_width - 2 - ctrl_len) / 2;
  int ctrl_padding_right = total_width - 2 - ctrl_len - ctrl_padding;


  snprintf(line_buf, sizeof(line_buf), "┌");
  for (int i = 0; i < total_width - 2; i++) strncat(line_buf, "─", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "┐\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);

  snprintf(line_buf, sizeof(line_buf), "│");
  for (int i = 0; i < title_padding; i++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, title, sizeof(line_buf) - strlen(line_buf) - 1);
  for (int i = 0; i < title_padding_right; i++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "│\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);

  snprintf(line_buf, sizeof(line_buf), "├");
  for (int i = 0; i < total_width - 2; i++) strncat(line_buf, "─", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "┤\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);


  for (int i = 0; i < cnt; i++)
  {
    if(i==selected_index)
    {
      snprintf(line_buf, sizeof(line_buf), "│> %2d. %-s", i+1, menu_list[i]);
    }
    else
    {
      snprintf(line_buf, sizeof(line_buf), "│  %2d. %-s", i+1, menu_list[i]);
    }
    //len = total_width - utf8_strlen(line_buf) - 1; // total_width is physical chars, utf8_strlen is logical chars
    int current_visible_width = utf8_strlen(line_buf); // visible width of current content
    int space_to_add = total_width - current_visible_width ; // calculate spaces based on visible width
   

    for (int j = 0; j < space_to_add; j++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
    strncat(line_buf, "│\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
    debug_printf("%s", line_buf);

  }

  // CTRL 문구
  snprintf(line_buf, sizeof(line_buf), "│");
  for (int i = 0; i < ctrl_padding; i++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, ctrl_msg, sizeof(line_buf) - strlen(line_buf) - 1);
  for (int i = 0; i < ctrl_padding_right; i++) strncat(line_buf, " ", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "│\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);



  // 하단 라인
  snprintf(line_buf, sizeof(line_buf), "└");
  for (int i = 0; i < total_width - 2; i++) strncat(line_buf, "─", sizeof(line_buf) - strlen(line_buf) - 1);
  strncat(line_buf, "┘\r\n", sizeof(line_buf) - strlen(line_buf) - 1);
  debug_printf("%s", line_buf);

  return cnt;
}

int32_t view_input_combobox(const char *title, const char *item_list[], int32_t item_count, int *choice)
{
  int32_t max_number;
  int status;
  
  while(1)
  {

//  max_number = print_menu(30, title, item_list,item_count);
    max_number = print_combobox(30, title, item_list,item_count,*choice);
  status = view_input_decimal("선택", choice, 1, max_number);
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
  int indexMax=0;
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

    status = view_input_decimal("번호를 선택해주세요",&index,0,indexMax-1);
    if(status!=MENU_OK)
    break;

      *choice = index;
      status = MENU_OK;
      break;
  } while (1);

  return status;
}



int32_t view_input_active(const char *title,int32_t *enable)
{
  const char *menu[]={"미사용","사용"};
  int32_t status;
  int32_t choice;

  status = view_input_combobox(title,menu,_countof(menu),&choice);

  if(status == MENU_OK)
  {
    *enable = choice-1;
  }

  return status;

}




int view_input_float(const char* prompt, float min, float max, float* value)
{
  int ret_scan;
  int ret;

  while (1)
  {
    debug_printf("%s: ", prompt);
    ret_scan = debug_scanf_s("%f", value);

    if (ret_scan == KEY_CODE_CTRL_C)
    {
      ret = MENU_BACK;
      break;
    }
    else if (ret_scan == KEY_CODE_CTRL_Q)
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
    status = debug_scanf_s("%s", input,sizeof(input));

    if(status == KEY_CODE_CTRL_C)
    {
      status = MENU_BACK;
      break;
    }
    else if (status == KEY_CODE_CTRL_Q)
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

int view_confirm_continue(const char *title,int32_t* ok)
{
  char input[16] = {0};
  int status;
  int len;
  while (1)
  {
    debug_printf("%s(yes/no)\r\n",title);
    debug_printf("입력:");
    status = debug_scanf_s("%s", input,sizeof(input));

    if (status == KEY_CODE_CTRL_C)
    {
      status = MENU_BACK;
      break;
    }
    else if (status == KEY_CODE_CTRL_Q)
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