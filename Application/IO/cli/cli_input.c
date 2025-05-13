
#define __STDC_WANT_LIB_EXT1__ 1

#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>

#include "cli_key_code.h"
#include "dev_io.h"
#include "cli_input.h"

#define UART_LINE_MAX    128
#define UART_HISTORY_DEPTH  4


#define KEYCODE_CTRL_C    3
#define KEYCODE_CTRL_Q    17
#define KEYCODE_ESC       27
#define KEYCODE_UNKNOWN  1





static char history[UART_HISTORY_DEPTH][UART_LINE_MAX];
static int history_count = 0;
static int history_index = -1;

static const char *cmdlist[] = {
  "help",
  "status",
  "reboot",
  "reset",
  "version",
  "config",
  NULL
};


int uart_recv(char *ch)
{
  debug_recv(ch,1,0xffffffff);
  
  return 0;
}

void uart_send(char ch) 
{ 
  debug_putch (ch);
}
void uart_puts(const char *s)
{
  debug_puts(s);
}

static const char *autocomplete(const char *input)
{
  int len = strlen(input);
  for (int i = 0; cmdlist[i]; i++)
  {
    if (strncmp(cmdlist[i], input, len) == 0)
      return cmdlist[i];
  }
  return NULL;
}


// 줄 전체 지우고 새로 출력 (히스토리 불러올 때 사용)
static void clear_line_and_print(const char *buf, int len)
{
  uart_puts("\r");
}

// 줄 다시 그리기 (삽입, 삭제 등)
static void refresh_line(const char *buf, int len, int cursor_pos)
{
  uart_puts("\r");
  for (int i = 0; i < len; i++) uart_send(buf[i]);
  uart_puts(" ");
  int move_back = len - cursor_pos;
  while (move_back-- >= 0)
    uart_puts("\b");
}

