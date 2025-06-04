
#include "tlsf.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "crc.h"
#include "driver_stm32_bsp.h"
#include "fsmc.h"
#include "pcb_define.h"
#include "task_start.h"
#include "test_sram.h"
#include "user_heap.h"

extern void manual_bss_init(void);

/*
시스템 동작 클럭:168MHz
*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;  // 2로 하면 uart 1200bps

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

int is_debug_mode(void) { return (CoreDebug->DHCSR & (1 << 0)) != 0; }







int main(void)
{

#if DEBUG_MODE_EN
      if (is_debug_mode())
  {
    __HAL_DBGMCU_FREEZE_IWDG();  // 디버깅 시 와치독 카운트 멈춤
    __HAL_DBGMCU_FREEZE_RTC();   // 디버깅 시 rtc 타이머 멈춤
  }
#endif

  HAL_Init();  // 타이머 4를 초기화 HAL 타이머 틱 인터럽트로 사용

  SystemClock_Config();

  driver_stm32_bsp_init();

  MX_FSMC_Init();  // TODO: SRAM초기화,SystemInit_ExtMemCtl 이함수에 적용해야함

  manual_bss_init();

  MX_CRC_Init();

  asw_tlsf_init(POOL_SIZE);
  
  osKernelInitialize();

  startTask_init();

  osKernelStart();

  while (1)
  {
    
  }
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
}
  
