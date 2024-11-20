
#include "cmsis_os.h"
#include "main.h"
#include "project_def.h"
#include "task_start.h"
#include "io.h"
#include "tlsf.h"

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
        Error_Handler(__FILE__,__LINE__);
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
        Error_Handler(__FILE__,__LINE__);
  }
}


int is_debug_mode(void)
{
  return (CoreDebug->DHCSR& (1 << 0)) != 0;
}

void configure_swo_as_input(void)
{
    // GPIO 핀을 설정하기 위해 핸들 정의
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // GPIOB 클럭 활성화
    __HAL_RCC_GPIOB_CLK_ENABLE();


    // PB3 핀을 입력으로 설정
    GPIO_InitStruct.Pin = GPIO_PIN_3;         // PB3 (SWO 핀)
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // 입력 모드
    GPIO_InitStruct.Pull = GPIO_NOPULL;      // 풀업/풀다운 없음
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
int main(void)
{

#if DEBUG_MODE_EN
  if(is_debug_mode())
  {
    __HAL_DBGMCU_FREEZE_IWDG(); // 디버깅 시 와치독 카운트 멈춤
    __HAL_DBGMCU_FREEZE_RTC();  // 디버깅 시 rtc 타이머 멈춤
  }
#endif
  
  

  HAL_Init();//타이머 4를 초기헤 HAL 타이머 틱 인터럽트로 사용

  SystemClock_Config();
 
  configure_swo_as_input();
  
  while(1);
 
  osKernelInitialize(); 

  startTask_init();

  osKernelStart();

  while(1)
  {

  }


}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(const char *file,const int32_t line)
{
 
   debug_printf("%s,%d\r\n",file,line);

   __disable_irq();
    
   __asm("BKPT #0"); 
         
           
  while (1)
  {
    
  }
  /* USER CODE END     Error_Handler(__FILE__,__LINE__);_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


void Error_Handler_init(void)
{

}


