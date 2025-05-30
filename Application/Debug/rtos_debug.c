
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "dev_io.h"
#include "pcb_define.h"
#include "task.h"
#include "user_heap.h"








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