

#include "bsp.h"
#include "cmsis_os2.h"
#include "task_start.h"


int is_debug_mode(void)
{ 
  return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0; 
}



#include <string.h>
#include "user_heap.h"
void tlsf_test(void)
{
  uint8_t *p_a = user_malloc(1054082);
  uint8_t *p_b = user_malloc(732);
  uint8_t *p_c = user_malloc(527040);
    
  memset(p_a,0xff,1054082);
  memset(p_b,0x00,732);
  memset(p_c,0xff,527040);
  
  user_free(p_c);
  user_free(p_b);
  user_free(p_a);
  
}

int main(void)
{

  if (is_debug_mode())
  {
    __HAL_DBGMCU_FREEZE_IWDG();  // 디버깅 시 와치독 카운트 멈춤
    __HAL_DBGMCU_FREEZE_RTC();   // 디버깅 시 rtc 타이머 멈춤
  }
  
  bsp_init();
 // tlsf_test();
  osKernelInitialize();

  startTask_init();

  osKernelStart();

  while(1);
  
}





