

#include <stdio.h>
#include "pcb_define.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "dev_io.h"

void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
char g_task_name[20];
bool g_stack_overflow=false;
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */


  snprintf(g_task_name,sizeof(g_task_name),"SOF,%s",pcTaskName);
 // debug_puts_nonos(g_task_name);
  io_printf("SOF,%s",g_task_name);
            __asm("BKPT #0");
  HAL_NVIC_SystemReset();
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
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

  io_printf("Free Heap Size          : %u bytes\r\n", (unsigned int)free_heap);
  io_printf("Minimum Ever Free Heap : %u bytes\r\n", (unsigned int)min_free_heap);


}