int uart_get_line_with_edit(char *buf, int maxlen)
{
  int len = 0;
  int cursor_pos = 0;
  char ch;

  memset(buf, 0, maxlen);

  while (1)
  {
    if (uart_recv(&ch) != 0)
      continue;

    // Ctrl+C, Ctrl+Q
    if (ch == KEYCODE_CTRL_C)
    {
      uart_puts("\r\nCtrl+C detected.\r\n");
      buf[0] = '\0';
      return KEYCODE_CTRL_C;
    }
    if (ch == KEYCODE_CTRL_Q)
    {
      uart_puts("\r\nCtrl+Q detected.\r\n");
      buf[0] = '\0';
      return KEYCODE_CTRL_Q;
    }

    // Enter
    if (ch == '\r' || ch == '\n')
    {
      uart_send('\r');
      uart_send('\n');
      break;
    }

    // ESC 시퀀스 (방향키, Del 등)
    if (ch == 0x1B)
    {
      char seq[3] = {0};
      if (uart_recv(&seq[0]) == 0 && uart_recv(&seq[1]) == 0)
      {
        if (seq[0] == '[')
        {
          if (seq[1] == 'D')  // ← Left
          {
            if (cursor_pos > 0)
            {
              uart_puts("\b");
              cursor_pos--;
            }
          }
          else if (seq[1] == 'C')  // → Right
          {
            if (cursor_pos < len)
            {
              uart_send(buf[cursor_pos]);
              cursor_pos++;
            }
          }
          else if (seq[1] == 'A')  // ↑ Up (히스토리 이전)
          {
            if (history_count > 0 && history_index < history_count - 1)
            {
              for (int i = 0; i < cursor_pos; i++)
              {
                uart_puts("\b");
              }

              for (int i = 0; i < cursor_pos; i++)
              {
                uart_puts(" ");
              }

              for (int i = 0; i < cursor_pos; i++)
              {
                uart_puts("\b");
              }

              history_index++;
              strcpy(buf, history[history_index]);
              len = strlen(buf);
              cursor_pos = len;

              uart_puts(buf);
            }
          }
          else if (seq[1] == 'B')  // ↓ Down (히스토리 다음)
          {
            if (history_index > 0)
            {

              for (int i = 0; i < cursor_pos; i++)
              {
                uart_puts("\b");
              }

              for (int i = 0; i < cursor_pos; i++)
              {
                uart_puts(" ");
              }

              for (int i = 0; i < cursor_pos; i++)
              {
                uart_puts("\b");
              }

              history_index--;
              strcpy(buf, history[history_index]);
              len = strlen(buf);
              cursor_pos = len;
              uart_puts(buf);
              // clear_line_and_print(buf, len);
            }
            else if (history_index == 0)
            {
              history_index = -1;
              buf[0] = '\0';
              len = 0;
              cursor_pos = 0;
              clear_line_and_print(buf, len);
            }
          }
          else if (seq[1] == '3')  // Delete (ESC [3~)
          {
            char tilde;
            uart_recv(&tilde);  // '~' 수신
            if (cursor_pos < len)
            {
              memmove(&buf[cursor_pos], &buf[cursor_pos + 1], len - cursor_pos);
              len--;
              refresh_line(buf, len, cursor_pos);
            }
          }
        }
      }
      continue;
    }

    // Tab 자동완성
    if (ch == '\t')
    {
      buf[len] = 0;
      const char *suggest = autocomplete(buf);
      if (suggest)
      {
        int remain = strlen(suggest) - len;
        strncpy(&buf[len], &suggest[len], remain);
        len += remain;
        cursor_pos = len;
        refresh_line(buf, len, cursor_pos);
      }
      continue;
    }

// Backspace (0x08) 또는 Delete (0x7F) 구분 처리
if (ch == 0x08)  // Backspace
{
  if (cursor_pos > 0)
  {
    cursor_pos--;
    len--;

    memmove(&buf[cursor_pos], &buf[cursor_pos+1], len - cursor_pos);

    buf[len]=0;
    
    uart_puts("\b");
    uart_puts(&buf[cursor_pos]);
    uart_puts("  \b");

    for (int i = cursor_pos; i <= len; i++)
    {
      uart_puts("\b");
    }

  }

  continue;
}
else if (ch == 0x7F)  // Delete
{
  if (cursor_pos < len)
  {
    memmove(&buf[cursor_pos], &buf[cursor_pos + 1], len - cursor_pos);
    len--;
    refresh_line(buf, len, cursor_pos);
  }
  continue;
}


    // 일반 문자 입력
    if (isprint((unsigned char)ch) && len < maxlen - 1)
    {
      memmove(&buf[cursor_pos + 1], &buf[cursor_pos], len - cursor_pos);
      buf[cursor_pos] = ch;
      cursor_pos++;
      len++;
      uart_send(ch);
      continue;
    }
  }

  buf[len] = '\0';

  // 히스토리 저장
  if (len > 0)
  {
    if (history_count < UART_HISTORY_DEPTH)
      history_count++;
    for (int i = UART_HISTORY_DEPTH - 1; i > 0; i--) strcpy(history[i], history[i - 1]);
    strcpy(history[0], buf);
  }
  history_index = -1;

  return 0;
}


int cli_scanf_s(const char *fmt, ...)
{
  char input[100];
  va_list args;
  int ret;
  int code;

  code = uart_get_line_with_edit(input, sizeof(input));

  if (code == KEYCODE_CTRL_C ||code == KEYCODE_CTRL_Q)
  {
    switch (code)
    {
      case KEYCODE_CTRL_C:
        return CLI_KEYCODE_CTRL_C;
      case KEYCODE_CTRL_Q:
       return CLI_KEYCODE_CTRL_Q;
    }
  }

  va_start(args, fmt);
  ret = vsscanf_s(input, fmt, args);  // vsscanf_s 사용!
  va_end(args);

  return ret;
}