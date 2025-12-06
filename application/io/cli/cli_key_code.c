#include "cli_key_code.h"
#include "debug_io.h"
#include "cmsis_os2.h"
#include "pcb_define.h"

extern uint32_t millis(void);  

int32_t get_key(uint32_t timeout_ms)
{
  uint8_t ch;
  uint32_t start_time = HAL_GetTick();
  uint32_t elapsed = 0;


  while (1)
  {
    uint32_t remain = timeout_ms - elapsed;
    if (remain == 0)
    {
      return (int32_t)KEY_CODE_NONE;
    }

    if (debug_recv(&ch, 1, remain) == 1)
    {
      break;
    }

    elapsed = HAL_GetTick() - start_time;
    if (elapsed >= timeout_ms)
    {
      return (int32_t)KEY_CODE_NONE;
    }
  }


  if (ch == 0x1B)
  {
    uint8_t seq[2];
    int seq_idx = 0;
    elapsed = HAL_GetTick() - start_time;

    while (seq_idx < 2)
    {
      uint32_t remain = timeout_ms - elapsed;
      if (remain == 0)
      {
        return (int32_t)KEY_CODE_ESC;
      }

      if (debug_recv(&seq[seq_idx], 1, remain) == 1)
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


  if (ch >= 0x01 && ch <= 0x1A)
  {
    return (int32_t)ch;
  }


  return (int32_t)ch;
}

