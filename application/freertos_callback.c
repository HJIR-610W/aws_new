

#include <stdio.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "dev_io.h"
#include "pcb_define.h"
#include "system_err.h"

void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook( void )
{
  /* vApplicationIdleHook() 함수는 FreeRTOSConfig.h에서 configUSE_IDLE_HOOK가 1로 설정되어 있어야만
     호출됩니다. 이 함수는 idle 태스크가 한 번 실행될 때마다 호출됩니다. 이 훅 함수에 추가되는
     코드에서는 **절대로 블로킹 동작을 시도해서는 안 됩니다** (예: xQueueReceive()를 블로킹 시간과
     함께 호출하거나, vTaskDelay()를 호출하는 경우 등).

     만약 애플리케이션에서 vTaskDelete() API 함수를 사용한다면
     (이 데모 애플리케이션이 그렇게 하듯이),
     vApplicationIdleHook() 함수가 호출한 함수로 반드시 **리턴(return)** 하도록 하는 것도
     중요합니다.

     그 이유는, **삭제된 태스크가 사용했던 메모리를 해제(clean up)하는 책임이 idle 태스크에 있기
     때문**입니다. */
}

char g_task_name[20];

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  /* configCHECK_FOR_STACK_OVERFLOW가 1 또는 2로 정의되어 있으면
     런타임 중 스택 오버플로우(overflow) 검사가 수행됩니다.
     이 훅 함수는 스택 오버플로우가 감지되었을 때 호출됩니다. */

   snprintf(g_task_name,sizeof(g_task_name),"SOF,%s",pcTaskName);
   io_printf("SOF,%s",g_task_name);
   //__asm("BKPT #0");
   reset_system("%s", g_task_name);
}

void vApplicationMallocFailedHook(void)
{
/* vApplicationMallocFailedHook() 함수는 오직
   FreeRTOSConfig.h에서 configUSE_MALLOC_FAILED_HOOK이 1로 설정된 경우에만 호출됩니다.
   이 함수는 pvPortMalloc() 호출이 실패했을 때 실행되는 **훅 함수(hook function)**입니다.
   pvPortMalloc()은 커널 내부에서 태스크, 큐, 타이머 또는 세마포어를 생성할 때 호출됩니다.
   또한 데모 애플리케이션의 여러 부분에서도 호출됩니다.

   만약 heap_1.c 또는 heap_2.c를 사용하는 경우, pvPortMalloc()이 사용할 수 있는 heap의 크기는
   FreeRTOSConfig.h에 정의된 configTOTAL_HEAP_SIZE에 의해 결정됩니다.
   현재 남아 있는 heap의 크기를 확인하려면 xPortGetFreeHeapSize() API 함수를 사용할 수 있습니다.
   단, 이 함수는 남아 있는 heap이 얼마나 조각(fragmented)나 있는지는 알려주지 않습니다.
*/
  
  size_t free_heap = xPortGetFreeHeapSize();            // 현재 사용 가능한 힙 크기
  size_t min_free_heap = xPortGetMinimumEverFreeHeapSize(); // 프로그램 실행 중 가장 작았던 힙 크기

  io_printf("Error : Memory allocation failed.\r\n");
  io_printf("Free Heap Size          : %u bytes\r\n", (unsigned int)free_heap);
  io_printf("Minimum Ever Free Heap : %u bytes\r\n", (unsigned int)min_free_heap);


}

void YourTimerInitFunction(void) 
{
  // Cortex-M의 DWT를 활성화

  if(CoreDebug->DEMCR &CoreDebug_DEMCR_TRCENA_Msk == 0)
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // DWT Enable
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // Cycle Counter Enable
    DWT->CYCCNT = 0;                                // Counter 초기화
  }
}

uint32_t YourTimerGetCounterValue(void) 
{
  return DWT->CYCCNT;  // Cycle Counter 값 반환
}


