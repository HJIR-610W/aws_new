#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"  // pvPortMalloc, vPortFree 사용 시 필요
#include "IO\dev_io.h"
#include "app_dataLogging.h"
#include "aws_data.h"
#include "cli_input.h"
#include "cmsis_os2.h"  // CMSIS-OS2 API 헤더
#include "console_define.h"
#include "console_utile.h"
#include "old_aws_define.h"
#include "task.h"  // (pvPortMalloc, vPortFree는 task.h 또는 FreeRTOS.h에 있을 수 있음)
#include "util_time.h"



// eTaskState 열거형을 문자열로 변환하는 헬퍼 함수
static const char *prvTaskStateToString(eTaskState eState)
{
  switch (eState)
  {
    case eRunning:
      return "Running";
    case eReady:
      return "Ready";
    case eBlocked:
      return "Blocked";
    case eSuspended:
      return "Suspended";
    case eDeleted:
      return "Deleted";
    case eInvalid:
      return "Invalid";  // 발생해서는 안 됨
    default:
      return "Unknown";
  }
}

// qsort를 위한 비교 함수:
// 1. uxBasePriority 기준 내림차순 (높은 우선순위 먼저)
// 2. uxBasePriority가 같으면, xTaskNumber 기준 오름차순 (configUSE_TRACE_FACILITY == 1 일 때)
// 3. xTaskNumber도 없거나 같으면, xHandle (주소값) 기준 오름차순
static int compareTaskBasePriority(const void *a, const void *b)
{
  const TaskStatus_t *taskA = (const TaskStatus_t *)a;
  const TaskStatus_t *taskB = (const TaskStatus_t *)b;

  // 1. 기본 우선순위(uxBasePriority) 기준 내림차순 정렬
  if (taskA->uxBasePriority < taskB->uxBasePriority)
  {
    return 1;  // taskB가 taskA보다 우선 (값이 작으므로 뒤로)
  }
  if (taskA->uxBasePriority > taskB->uxBasePriority)
  {
    return -1;  // taskA가 taskB보다 우선 (값이 크므로 앞으로)
  }

  // 기본 우선순위가 같은 경우, 보조 정렬 기준 적용
#if (configUSE_TRACE_FACILITY == 1)
  // 2. 태스크 번호(xTaskNumber) 기준 오름차순 정렬
  if (taskA->xTaskNumber < taskB->xTaskNumber)
  {
    return -1;
  }
  if (taskA->xTaskNumber > taskB->xTaskNumber)
  {
    return 1;
  }
#else
  // configUSE_TRACE_FACILITY가 0이면 xTaskNumber 필드가 없거나 유효하지 않음
  // 3. 태스크 핸들(xHandle - 주소값) 기준 오름차순 정렬 (고유성을 보장하기 위함)
  if ((uintptr_t)taskA->xHandle < (uintptr_t)taskB->xHandle)
  {
    return -1;
  }
  if ((uintptr_t)taskA->xHandle > (uintptr_t)taskB->xHandle)
  {
    return 1;
  }
#endif

  return 0;  // 모든 정렬 기준이 동일
}
void print_task_info(void)
{
  TaskStatus_t *pxTaskStatusArray;
  volatile UBaseType_t uxArraySize, x;
  uint32_t ulTotalRunTime = 0;
  unsigned long ulStatsAsPercentage;

  uxArraySize = uxTaskGetNumberOfTasks();
  if (uxArraySize == 0)
  {
    io_printf("No tasks are currently running.\r\n");
    return;
  }

  pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
  if (pxTaskStatusArray == NULL)
  {
    io_printf("Error: Failed to allocate memory for TaskStatus_t array.\r\n");
    return;
  }

  uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

#if (configUSE_TRACE_FACILITY == 1)
  if (uxArraySize > 1)
  {  // 태스크가 2개 이상일 때만 정렬 의미 있음
    qsort(pxTaskStatusArray, uxArraySize, sizeof(TaskStatus_t), compareTaskBasePriority);
  }
#endif  // configUSE_TRACE_FACILITY


#if (configUSE_TRACE_FACILITY == 1)
  io_printf("Info: Output sorted by Task Number (Task#).\r\n");
#else
  io_printf(
      "Warning: configUSE_TRACE_FACILITY is not 1. Task names, stack info, task numbers, and "
      "sorting by Task# are unavailable/limited.\r\n");
#endif
#if (configGENERATE_RUN_TIME_STATS == 1)
  io_printf("Info: Total system run time for stats: %lu ticks.\r\n",
               (unsigned long)ulTotalRunTime);
  if (ulTotalRunTime == 0 && uxArraySize > 0)
  {
    io_printf(
        "Warning: Total run time is 0. CPU usage statistics might be inaccurate. Ensure runtime "
        "stats timer is configured.\r\n");
  }
#else
  io_printf(
      "Info: configGENERATE_RUN_TIME_STATS is 0. CPU usage statistics are unavailable.\r\n");
#endif

  // 헤더 출력
  io_printf("%-18s %-10s %-9s %-7s %-10s %-16s %-5s", "Name", "Handle", "State", "PrioC/B",
               "StackBase", "StackHWM_Free(B)", "Task#");
#if (configGENERATE_RUN_TIME_STATS == 1)
  io_printf(" %-7s", "CPU(%)");
#endif
#if ((defined(configNUM_CORES) && configNUM_CORES > 1) && \
     (defined(configUSE_CORE_AFFINITY) && configUSE_CORE_AFFINITY == 1))
  io_printf(" %-6s", "CoreID");
