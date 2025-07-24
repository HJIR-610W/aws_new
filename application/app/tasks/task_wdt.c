#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>

#include "dev_io.h"
#include "system_err.h"
#include "FreeRTOS.h"
#define TASK_MAX 10

const osThreadAttr_t kWdtTask_attributes = {
    .name = "wdt",
    .stack_size = TASK_WDT_STACK_SIZE,
    .priority = (osPriority_t)osPriorityRealtime2};

typedef struct
{
  const char *name;
  uint32_t timeout_ms;         
  uint32_t last_count;
  uint32_t last_updated_tick;
  uint8_t active;
} task_watchdog_t;

static task_watchdog_t task_watch_list[TASK_MAX];


static inline uint32_t now_tick(void)
{
  return osKernelGetTickCount();
}

void wdt_task_feed(int index)
{
  if (index < 0 || index >= TASK_MAX)
    return;

  if (task_watch_list[index].active == 0)
    return;

  task_watch_list[index].last_count++;
  task_watch_list[index].last_updated_tick = now_tick();
}

void wdt_task_unregister(int index)
{
  if (index < 0 || index >= TASK_MAX)
    return;

  task_watch_list[index].active = 0;
}

int wdt_task_register(const char *name, uint32_t timeout_ms)
{
  for (int i = 0; i < TASK_MAX; i++)
  {
    if (task_watch_list[i].active == 0)
    {
      task_watch_list[i].name = name;
      task_watch_list[i].timeout_ms = timeout_ms;
      task_watch_list[i].last_count = 0;
      task_watch_list[i].last_updated_tick = osKernelGetTickCount();
      task_watch_list[i].active = 1;
      return i;
    }
  }

  ERROR_PRINTF("더이상 task등록 못해요");
  return -1;
}

void wdtTask(void *argument)
{
  for (;;)
  {
    osDelay(60000);  

    uint32_t now = osKernelGetTickCount();

    for (int i = 0; i < TASK_MAX; i++)
    {
      if (task_watch_list[i].active == 0)
        continue;

      uint32_t elapsed = now - task_watch_list[i].last_updated_tick;

      if (elapsed > task_watch_list[i].timeout_ms)
      {
        ERROR_PRINTF("[WDT] Task '%s' not responding for %lu ms\r\n", task_watch_list[i].name,
                     elapsed);

        reset_system("[WDT]%s", task_watch_list[i].name);
      }
    }
  }
}

void wdtTask_init(void)
{
  osThreadNew(wdtTask, NULL, &kWdtTask_attributes);
}
