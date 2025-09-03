

#include "bsp.h"
#include "cmsis_os2.h"
#include "task_start.h"


int is_debug_mode(void)
{ 
  return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0; 
}

volatile uint8_t data;
uint8_t user_data=0x01;
void fsmc_test(void)
{
volatile uint8_t *p_lcd   = (volatile uint8_t *)0x60000000;
volatile uint8_t *p_sram  = (volatile uint8_t *)0x64000000; 
volatile uint8_t *p_uart  = (volatile uint8_t *)0x68000000;



  
    *p_lcd = user_data;  
  p_sram[0] = 0x1;
  p_sram[2] = 0x2;
  p_sram[4] = 0x3;
    while(1)
  {

    data = p_sram[0];
    for(int i = 0 ; i< 3;i++);
    data += p_sram[2];
        for(int i = 0 ; i< 3;i++);
    data += p_sram[4];
        for(int i = 0 ; i< 3;i++);

    for(int i = 0 ; i< 168000;i++);

    
  }
}

int main(void)
{

  if (is_debug_mode())
  {
    __HAL_DBGMCU_FREEZE_IWDG();  // 디버깅 시 와치독 카운트 멈춤
    __HAL_DBGMCU_FREEZE_RTC();   // 디버깅 시 rtc 타이머 멈춤
  }
  
  bsp_init();


  
  osKernelInitialize();

  startTask_init();

  osKernelStart();

  while(1);
  
}