#endif
  io_printf("\r\n");

  char separator_line[150];
  int current_len = 0;
  current_len += snprintf(
      separator_line + current_len, sizeof(separator_line) - current_len,
      "------------------ ---------- --------- ------- ---------- ----------------- -----");
#if (configGENERATE_RUN_TIME_STATS == 1)
  current_len +=
      snprintf(separator_line + current_len, sizeof(separator_line) - current_len, " -------");
#endif
#if ((defined(configNUM_CORES) && configNUM_CORES > 1) && \
     (defined(configUSE_CORE_AFFINITY) && configUSE_CORE_AFFINITY == 1))
  current_len +=
      snprintf(separator_line + current_len, sizeof(separator_line) - current_len, " ------");
#endif
  io_printf("%s\r\n", separator_line);

  for (x = 0; x < uxArraySize; x++)
  {
    char prio_str[8];
    snprintf(prio_str, sizeof(prio_str), "%u/%u",
             (unsigned int)pxTaskStatusArray[x].uxCurrentPriority,
             (unsigned int)pxTaskStatusArray[x].uxBasePriority);

    io_printf("%-18s %p %-9s %-7s %p %-16lu ",
                 pxTaskStatusArray[x].pcTaskName ? pxTaskStatusArray[x].pcTaskName : "N/A",
                 pxTaskStatusArray[x].xHandle,
                 prvTaskStateToString(pxTaskStatusArray[x].eCurrentState), prio_str,
                 pxTaskStatusArray[x].pxStackBase,
                 (unsigned long)pxTaskStatusArray[x].usStackHighWaterMark * sizeof(StackType_t));

#if (configUSE_TRACE_FACILITY == 1)
    io_printf("%-5u", (unsigned int)pxTaskStatusArray[x].xTaskNumber);
#else
    io_printf("%-5s", "N/A");
#endif

#if (configGENERATE_RUN_TIME_STATS == 1)
    if (ulTotalRunTime > 0)
    {
      ulStatsAsPercentage = (pxTaskStatusArray[x].ulRunTimeCounter * 100) / ulTotalRunTime;
    }
    else
    {
      ulStatsAsPercentage = 0;
    }
    io_printf(" %-7lu", ulStatsAsPercentage);
#endif

#if ((defined(configNUM_CORES) && configNUM_CORES > 1) && \
     (defined(configUSE_CORE_AFFINITY) && configUSE_CORE_AFFINITY == 1))
    if (pxTaskStatusArray[x].xCoreID == tskNO_AFFINITY)
    {
      io_printf(" %-6s", "Any");
    }
    else
    {
      io_printf(" %-6d", (int)pxTaskStatusArray[x].xCoreID);
    }
#endif
    io_printf("\r\n");
  }

  io_printf("%s\r\n", separator_line);

#if (configGENERATE_RUN_TIME_STATS == 1)
  io_printf("CPU(%%): ulTotalRunTime 대비 각 태스크의 ulRunTimeCounter 비율 (근사치).\r\n");
#endif
  io_printf(
      "--------------------------------------------------------------------------------------------"
      "---------\r\n\r\n");

  vPortFree(pxTaskStatusArray);
}