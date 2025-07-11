
#include "util_stdio.h"

#include <string.h>

int32_t get_formatted_length_v(const char *format, va_list args)
{
    // vsnprintf를 사용하여 길이 계산
    int len = vsnprintf(NULL, 0, format, args);
    return len;
}

size_t utf8_strlen(const char* s)
{
  size_t count = 0;
  while (*s)
  {
    // 현재 바이트가 멀티바이트 문자의 시작 바이트가 아닌 경우에만 카운트를 증가시킵니다.
    // 0x80 (10000000), 0xC0 (11000000)
    // (s[0] & 0xC0) != 0x80 는 해당 바이트가 연속 바이트(10xxxxxx)가 아님을 의미합니다.
    if ((*s & 0xC0) != 0x80)
    {
      count++;
      if ((*s & 0xC0) == 0xC0)
      {
        count++;
      }
    }
    s++;
  }
  return count;
}

void make_centered(char *buffer, size_t buf_size, const char *text, int width)
{
  int text_len = strlen(text);

  if (text_len >= width || buf_size <= 1)
  {
    snprintf(buffer, buf_size, "%.*s", (int)buf_size - 1, text);
    return;
  }
  int left_padding = (width - text_len) / 2;
  int written = snprintf(buffer, buf_size, "%*s%s", left_padding, "", text);
  if (written < 0 || written >= buf_size - 1)
  {
    return;
  }

  for (int i = written; i < width && i < buf_size - 1; i++)
  {
    buffer[i] = ' ';
  }
  int end_pos = (width < buf_size) ? width : (int)buf_size - 1;
  buffer[end_pos] = '\0';
}