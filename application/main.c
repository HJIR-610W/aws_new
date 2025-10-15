

#include "bsp.h"
#include "cmsis_os2.h"
#include "task_start.h"
#include "system_err.h"

int is_debug_mode(void)
{ 
  return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0; 
}





int main(void)
{

  if (is_debug_mode())
  {
    __HAL_DBGMCU_FREEZE_IWDG();  // 디버깅 시 와치독 카운트 멈춤
    __HAL_DBGMCU_FREEZE_RTC();   // 디버깅 시 rtc 타이머 멈춤
  }
  
  bsp_init();
#if IWDG_USE
  bsp_iwdg_init(16000);//iwdg task가 실행 전까지는 16초로 타임아웃
#endif
  osKernelInitialize();

  startTask_init();

  HAL_SuspendTick();
  
  osKernelStart();

  while(1);
  
}





