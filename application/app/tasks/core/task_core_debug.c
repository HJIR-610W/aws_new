
#include "task_core_debug.h"

#include <stdarg.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"  // pvPortMalloc, vPortFree 사용 시 필요
#include "debug_io.h"
/**
 * @brief Task에서 디버깅용으로 출력 하고 싶을때
 *         콘솔 메뉴에서 task id를 설정해주면 id가 일치하는 task는 
 *         printf 
 */

static void *g_task_id;
static bool foreced_print = false;
void set_task_id(void *task_id)
{
  g_task_id = task_id;
}

void set_forced_print(bool set) { foreced_print = set; }


void task_printf(const char *pFmt, ...)
{
  void *task_id;

  task_id = osThreadGetId();

  if (task_id == NULL && foreced_print==false)
  {
    return;
  }

  if (g_task_id == NULL && foreced_print==false)
  {
    return;
  }

  if (task_id == g_task_id || (foreced_print))
  {
    va_list args;
    va_start(args, pFmt);
    debug_vprintf(pFmt, args); 
    va_end(args);
  }
}


void task_hex_dump(const char *title, const uint8_t *data, size_t length)
{
  if (title || foreced_print)
    task_printf("%s (len=%d):\r\n", title, (int)length);

  for (uint32_t i = 0; i < length; i++)
  {
    if (i % 16 == 0)
      task_printf("%04X: ", (unsigned int)i);  // 주소/인덱스 출력

    task_printf("%02X ", data[i]);

    if ((i + 1) % 16 == 0 || i + 1 == length)
      task_printf("\r\n");
  }
}
