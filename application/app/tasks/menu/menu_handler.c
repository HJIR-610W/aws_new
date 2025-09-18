#define __STDC_WANT_LIB_EXT1__ 1
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>


#include "menu_handler.h"
#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "os_user_def.h"
#include "dev_io.h"
#include "util_memory.h"
#include "util_stdio.h"
#include "system_err.h"


#define MAX_FIELDS 6

typedef struct
{
  int start;
  int len;
} fmt_field_t;

void (*key_callback)(void);

void register_key_callback(void (*callback)(void))
{
key_callback = callback;
}

void unregister_key_callback(void)
{
  key_callback = NULL;
}


int32_t get_menu_key(uint32_t timeout_ms)
{
  int32_t key;

    key =  get_button_key(timeout_ms);

    if (key_callback && key !=KEY_CODE_NONE )
    {
      key_callback();
    }
    return key;
}

menu_status_t input_decimal(const char *title, int min, int max, int *val)
{
  char buff[LCD_COLS + 1] = {0};
  
  int cursor_pos = 0;
  int number_width = 0;
  int blink_state = 1;
  int sign_enable=0;
  uint32_t last_blink;

  if (val == NULL || title == NULL || min > max)
  {
    return MENU_BACK;
  }

  screen_clear();
  make_centered(buff, sizeof(buff), title, LCD_COLS);
  screen_printf(0, 0, "%s", buff);

  if (min < 0)
    sign_enable = 1;

    // 최대 자릿수 계산 (음수 고려)
    int temp_max = (abs(max) > abs(min)) ? abs(max) : abs(min);
    if (temp_max == 0)
      temp_max = 1;

    if (sign_enable)
      number_width = (int)log10(temp_max) + 2;
    else
      number_width = (int)log10(temp_max) + 1;
    if (min < 0)
    {
      if (sign_enable)
        number_width += 2;  // 음수 부호 고려
      else
        number_width += 1;
    }
  // 버퍼 크기 제한
  if (number_width >= LCD_COLS) number_width = LCD_COLS - 1;
  
  // 현재 값으로 버퍼 초기화 (부호 포함, 고정 폭)
  if(sign_enable)
  snprintf_s(buff, sizeof(buff), "%+0*d", number_width, *val);
  else
    snprintf_s(buff, sizeof(buff), "%0*d", number_width, *val);
  buff[sizeof(buff) - 1] = '\0';

  cursor_pos = 0; // 부호 위치(맨 왼쪽)부터 시작
  last_blink = OS_GET_TICK();
  
  while (1)
  {
    // 화면 출력

    if(sign_enable)
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
    screen_printf(4, 0, "Press ESC to Cancel", buff);
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
      if(sign_enable)
       screen_put_ch(3, 4+cursor_pos, display_char);
      else
        screen_put_ch(3, 4 + cursor_pos, display_char);
    }

    screen_refresh();

    int32_t key = get_menu_key(100);  
    
    if (key == KEY_CODE_NONE)
    continue;

    // 키 입력 시 커서 즉시 표시
    blink_state = 1;
    last_blink = OS_GET_TICK();
    screen_printf(3, 0, "Val:%s", buff);

    if (key == KEY_CODE_LEFT)
    {
      if (cursor_pos > 0)
      {
        cursor_pos--;
      }
    }
    else if (key == KEY_CODE_RIGHT)
    {
      if (cursor_pos < number_width - 1)  // 부호 포함 전체 길이 내에서 이동
      {
        cursor_pos++;
      }
    }
    if (key == KEY_CODE_UP || key == KEY_CODE_DOWN)
    {
      if (cursor_pos == 0&&sign_enable)  // 부호 위치
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
        if (key == KEY_CODE_UP)
        {
          ch += 1;
          if (ch > '9')
          {
            ch = '9';
           }
           buff[cursor_pos] = ch;
         }
         else if (key == KEY_CODE_DOWN)
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
        if (cursor_pos > 0 ||sign_enable==0)  // 부호 위치
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
        if(*val >=min && *val <=max)
        {
          return MENU_OK;
        }
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
/*
012345678901234567890
      PASS WORD
       [****]
*/
menu_status_t input_password(const char *title, int32_t *password)
{
  char temp[LCD_COLS + 1] = {0};
  char buff[6];
  int start_pos;
  int blink_state = 1;
  int cursor_pos = 0;
  int number_width = 4;
  uint32_t last_blink;
  
  
  screen_clear();
  make_centered(temp, sizeof(temp), title, LCD_COLS);
  screen_printf(1, 0, "%s", temp);

  make_centered(temp, sizeof(temp), "[****]", LCD_COLS);
  start_pos = (int)(strchr(temp, '[') - temp) + 1;
  screen_printf(3, 0, "%s", temp);
  while(1)
  {
    if (OS_GET_TICK() - last_blink >= 500)
    {
      last_blink = OS_GET_TICK();
      blink_state = !blink_state;
    }

    if (cursor_pos < 4)
    {
      char display_char = blink_state ? '*': ' ';
      screen_put_ch(3, start_pos + cursor_pos, display_char);
    }

    screen_refresh();

    int32_t key = get_menu_key(100); 
    if (key == KEY_CODE_NONE)
      continue;


    blink_state = 1;
    last_blink = OS_GET_TICK();


  if (key >= '0' && key <= '9')
  {
    buff[cursor_pos] = key;
    if (cursor_pos < number_width - 1) // 부호 포함 전체 길이 내에서 이동
    {
      screen_put_ch(3, start_pos + cursor_pos, '*');
      cursor_pos++;
    }
    
  }
  else if (key == KEY_CODE_ENTER)
  {
      *password = atoi(buff);
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

menu_status_t input_fmt(string_fmt_t *strfmt, const char *title)
{
  const char *fmt;
  char display[21];
  char buff[LCD_COLS];
  int field_count = 0;
  int fmt_len;
  int data_index = 0;
  int i, j, width;
  int current_field = 0;
  int cursor_pos = 0;
  int blink_state = 1;
  int f;
  int key;
  uint32_t last_blink;
  fmt_field_t fields[MAX_FIELDS];

  if (strfmt == NULL || title == NULL || strfmt->fmt == NULL)
  {
    return MENU_ERROR;
  }
  
  fmt_len = strlen(strfmt->fmt);
  fmt = strfmt->fmt;
  memset_s (display,sizeof(display), 0, sizeof(display));
  
  screen_clear();
  
  // Format string 파싱하여 편집 가능한 필드들 찾기
  for (i = 0; i < fmt_len && data_index < (int)sizeof(display) - 1; i++)
  {
    if (fmt[i] == '%')
    {
      width = 0;
      i++;
      while (i < fmt_len && isdigit(fmt[i]))
      {
        width = width * 10 + (fmt[i] - '0');
        i++;
      }
      if (i < fmt_len && fmt[i] == 'd')
      {
        if (field_count < MAX_FIELDS)
        {
          fields[field_count].start = data_index;
          fields[field_count].len = width;
          // 기본값으로 0으로 채우기
          for (j = 0; j < width && data_index < (int)sizeof(display) - 1; j++)
          {
            display[data_index++] = '0';
          }
          field_count++;
        }
      }
    }
    else
    {
      if (data_index < (int)sizeof(display) - 1)
      {
        display[data_index++] = fmt[i];
      }
    }
  }
  display[data_index] = '\0';
  
  // 기존 데이터가 있으면 복사
  if (strfmt->data[0] != '\0')
  {
    strncpy_s(display, sizeof(display),strfmt->data, sizeof(display) - 1);
    display[sizeof(display) - 1] = '\0';
  }
  
  // 첫 번째 필드에 커서 위치
  cursor_pos = (field_count > 0) ? fields[current_field].start : 0;
  last_blink = OS_GET_TICK();
  
  while (1)
  {
    // 화면 출력
    make_centered(buff,sizeof(buff),title,LCD_COLS);
    screen_printf(0, 0, "%s", buff);
    screen_printf(1, 0, "%s", display);
   // screen_printf(2, 0, "Field: %d/%d", current_field + 1, field_count);
   // screen_printf(3, 0, "Cursor: %d", cursor_pos);
    
    // 커서 깜빡임 처리 (500ms 간격)
    if (OS_GET_TICK() - last_blink >= 500)
    {
      last_blink = OS_GET_TICK();
      blink_state = !blink_state;
    }
    
    // 커서 위치의 문자만 깜빡이게 표시
    if (cursor_pos < (int)strlen(display))
    {
      char display_char = blink_state ? display[cursor_pos] : ' ';
      screen_put_ch(1, cursor_pos, display_char);
    }
    
    screen_refresh();
    
    key = get_menu_key(10);
    if (key == KEY_CODE_NONE) continue;
    
    // 키 입력 시 커서 즉시 표시
    blink_state = 1;
    last_blink = OS_GET_TICK();
    screen_printf(1, 0, "%s", display);
    
    switch (key)
    {
      case KEY_CODE_LEFT:
        // 이전 편집 가능한 필드로 이동
        do {
          if (cursor_pos > 0) cursor_pos--;
          else break;
          for (f = 0; f < field_count; f++)
          {
            if (cursor_pos >= fields[f].start && cursor_pos < fields[f].start + fields[f].len)
            {
              current_field = f;
              goto left_done;
            }
          }
        } while (1);
        left_done:;
        break;
        
      case KEY_CODE_RIGHT:
        // 다음 편집 가능한 필드로 이동
        do {
          if (cursor_pos < (int)strlen(display) - 1) cursor_pos++;
          else break;
          for (f = 0; f < field_count; f++)
          {
            if (cursor_pos >= fields[f].start && cursor_pos < fields[f].start + fields[f].len)
            {
              current_field = f;
              goto right_done;
            }
          }
        } while (1);
        right_done:;
        break;
        
      case KEY_CODE_ENTER:
        // 데이터 저장 및 콜백 호출
        if (strfmt->data != NULL)
        {
          strncpy_s(strfmt->data,sizeof(strfmt->data), display, sizeof(strfmt->data) - 1);
          strfmt->data[sizeof(strfmt->data) - 1] = '\0';
        }
        return MENU_OK;
        
      case KEY_CODE_CTRL_C:
        return MENU_BACK;
        
      case KEY_CODE_CTRL_Q:
        return MENU_ABORT;
        
      default:
        // 숫자 입력 처리
        if (key >= '0' && key <= '9')
        {
          if (current_field < field_count && 
              cursor_pos >= fields[current_field].start &&
              cursor_pos < fields[current_field].start + fields[current_field].len)
          {
            display[cursor_pos] = (char)key;
            screen_put_ch(1, cursor_pos, display[cursor_pos]);
            
            // 다음 위치로 커서 이동
            if (!(current_field == field_count - 1 && 
                  cursor_pos == fields[current_field].start + fields[current_field].len - 1))
            {
              cursor_pos++;
              if (cursor_pos >= fields[current_field].start + fields[current_field].len && 
                  current_field < field_count - 1)
              {
                current_field++;
                cursor_pos = fields[current_field].start;
              }
            }
          }
        }
        break;
    }
  }
}

menu_status_t input_float(const char *title, float min, float max, float *val, const char *fmt)
{
  char buff[LCD_COLS + 1] = {0};
  int cursor_pos = 0;
  int total_width = 0;
  int decimal_places = 0;
  int integer_places = 0;
  uint32_t last_blink;
  int blink_state = 1;
  int sign_enable = 0;
  int dot_pos = -1;
  float temp_val;
  int i;
  
  // 입력 검증
  if (val == NULL || title == NULL || min > max || fmt == NULL)
  {
    return MENU_ERROR;
  }
  
  // fmt 파라미터 분석 (예: "%6.1f")
  // %[total_width].[decimal_places]f 형식
  const char *p = fmt;
  if (*p != '%') return MENU_ERROR;
  p++;
  
  // total_width 파싱
  while (*p >= '0' && *p <= '9')
  {
    total_width = total_width * 10 + (*p - '0');
    p++;
  }
  
  total_width++;
  
  // 소수점 확인 및 decimal_places 파싱
  if (*p == '.')
  {
    p++;
    while (*p >= '0' && *p <= '9')
    {
      decimal_places = decimal_places * 10 + (*p - '0');
      p++;
    }
  }
  
  if (*p != 'f') return MENU_ERROR;
  
    screen_clear();

  make_centered(buff,sizeof(buff),title,LCD_COLS);
  screen_printf(0, 0, "%s", buff);
  
  // 부호 사용 여부 결정
  if (min < 0.0f)
  {
    sign_enable = 1;
  }
  
  // integer_places 계산 (부호 + 정수부 + 소수점)
  integer_places = total_width - decimal_places;
  if (decimal_places > 0) integer_places--; // 소수점 자리
  if (sign_enable) integer_places--; // 부호 자리
  
  // 버퍼 크기 제한
  if (total_width >= LCD_COLS) total_width = LCD_COLS - 1;
  
  // 현재 값으로 버퍼 초기화
  if (sign_enable)
  {
    if (decimal_places > 0)
    {
      snprintf_s(buff, sizeof(buff), "%+0*.*f", total_width, decimal_places, *val);
    }
    else
    {
      snprintf_s(buff, sizeof(buff), "%+0*d", total_width, (int)*val);
    }
  }
  else
  {
    if (decimal_places > 0)
    {
      snprintf_s(buff, sizeof(buff), "%0*.*f", total_width, decimal_places, *val);
    }
    else
    {
      snprintf_s(buff, sizeof(buff), "%0*d", total_width, (int)*val);
    }
  }
  buff[sizeof(buff) - 1] = '\0';
  
  // 소수점 위치 찾기
  for (i = 0; i < total_width; i++)
  {
    if (buff[i] == '.')
    {
      dot_pos = i;
      break;
    }
  }
  
  cursor_pos = 0; // 첫 번째 자리부터 시작
  last_blink = OS_GET_TICK();
  


  while (1)
  {
    // 화면 출력

    if (sign_enable)
    {
      screen_printf(1, 0, "Min: %+*.*f", total_width, decimal_places, min);
      screen_printf(2, 0, "Max: %+*.*f", total_width, decimal_places, max);
    }
    else
    {
      screen_printf(1, 0, "Min: %*.*f", total_width, decimal_places, min);
      screen_printf(2, 0, "Max: %*.*f", total_width, decimal_places, max);
    }

    screen_printf(3, 0, "Val:%s", buff);
    
    // 커서 깜빡임 처리 (500ms 간격)
    if (OS_GET_TICK() - last_blink >= 500)
    {
      last_blink = OS_GET_TICK();
      blink_state = !blink_state;
    }
    
    // 커서 위치의 문자만 깜빡이게 표시
    if (cursor_pos < total_width)
    {
      char display_char = blink_state ? buff[cursor_pos] : ' ';
      screen_put_ch(3, 4 + cursor_pos, display_char);
    }

    screen_refresh();

    int32_t key = get_menu_key(10);  // 10ms 대기
    if (key == KEY_CODE_NONE)
      continue;

    // 키 입력 시 커서 즉시 표시
    blink_state = 1;
    last_blink = OS_GET_TICK();
    screen_printf(3, 0, "Val:%s", buff);

    if (key == KEY_CODE_LEFT)
    {
      if (cursor_pos > 0)
      {
        cursor_pos--;
        // 소수점 건너뛰기
        if (dot_pos >= 0 && cursor_pos == dot_pos)
        {
          cursor_pos--;
        }
      }
    }
    else if (key == KEY_CODE_RIGHT)
    {
      if (cursor_pos < total_width - 1)
      {
        cursor_pos++;
        // 소수점 건너뛰기
        if (dot_pos >= 0 && cursor_pos == dot_pos && cursor_pos < total_width - 1)
        {
          cursor_pos++;
        }
      }
    }
    else if (key == KEY_CODE_UP || key == KEY_CODE_DOWN)
    {
      if (cursor_pos == 0 && sign_enable)  // 부호 위치
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
      else if (dot_pos < 0 || cursor_pos != dot_pos)  // 소수점이 아닌 위치
      {
        uint8_t ch = buff[cursor_pos];
        if (key == KEY_CODE_UP)
        {
          ch += 1;
          if (ch > '9')
          {
            ch = '9';
          }
          buff[cursor_pos] = ch;
        }
        else if (key == KEY_CODE_DOWN)
        {
          ch -= 1;
          if (ch < '0')
          {
            ch = '0';
          }
          buff[cursor_pos] = ch;
        }
      }
      
      // 범위 체크
      temp_val = atof(buff);
      if (temp_val < min)
      {
        if (sign_enable)
        {
          snprintf_s(buff, sizeof(buff), "%+0*.*f", total_width, decimal_places, min);
        }
        else
        {
          snprintf_s(buff, sizeof(buff), "%0*.*f", total_width, decimal_places, min);
        }
      }
      else if (temp_val > max)
      {
        if (sign_enable)
        {
          snprintf_s(buff, sizeof(buff), "%+0*.*f", total_width, decimal_places, max);
        }
        else
        {
          snprintf_s(buff, sizeof(buff), "%0*.*f", total_width, decimal_places, max);
        }
      }
    }
    else if (key >= '0' && key <= '9')
    {
      if ((cursor_pos > 0 || sign_enable == 0) && (dot_pos < 0 || cursor_pos != dot_pos))
      {
        buff[cursor_pos] = key;
        if (cursor_pos < total_width - 1)
        {
          cursor_pos++;
          // 소수점 건너뛰기
          if (dot_pos >= 0 && cursor_pos == dot_pos && cursor_pos < total_width - 1)
          {
            cursor_pos++;
          }
        }
        
        // 범위 체크
        temp_val = atof(buff);
        if (temp_val < min)
        {
          if (sign_enable)
          {
            snprintf_s(buff, sizeof(buff), "%+0*.*f", total_width, decimal_places, min);
          }
          else
          {
            snprintf_s(buff, sizeof(buff), "%0*.*f", total_width, decimal_places, min);
          }
        }
        else if (temp_val > max)
        {
          if (sign_enable)
          {
            snprintf_s(buff, sizeof(buff), "%+0*.*f", total_width, decimal_places, max);
          }
          else
          {
            snprintf_s(buff, sizeof(buff), "%0*.*f", total_width, decimal_places, max);
          }
        }
      }
    }
    else if (key == KEY_CODE_ENTER)
    {
      *val = atof(buff);
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

menu_status_t input_combobox(const char *title, const char *item_list[], int32_t item_count, int *choice)
{
  const char** combo_list;
  int32_t current_selection;
  int32_t max_display_rows;
  int32_t scroll_offset;
  int32_t status = MENU_OK;
  int total_width = LCD_COLS;  // 좌우 여백 및 메뉴 번호 고려
  char buff[LCD_COLS+1];
  int len=0;

      if (item_count <= 0 || choice == NULL || title == NULL || item_list == NULL)
  {
    return MENU_ERROR;
  }

  current_selection = *choice;
  if (current_selection < 0 || current_selection >= item_count)
  {
    current_selection = 0;
  }

  combo_list = item_list;
  max_display_rows = (item_count < LCD_ROWS - 1) ? item_count : (LCD_ROWS - 1);
  
  scroll_offset = 0;
  if (current_selection >= max_display_rows) 
  {
    scroll_offset = current_selection - max_display_rows + 1;
  }

  screen_clear();

  // 타이틀 가운데 정렬
  int title_len = utf8_strlen(title);
  int title_padding = (total_width - 2 - title_len) / 2;

  
  for (int i = 0; i < title_padding; i++) buff[len++] = ' ';

  snprintf_s(&buff[len], sizeof(buff) - len, "%s", title);
  screen_printf(0, 0, "%s", buff);

  while (1)
  {

    
    for (int32_t i = 0; i < max_display_rows; i++)
    {
      int32_t item_index = scroll_offset + i;
      if (item_index >= item_count)
        break;

      if (item_index == current_selection)
      {
        screen_printf(i + 1, 0, "*%s", combo_list[item_index]);
      }
      else
      {
        screen_printf(i + 1, 0, " %s", combo_list[item_index]);
      }
    }


    screen_refresh();

    int32_t key = get_menu_key(100);

    switch (key)
    {
      case KEY_CODE_UP:
        if (current_selection > 0)
        {
          current_selection--;
          if (current_selection < scroll_offset)
          {
            scroll_offset--;
          }
        }
        break;

      case KEY_CODE_DOWN:
        if (current_selection < item_count - 1)
        {
          current_selection++;
          if (current_selection >= scroll_offset + max_display_rows)
          {
            scroll_offset++;
          }
        }
        break;

      case KEY_CODE_ENTER:
        *choice = current_selection;
        return MENU_OK;

      case KEY_CODE_CTRL_Q:
        status = MENU_ABORT;
        break;

      case KEY_CODE_CTRL_C:
        status = MENU_BACK;
        break;

      default:
        break;
    }

    if (status != MENU_OK)
      break;
  }

  return status;
}

/**
 * @brief 정보 화면
 * 아무키나 눌러야지만 종료
 */
menu_status_t show_popup(const char *title, const char *message)
{
  char buff[LCD_COLS + 1];
  const int32_t lcd_cols = screen_get_instance()->font_cols;
  const int32_t lcd_rows = screen_get_instance()->font_rows; 
  int32_t key;
  int32_t message_len ;
  int32_t current_row ; // 메시지 시작 row
  int32_t current_col ;
  int32_t i;

  screen_clear();
  make_centered(buff, sizeof(buff), title, LCD_COLS);
  screen_printf(0, 0, "%s", buff);
  
  for (int col = 1; col < lcd_cols-1; col++)
  {
    screen_put_ch(1, col, '-');
  }

  if (message != NULL)
  {
     message_len = strlen(message);
     current_row = 2; // 메시지 시작 row
     current_col = 0;

    for ( i = 0; i < message_len; i++)
    {
      if (current_col >= lcd_cols)
      {
        current_row++;
        current_col = 0;
        if (current_row >= lcd_rows)
        {
          break; // LCD 영역 초과 시 출력 종료
        }
        screen_set_cursor(current_row, current_col);
      }

      screen_put_ch(current_row, current_col, message[i]);
      current_col++;
    }
  }

  screen_refresh();

  key = get_menu_key(WAIT_FOREVER);

  return convert_key_to_status(key);
}

menu_status_t input_active(const char *title, int32_t *choice)
{
  const char *yes = "[Yes] No ";
  const char *no =  " Yes [No]";
  int key;
  int enabled = *choice;

  char buff[50];
  int len=0;
  int total_width = LCD_COLS;  // 좌우 여백 및 메뉴 번호 고려

  // 타이틀 가운데 정렬
  int title_len = utf8_strlen(title);
  int title_padding = (total_width - 2 - title_len) / 2;

  for (int i = 0; i < title_padding; i++)
  buff[len++]=' ';

  snprintf_s(&buff[len],sizeof(buff)-len,"%s",title);

  screen_clear();

  screen_printf(0, 0, "%s", buff);

  while (1)
  {
    len = 0;
    if (enabled)
    {

          // 타이틀 가운데 정렬
           title_len = utf8_strlen(yes);
       title_padding = (total_width - 2 - title_len) / 2;

      for (int i = 0; i < title_padding; i++) buff[len++] = ' ';

      snprintf_s(&buff[len], sizeof(buff) - len, "%s", yes);

    }
    else
    {
      // 타이틀 가운데 정렬
       title_len = utf8_strlen(no);
       title_padding = (total_width - 2 - title_len) / 2;

      for (int i = 0; i < title_padding; i++) buff[len++] = ' ';

      snprintf_s(&buff[len], sizeof(buff) - len, "%s", no);
    }
    screen_printf(2, 0, "%s", buff);
    screen_refresh();

    key = get_menu_key(100);

    if (key == KEY_CODE_ENTER || key == KEY_CODE_CTRL_C || key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    if(key == KEY_CODE_RIGHT)
    {
      if(enabled==1)
      {
        enabled = 0;
      }
    }
    else if(key == KEY_CODE_LEFT)
    {
      if(enabled==0)
      {
        enabled = 1;
      }
    }
   }

   *choice = enabled;
   return convert_key_to_status(key);
}



int32_t make_sreen_row(char *buff,const char *pFmt, ...)
{
  va_list ap;
  int32_t len;

  int32_t remain_len;
  
  va_start(ap, pFmt);
  len = vsnprintf_s(buff, LCD_COLS+1, (char *)pFmt, ap);
  va_end(ap);

  if(len<0)
  {
    return 0;
  }

  if(len<LCD_COLS)
  {
    remain_len = LCD_COLS - len;
    for(int i = 0 ; i< remain_len; i++)
    {
      buff[len++]  = ' ';
    }
    buff[len]=0;
  }
  return len;
}

int32_t convert_key_to_status(int key)
{
  int32_t status = MENU_OK;

  if (key == KEY_CODE_CTRL_Q)
  {
    status = MENU_ABORT;
  }
  else if (key == KEY_CODE_CTRL_C)
  {
    status = MENU_BACK;
  }

  return status;
}

void screen_page_create(screen_page_t *win)
{
  win->p_screen = screen_get_instance();
  win->current_row = 0;
  win->current_page = 0;
  win->total_pages = 1;

  for (int i = 0; i < SCREEN_PAGE_MAX; i++)
  {
    win->scroll_offset[i] = 0;
    win->total_items[i] = 0;
  }
}

/**
 * @brief 타이틀이 메뉴용
 */
void screen_menu_create(screen_menu_t *win, const char *titile)
{
  win->p_screen = screen_get_instance();
  win->current_row = 0;
  win->scroll_offset = 0;
  win->total_items = 0;
  win->selected_index = 0; // 첫 번째 메뉴 항목이 기본 선택
  win->enter_long_key_active = false;

  if (titile)
  {
    snprintf_s(win->title, sizeof(win->title), "%s", titile); // 왼쪽 정렬
  }
  else
  {
    memset_s(win->title, sizeof(win->title),0, sizeof(win->title)); // 타이틀 초기화
  }
}

void screen_page_handle(screen_page_t *win, int key)
{
  int page = win->current_page;
  int new_offset = 0;
  int total_items = 0;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;

  // 행의 개수를 화면 행의 배수로 만든다.
  total_items = ALIGN_UP(win->total_items[page], view_rows);

  switch (key)
  {
  case KEY_CODE_UP: // 위로 스크롤
    if (win->scroll_offset[page] > 0)
    {
      if (win->chunk_scroll_enable) // 화면 전체행단위로 스크롤이라면
      {
        win->scroll_offset[page] -= view_rows; // 화면
      }
      else
      {
        win->scroll_offset[page] -= 1; // 1개행씩 스크롤
      }

      if (win->scroll_offset[page] < 0)
      {
        win->scroll_offset[page] = 0;
      }
    }
    break;
  case KEY_CODE_DOWN: // 아래로 스크롤
    if (win->chunk_scroll_enable)
    {
      new_offset = win->scroll_offset[page] + view_rows;
    }
    else
    {
      new_offset = win->scroll_offset[page] + 1;
    }

    if (new_offset < total_items)
    {
      win->scroll_offset[page] = new_offset;
    }
    break;
  case KEY_CODE_LEFT: // 이전 페이지
    if (win->multi_page_enable && (win->current_page > 0))
    {
      win->current_page--;
    }
    break;
  case KEY_CODE_RIGHT: // 다음 페이지
    if (win->multi_page_enable)
    {
      if (win->current_page < win->total_pages - 1)
      {
        win->current_page++;
      }
    }
    break;
  }
}

void screen_menu_printf(screen_menu_t *win, int index, const char *format, ...)
{
  char s_format_buffer[LCD_COLS]; // 정적 버퍼 크기는 필요에 따라 조정
  char selection_indicator;
  int display_row;
  int i;
  int title_offset = 0;
  int row_index;
  va_list args;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;

  row_index = win->total_items;

  win->index_list[row_index] = index;
  win->total_items++;

  if (win->title != NULL)
  {
    if (win->current_row == 0)
    {
      screen_printf(0, 0, win->title);
      win->current_row++;
    }
    title_offset = 1;
  }

  if (win->current_row >= view_rows)
  {
    return;
  }

  // 가변 인자를 문자열로 포맷팅
  va_start(args, format);
  vsnprintf_s(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);

  // 타이틀 오프셋을 고려하여 스크롤 범위 조정
  if (row_index >= win->scroll_offset && row_index < win->scroll_offset + view_rows - title_offset)
  {
    display_row = row_index - win->scroll_offset + title_offset;

    //    screen_set_cursor(display_row, 0);

    // 선택된 항목이면 '*', 아니면 ' ' 표시
    if (win->enter_long_key_active == true)
    {
      selection_indicator = (win->selected_index == row_index) ? '>' : ' ';
    }
    else
    {
      selection_indicator = (win->selected_index == row_index) ? '*' : ' ';
    }
    screen_put_ch(display_row, 0, selection_indicator);
    screen_printf(display_row, 1, "%s", s_format_buffer);

    win->current_row++;
  }

  if (row_index >= win->total_items)
  {
    win->total_items = row_index + 1;
  }
}

void screen_menu_handle(screen_menu_t *win, int key)
{
  int title_offset = (strlen(win->title) > 0) ? 1 : 0; // 타이틀이 존재 하면
  int effective_view_row;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;

  effective_view_row = view_rows - title_offset; // 타이틀을 제외한 행만 유효한 표시행

  switch (key)
  {
  case KEY_CODE_UP: // 위로 이동
    if (win->selected_index > 0)
    {
      win->selected_index--;

      // 선택된 항목이 화면 위쪽을 벗어나면 스크롤
      if (win->selected_index < win->scroll_offset)
      {
        win->scroll_offset = win->selected_index;
      }
    }
    break;

  case KEY_CODE_DOWN: // 아래로 이동
    if (win->selected_index < win->total_items - 1)
    {
      win->selected_index++;

      // 선택된 항목이 화면 아래쪽을 벗어나면 스크롤
      if (win->selected_index >= win->scroll_offset + effective_view_row)
      {
        win->scroll_offset = win->selected_index - effective_view_row + 1;
      }
    }
    break;
  }
}

void screen_menu_start(screen_menu_t *win)
{
  win->current_row = 0;
  win->total_items = 0;
}

void screen_menu_clear(screen_menu_t *win)
{
  int i;
  int display_row;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;

  for (display_row = win->current_row; display_row < view_rows; display_row++)
  {
    for (i = 0; i < view_cols; i++)
    {
      screen_put_ch(display_row, i, ' ');
    }
  }
}

/**
 * @brief win->current_row기준으로 view_row 남은 행을 전부 공백표시,clear
 */
void screen_page_clear(screen_page_t *win)
{
  int i;
  int display_row;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;

  for (display_row = win->current_row; display_row < view_rows; display_row++)
  {
    for (i = 0; i < view_cols; i++)
    {
      screen_put_ch(display_row, i, ' ');
    }
  }
}

void screen_page_start(screen_page_t *win)
{
  int page = win->current_page;
  win->current_row = 0;
  win->total_items[page] = 0;
}

void screen_page_printf(screen_page_t *win, const char *format, ...)
{
  char s_format_buffer[LCD_COLS + 1]; // 정적 버퍼 크기는 필요에 따라 조정
  int display_row;

  int i;
  int text_len;
  int page;
  int row_index;
  va_list args;
  int view_rows = win->p_screen->font_rows;
  int view_cols = win->p_screen->font_cols;

  row_index = win->total_items[win->current_page];

  win->total_items[win->current_page]++;

  if (win->current_row >= view_rows)
  {
    return;
  }

  // 가변 인자를 문자열로 포맷팅
  va_start(args, format);
  vsnprintf_s(s_format_buffer, sizeof(s_format_buffer), format, args);
  va_end(args);

  page = win->current_page;

  if (row_index >= win->scroll_offset[page] && row_index < win->scroll_offset[page] + view_rows)
  {
    display_row = row_index - win->scroll_offset[page];

    // screen_set_cursor(display_row, 0);

    screen_printf(display_row, 0, s_format_buffer);

    win->current_row++;
  }
}
