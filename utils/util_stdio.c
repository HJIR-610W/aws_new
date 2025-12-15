
#include "util_stdio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "util_memory.h"
#include "util_safe.h"

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

// utf8용 자간 일정하게 만드는 make_label
char *m_l(char *label, int width)
{
  int len;
  int remain;
  int current_len;
  static char buff[64];  // 버퍼 크기 증가

  if (label == NULL || width < 0)
  {
    buff[0] = 0;
    return buff;
  }

  strcpy_safe(buff, sizeof(buff), label);
  len = strlen(buff);
  current_len = utf8_strlen(label);
  
  remain = width - current_len;

  // remain이 양수이고 버퍼 오버플로우 방지
  if (remain > 0 && len + remain < sizeof(buff) - 1)
  {
    for (int i = 0; i < remain; i++)
    {
      buff[len++] = ' ';
    }
  }
  buff[len] = 0;

  return buff;
}

#define printf debug_printf

// 한글 한글자는 3바이트로 처리되는데 실제 화면 출력시 자간 2칸 사용됨
// utf8_strlen은 화면에 출력되는 기준으로 자간임
// wd가 20이라면 화면 출력 기준 20글자가 되어야 함
// 따라서 buff에 string을 복사하고 화면 출력시 총 자간 20자가 되도록 나머지 영역은 ' '으로 채워햐함
void make_utf8_string(char *buff, int buff_size, int wd,const char *string)
{
  int len;
  int remain;
  int current_len;
  
  if (buff == NULL || string == NULL || buff_size <= 0)
  {
    return;
  }

  // 문자열을 안전하게 복사
  strcpy_safe(buff, buff_size, string);
  
  // 현재 문자열의 화면 출력 길이 계산
  current_len = utf8_strlen(string);
  
  // 버퍼에서 문자열 끝 위치 찾기
  len = strlen(buff);
  
  // wd까지 남은 공백 수 계산
  remain = wd - current_len;
  
  // 남은 공간이 있고 버퍼 크기를 초과하지 않으면 공백으로 채우기
  if (remain > 0 && len + remain < buff_size)
  {
    for (int i = 0; i < remain; i++)
    {
      buff[len++] = ' ';
    }
  }
  buff[len] = 0;
}

// 소수점 n자리에서 반올림하는 함수
float round_to_n_digits(double value, int n)
{
  float factor = pow(10.0, n);
  // round 가까운 정수로 반올림
  return round(value * factor) / factor;
}

bool normalize_bool(uint8_t raw)
{
  return raw == 1 ? true : false;
}


