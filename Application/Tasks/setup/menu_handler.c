#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "menu_handler.h"
#include "app_button.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "os_user_def.h"

#include "util_memory.h"
#define MAX_ROWS 8
#define MAX_COLS 20

int32_t print_menu_list(const char* menu_list[], int32_t menu_count, int* choice)
{
  int status = MENU_OK;
  if (menu_count <= 0 || choice == NULL)
  {
    return MENU_BACK;
  }

  int current_selection = 0;
  int scroll_offset = 0;
  int max_display_rows = (menu_count < MAX_ROWS) ? menu_count : MAX_ROWS;

  while (true)
  {
    for (int i = 0; i < max_display_rows; i++)
    {
      int item_index = scroll_offset + i;
      if (item_index >= menu_count)

        break;

      if (item_index == current_selection)
      {
        screen_printf(i, 0, "*%s", menu_list[item_index]);
      }
      else
      {
        screen_printf(i, 0, " %s", menu_list[item_index]);
      }
    }

    screen_refresh();

    int32_t key = get_button_key(100);

    switch (key)
    {
      case '8':  // Up arrow
        if (current_selection > 0)
        {
          current_selection--;
          if (current_selection < scroll_offset)
          {
            scroll_offset--;
          }
        }
        break;

      case '2':  // Down arrow
        if (current_selection < menu_count - 1)
        {
          current_selection++;
          if (current_selection >= scroll_offset + MAX_ROWS)
          {
            scroll_offset++;
          }
        }
        break;

      case KEY_CODE_ENTER:  // Enter
        *choice = current_selection;
        return MENU_OK;

      case KEY_CODE_CTRL_Q:  // ESC
        status = MENU_ABORT;
        break;
      case KEY_CODE_CTRL_C:
        status = MENU_BACK;
        break;
    }

    if (status != MENU_OK)
      break;
  }

  return status;
}

#define SCREEN_KEY_UP    'w'
#define SCREEN_KEY_DOWN  's'
#define SCREEN_KEY_RIGHT 'd'
#define SCREEN_KEY_LEFT  'a'

int32_t input_decimal(const char *title, int min, int max, int *val,int sign_use)
{
  char buff[MAX_COLS + 1] = {0};
  int cursor_pos = 0;
  int number_width = 0;
  uint32_t last_blink;
  int blink_state = 1;
  int place, step, delta, new_val;
  
  // 입력 검증
  if (val == NULL || title == NULL || min > max)
  {
    return MENU_ERROR;
  }

  // 최대 자릿수 계산 (음수 고려)
  int temp_max = (abs(max) > abs(min)) ? abs(max) : abs(min);
  if (temp_max == 0) temp_max = 1;

  if(sign_use)
  number_width = (int)log10(temp_max) + 2;
  else
    number_width = (int)log10(temp_max) + 1;
  if (min < 0)
  {
    if(sign_use)
    number_width +=2; // 음수 부호 고려
    else
    number_width +=1;
  }
  // 버퍼 크기 제한
  if (number_width >= MAX_COLS) number_width = MAX_COLS - 1;
  
  // 현재 값으로 버퍼 초기화 (부호 포함, 고정 폭)
  if(sign_use)
  snprintf(buff, sizeof(buff), "%+0*d", number_width, *val);
  else
    snprintf(buff, sizeof(buff), "%0*d", number_width, *val);
  buff[sizeof(buff) - 1] = '\0';

  cursor_pos = 0; // 부호 위치(맨 왼쪽)부터 시작
  last_blink = OS_GET_TICK();
  
  screen_clear(MAX_ROWS, MAX_COLS);

  while (1)
  {
    // 화면 출력
    screen_printf(0, 0, "%s", title);
    if(sign_use)
    {
      screen_printf(1, 0, "Min: %+0*d", number_width, min);
      screen_printf(2, 0, "Max: %+0*d", number_width, max);
    }
    else
    {
      screen_printf(1, 0, "Min: %0*d", number_width, min);
      screen_printf(2, 0, "Max: %0*d", number_width, max);
    }

    screen_printf(3, 0, "Val:%s", buff);
    
    // 커서 깜빡임 처리 (500ms 간격)
    if (OS_GET_TICK() - last_blink >= 500)
    {
      last_blink = OS_GET_TICK();
      blink_state = !blink_state;
    }
    
    // 커서 위치의 문자만 깜빡이게 표시
    if (cursor_pos < number_width)
    {
      char display_char = blink_state ? buff[cursor_pos] : ' ';
      if(sign_use)
      screen_put_ch(3, 4+cursor_pos, display_char);
      else
        screen_put_ch(3, 4 + cursor_pos, display_char);
    }

    screen_refresh();

    int32_t key = get_button_key(10);  // 10ms 대기
    if (key == -1) continue;

    // 키 입력 시 커서 즉시 표시
    blink_state = 1;
    last_blink = OS_GET_TICK();
    screen_printf(3, 0, "Val:%s", buff);

    // 현재 커서 위치에서 자릿수 계산 (부호 위치 제외)
    if (cursor_pos > 0)
    {
      place = number_width - 1 - cursor_pos;
      step = (int)pow(10, place);
    }

    if (key == SCREEN_KEY_LEFT)
    {
      if (cursor_pos > 0)
      {
        cursor_pos--;
      }
    }
    else if (key == SCREEN_KEY_RIGHT)
    {
      if (cursor_pos < number_width - 1)  // 부호 포함 전체 길이 내에서 이동
      {
        cursor_pos++;
      }
    }
    if(key == SCREEN_KEY_UP || key== SCREEN_KEY_DOWN)
    {
      if (cursor_pos == 0&&sign_use)  // 부호 위치
      {
        if (buff[0] == '-')
        {
          buff[0] = '+';
        }
        else if (buff[0] == '+')
        {
          buff[0] = '-';
        }
      }
      else
      {
        uint8_t ch = buff[cursor_pos];
        if (key == SCREEN_KEY_UP)
        {
          ch += 1;
          if (ch > '9')
          {
            ch = '9';
           }
           buff[cursor_pos] = ch;
         }
         else if (key == SCREEN_KEY_DOWN)
         {
           ch -= 1;
           if(ch<'0')
           ch = '0';
           buff[cursor_pos] = ch;
         }
        }
      }
      else if(key>='0'&& key<='9')
      {
        if (cursor_pos > 0 ||sign_use==0)  // 부호 위치
        {
          buff[cursor_pos] = key;
          if (cursor_pos < number_width - 1)  // 부호 포함 전체 길이 내에서 이동
          {
            cursor_pos++;
          }
        }
      }
      else if (key == KEY_CODE_ENTER)
      {
        *val = atoi(buff);
        return MENU_OK;
      }
      else if (key == KEY_CODE_CTRL_C)
      {
        return MENU_BACK;
      }
      else if (key == KEY_CODE_CTRL_Q)
      {
        return MENU_ABORT;
      }

  }
}