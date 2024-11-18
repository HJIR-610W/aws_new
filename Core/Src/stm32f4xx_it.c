
#include <stdio.h>
#include <stdarg.h>
#include "main.h"
#include "stm32f4xx_it.h"

extern ETH_HandleTypeDef heth;
extern TIM_HandleTypeDef htim4;


void NMI_Handler(void)
{

  while (1)
  {
    __asm("BKPT #0"); 
  }

}

__no_init volatile uint32_t stacked_reg[8];
__no_init volatile uint32_t fault_reg[4];

static const  uint32_t exc_ret[6]={0xFFFFFFF1,0xFFFFFFF9,0xFFFFFFFD,0xFFFFFFE1,0xFFFFFFE9,0xFFFFFFED};




   uint32_t *reg_sp=0;
   uint32_t reg_lr;
   uint32_t reg_msp;
   uint32_t reg_psp;
   uint32_t i;

void HardFault_Handler(void)
{
  reg_msp = __get_MSP()+16;//이코드전에 push 4개의 레지스터 동작해서 MSP가 변경됨
  reg_psp = __get_PSP();
  reg_lr = __get_LR();
    
  for(i = 0 ; i < 6;i++)
  {
    if(reg_lr == exc_ret[i])
    {
      if(i==0 || i ==3)
      {
        reg_sp = (uint32_t *)reg_msp;
      }
      else
      {
        reg_sp = (uint32_t*)reg_psp;
        if(reg_sp==0)
        {
          reg_sp = (uint32_t *)reg_msp;
        }
      }
      break;
    }
  }
      
  if(reg_sp)
  {
    stacked_reg[0] = reg_sp[0]; // R0
    stacked_reg[1] = reg_sp[1]; // R1
    stacked_reg[2] = reg_sp[2]; // R2
    stacked_reg[3] = reg_sp[3]; // R3
    stacked_reg[4] = reg_sp[4]; // R12
    stacked_reg[5] = reg_sp[5]; // LR
    stacked_reg[6] = reg_sp[6]; // PC
    stacked_reg[7] = reg_sp[7]; // PSR

    fault_reg[0] =  SCB->CFSR;
    fault_reg[1] =  SCB->HFSR;
    fault_reg[2] =  SCB->MMFAR;
    fault_reg[3] =  SCB->BFAR;
#if 0      
    debug_uart_init(115200);
        
    debug_printf("HardFault_Handler\r\n");
    debug_printf("R0   0x%08X\r\n",stacked_reg[0]);
    debug_printf("R1   0x%08X\r\n",stacked_reg[1]);
    debug_printf("R2   0x%08X\r\n",stacked_reg[2]);
    debug_printf("R3   0x%08X\r\n",stacked_reg[3]);
    debug_printf("R12  0x%08X\r\n",stacked_reg[4]);
    debug_printf("LR   0x%08X\r\n",stacked_reg[5]);
    debug_printf("PC   0x%08X\r\n",stacked_reg[6]);
    debug_printf("xPSR 0x%08X\r\n",stacked_reg[7]);
    debug_printf("SCB->CFSR  %08X\r\n",fault_reg[0]);
    debug_printf("SCB->HFSR  %08X\r\n",fault_reg[1]);
    debug_printf("SCB->MMFAR %08X\r\n",fault_reg[2]);
    debug_printf("SCB->BFAR  %08X\r\n",fault_reg[3]);
#endif
  }
     
  while (1)
  {
    #if DEBUG_MODE
    __asm("BKPT #0");
    #endif 
    HAL_NVIC_SystemReset();
  }
}

void MemManage_Handler(void)
{

  while (1)
  {
     __asm("BKPT #0"); 
  }
}


void BusFault_Handler(void)
{
  while (1)
  {
     __asm("BKPT #0"); 
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
    __asm("BKPT #0"); 
  }
}

void DebugMon_Handler(void)
{

}


void TIM4_IRQHandler(void)
{

  HAL_TIM_IRQHandler(&htim4);

}



/**
  * @brief This function handles Ethernet global interrupt.
  */
void ETH_IRQHandler(void)
{

  HAL_ETH_IRQHandler(&heth);

}


void RTC_WKUP_IRQHandler(void)
{
  #if 0
    // 1. RTC 웨이크업 인터럽트 발생 여부를 확인
    if (__HAL_RTC_WAKEUPTIMER_GET_FLAG(&hrtc, RTC_FLAG_WUTF) != RESET) {
        // 2. 인터럽트 플래그를 클리어
        __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
    }

    // 3. EXTI Line 22의 펜딩 비트를 클리어
    __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
    #endif
        // 1. RTC ISR 레지스터에서 WUTF 비트를 클리어합니다.
    RTC->ISR &= ~RTC_ISR_WUTF;

    // 2. EXTI PR 레지스터에서 EXTI Line 22 (RTC 웨이크업) 펜딩 비트를 클리어합니다.
    EXTI->PR = EXTI_PR_PR22;

}