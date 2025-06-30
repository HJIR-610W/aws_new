#include "cli_key_code.h"
#include "dev_io.h"
#include "cmsis_os2.h"
#include "pcb_define.h"

extern uint32_t millis(void);  // 현재 ms를 가져오는 함수 (플랫폼에 맞게 구현)

int32_t get_key(uint32_t timeout_ms)
{
  char ch;
  uint32_t start_time = HAL_GetTick();
  uint32_t elapsed = 0;

  // 1. 첫 번째 바이트 수신 (최대 timeout_ms까지 기다림)
  while (1)
  {
    uint32_t remain = timeout_ms - elapsed;
    if (remain == 0)
    {
      return (int32_t)KEY_CODE_NONE;
    }

    if (io_recv(&ch, 1, remain) == 1)
    {
      break;
    }

    elapsed = HAL_GetTick() - start_time;
    if (elapsed >= timeout_ms)
    {
      return (int32_t)KEY_CODE_NONE;
    }
  }

  // 2. 첫 바이트 처리
  if (ch == 0x1B)
  {
    char seq[2];
    int seq_idx = 0;
    elapsed = HAL_GetTick() - start_time;

    while (seq_idx < 2)
    {
      uint32_t remain = timeout_ms - elapsed;
      if (remain == 0)
      {
        return (int32_t)KEY_CODE_ESC;
      }

      if (io_recv(&seq[seq_idx], 1, remain) == 1)
      {
        seq_idx++;
      }

      elapsed = HAL_GetTick() - start_time;
      if (elapsed >= timeout_ms)
      {
        return (int32_t)KEY_CODE_ESC;
      }
    }

    if (seq[0] == '[')
    {
      switch (seq[1])
      {
        case 'A':
          return KEY_CODE_UP;
        case 'B':
          return KEY_CODE_DOWN;
        case 'C':
          return KEY_CODE_RIGHT;
        case 'D':
          return KEY_CODE_LEFT;
        case 'H':
          return KEY_CODE_HOME;
        case 'F':
          return KEY_CODE_END;
        default:
          return KEY_CODE_UNKNOWN;
      }
    }
    else if (seq[0] == 'O')
    {
      switch (seq[1])
      {
        case 'H':
          return KEY_CODE_HOME;
        case 'F':
          return KEY_CODE_END;
        default:
          return KEY_CODE_UNKNOWN;
      }
    }

    return KEY_CODE_UNKNOWN;
  }

  // 3. Ctrl 키 조합
  if (ch >= 0x01 && ch <= 0x1A)
  {
    return (int32_t)ch;
  }

  // 4. 일반 키
  return (int32_t)ch;
}
