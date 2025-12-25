
#include <stdio.h>
#include <stdarg.h>
#include "pcb_define.h"
#include "stm32f4xx_it.h"
#include "system_err.h"
extern ETH_HandleTypeDef heth;
extern TIM_HandleTypeDef htim4;


void NMI_Handler(void)
{

  while (1)
  {
    __asm("BKPT #0"); 
  }

}

__no_init  uint32_t stacked_reg[8];
__no_init  uint32_t fault_reg[4];

static const  uint32_t exc_ret[6]={0xFFFFFFF1,0xFFFFFFF9,0xFFFFFFFD,0xFFFFFFE1,0xFFFFFFE9,0xFFFFFFED};

void fault_uart_init(uint32_t baud_rate)
{
    // 1. UART3 및 GPIO 클럭 활성화
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;  // UART3 클럭 활성화
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;   // GPIOB 클럭 활성화

    RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;  // USART3 리셋 활성화
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST; // USART3 리셋 비활성화
    
    // 2. GPIO 핀 설정 (PB10: TX, PB11: RX)
    GPIOB->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);  // 초기화
    GPIOB->MODER |= (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1); // AF 모드 설정
    GPIOB->AFR[1] &= ~((0xF << (2 * 4)) | (0xF << (3 * 4))); // AFR[1] 클리어 (핀 10, 11)
    GPIOB->AFR[1] |= (7 << (2 * 4)) | (7 << (3 * 4));         // AF7 (USART3)

    // 3. UART 설정
    USART3->CR1 &= ~USART_CR1_UE;  // UART 비활성화


    // BRR 레지스터 설정
    USART3->BRR =     UART_BRR_SAMPLING16(16000000, baud_rate);

    // (2) 데이터 비트, 패리티, 정지 비트 설정
    USART3->CR1 &= ~USART_CR1_M;    // 8 데이터 비트
    USART3->CR2 &= ~USART_CR2_STOP; // 1 정지 비트
    USART3->CR1 &= ~USART_CR1_PCE;  // 패리티 비활성화

    // (3) 송신(TX) 및 수신(RX) 활성화
    USART3->CR1 |= USART_CR1_TE;   // 송신 활성화

    // (4) UART 활성화
    USART3->CR1 |= USART_CR1_UE;   // UART 활성화

    // (5) 송신 준비 확인
    while (!(USART3->SR & USART_SR_TC));  // 송신 완료 플래그 확인
}

void fault_printf(const char * pFmt, ...)
{
  char buff[50];
  //char *ptr=buff;
  va_list ap;  



  va_start(ap, pFmt);
  vsnprintf((char *)buff, sizeof(buff), (char *)pFmt, ap);
  va_end(ap);
#if 0     
  while(*ptr)
  {
    while (!(USART3->SR & USART_SR_TXE));  // 송신 버퍼가 비어있는지 확인
    USART3->DR = (uint8_t)*ptr++;              // 데이터 레지스터에 문자 송신
  }
#endif

}

   uint32_t *reg_sp=0;
   uint32_t reg_lr;
   uint32_t reg_msp;
   uint32_t reg_psp;
   uint32_t i;


#include <stdio.h>

#define printf debug_printf
#include <stdio.h>

#define printf debug_printf

void analyze_fault(uint32_t cfsr, uint32_t hfsr, uint32_t mmfar, uint32_t bfar)
{
  fault_printf("Analyzing Fault...\r\n");

  // Usage Fault Analysis
  if (cfsr & 0xFFFF0000) {
    if (cfsr & (1 << 16)) {
      fault_printf("UsageFault: Undefined instruction usage detected.\r\n");
    }
    if (cfsr & (1 << 17)) {
      fault_printf("UsageFault: Attempt to execute an illegal state instruction.\r\n");
    }
    if (cfsr & (1 << 18)) {
      fault_printf("UsageFault: Illegal access to EPSR (exception state).\r\n");
    }
    if (cfsr & (1 << 19)) {
      fault_printf("UsageFault: Coprocessor access error.\r\n");
    }
    if (cfsr & (1 << 24)) {
      fault_printf("UsageFault: Divide by zero.\r\n");
    }
    if (cfsr & (1 << 25)) {
      fault_printf("UsageFault: Unaligned memory access.\r\n");
    }
  }

  // Bus Fault Analysis
  if (cfsr & 0x0000FF00) {
    if (cfsr & (1 << 8)) {
      fault_printf("BusFault: Instruction prefetch error.\r\n");
    }
    if (cfsr & (1 << 9)) {
      fault_printf("BusFault: Precise data bus error.\r\n");
      if (cfsr & (1 << 15)) { // BFARVALID
        fault_printf("BusFault: Fault address valid at BFAR: 0x%08X.\r\n", bfar);
      }
    }
    if (cfsr & (1 << 10)) {
      fault_printf("BusFault: Imprecise data bus error.\r\n");
    }
    if (cfsr & (1 << 11)) {
      fault_printf("BusFault: Bus fault occurred during exception entry.\r\n");
    }
  }

  // Memory Management Fault Analysis
  if (cfsr & 0x000000FF) {
    if (cfsr & (1 << 0)) {
      fault_printf("MemManageFault: Instruction access violation.\r\n");
    }
    if (cfsr & (1 << 1)) {
      fault_printf("MemManageFault: Data access violation.\r\n");
    }
    if (cfsr & (1 << 7)) {
      fault_printf("MemManageFault: Fault address valid at MMFAR: 0x%08X.\r\n", mmfar);
    }
  }

  // Hard Fault Analysis
  if (hfsr & (1 << 30)) {
    fault_printf("HardFault: Forced HardFault, possibly escalated from other faults.\r\n");
  }
  if (hfsr & (1 << 1)) {
    fault_printf("HardFault: Vector table read error.\r\n");
  }


}



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
        
   // fault_uart_init(115200);
        
    fault_printf("\r\nHardFault_Handler\r\n");
    fault_printf("R0   0x%08X\r\n",stacked_reg[0]);
    fault_printf("R1   0x%08X\r\n",stacked_reg[1]);
    fault_printf("R2   0x%08X\r\n",stacked_reg[2]);
    fault_printf("R3   0x%08X\r\n",stacked_reg[3]);
    fault_printf("R12  0x%08X\r\n",stacked_reg[4]);
    fault_printf("LR   0x%08X\r\n",stacked_reg[5]);
    fault_printf("PC   0x%08X\r\n",stacked_reg[6]);
    fault_printf("xPSR 0x%08X\r\n",stacked_reg[7]);
    fault_printf("SCB->CFSR  %08X\r\n",fault_reg[0]);
    fault_printf("SCB->HFSR  %08X\r\n",fault_reg[1]);
    fault_printf("SCB->MMFAR %08X\r\n",fault_reg[2]);
    fault_printf("SCB->BFAR  %08X\r\n",fault_reg[3]);

    analyze_fault(fault_reg[0], fault_reg[1], fault_reg[2],fault_reg[3]);
  }
     
  while (1)
  {
#if USE_DEBUG
    __asm("BKPT #0");
    #endif 
    reset_system("hard fault");
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


void FPU_IRQHandler(void)
{
  
}



extern ADC_HandleTypeDef *get_adc_handle(void);

void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler(get_adc_handle());
}