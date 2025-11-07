/**
 *
 * @note
 * FSMC Bank Address Ranges:
 * 
 * NE1 (FSMC_NORSRAM_BANK1): 0x60000000 - 0x63FFFFFF (64MB)
 *   - ST7920 LCD Controller (8-bit interface)
 *   - Base Address: 0x60000000
 *   - Address Range: 64MB
 * 
 * NE2 (FSMC_NORSRAM_BANK2): 0x64000000 - 0x67FFFFFF (64MB)  
 *   - External SRAM (16-bit interface)
 *   - Base Address: 0x64000000
 *   - Current Usage: SRAM memory 
 * 
 * NE3 (FSMC_NORSRAM_BANK3): 0x68000000 - 0x6BFFFFFF (64MB)
 *   - QUAD UART Controller (8-bit interface)  
 *   - Base Address: 0x68000000
 *   - Current Usage: Serial communication 
 * 
 * NE4 (FSMC_NORSRAM_BANK4): 0x6C000000 - 0x6FFFFFFF (64MB)
 *   - Reserved/Unused
 *   - Available for future expansion
 *
 */


#include "fsmc.h"


SRAM_HandleTypeDef hsram;
SRAM_HandleTypeDef hsram_uart;
SRAM_HandleTypeDef hsram_lcd; 


void MX_FSMC_Init(void)
{

  FSMC_NORSRAM_TimingTypeDef Timing = {0};
  FSMC_NORSRAM_TimingTypeDef ExtTiming = {0};

#if 1 
  // ST7920 LCD Controller (NE1 Bank)
  /** Perform the ST7920 LCD Controller initialization sequence
  */
  hsram_lcd.Instance = FSMC_NORSRAM_DEVICE;
  hsram_lcd.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram_lcd.Init */
  hsram_lcd.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram_lcd.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram_lcd.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram_lcd.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_8;
  hsram_lcd.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram_lcd.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram_lcd.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram_lcd.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram_lcd.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram_lcd.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram_lcd.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram_lcd.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram_lcd.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram_lcd.Init.PageSize = FSMC_PAGE_SIZE_NONE;

// Write Timing 설정 (더 보수적으로) 5.95ns
  //NE address setup time + data setup time 160ns  실측
  //WE data setup time 119ns
  
  Timing.AddressSetupTime = 5;
  Timing.AddressHoldTime = 15;//의미 없음 사용 안함 ,//0으로 하면 assert 발생, 재검토,HAL드라이버 문제 
  Timing.DataSetupTime = 20;//15;
  Timing.BusTurnAroundDuration = 5;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;



if (HAL_SRAM_Init(&hsram_lcd, &Timing, NULL) != HAL_OK)
{
    ERROR_PRINTF("famcd lcd");
  }
#endif


#if 1 
  //SRAM
  /** Perform the SRAM1 memory initialization sequence
  */
  hsram.Instance = FSMC_NORSRAM_DEVICE;
  hsram.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram.Init */
  hsram.Init.NSBank = FSMC_NORSRAM_BANK2;
  hsram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

  //NE address setup time + data setup time 22ns  실측 +1 *5.95
  //WE data setup time 11.9ns
  //OE 18ns 실측 (1+2)×5.95ns = 18ns
 Timing.AddressSetupTime       = 1;  
 Timing.AddressHoldTime        = 1;  
 Timing.DataSetupTime          = 3;  
 Timing.BusTurnAroundDuration  = 1;  
 Timing.CLKDivision            = 2;  //  비동기 모드에서는 무시,0설정시 assert 발생
 Timing.DataLatency            = 2;  //  비동기 모드에서는 무시
 Timing.AccessMode             = FSMC_ACCESS_MODE_A;  // 기본 액세스 모드

  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram, &Timing, NULL) != HAL_OK)
  {
    ERROR_PRINTF("fsmc sram");
  }
#endif


  //QUAD UART
  hsram_uart.Instance = FSMC_NORSRAM_DEVICE;
  hsram_uart.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

  hsram_uart.Init.NSBank = FSMC_NORSRAM_BANK3;
  hsram_uart.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram_uart.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram_uart.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_8;
  hsram_uart.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram_uart.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram_uart.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram_uart.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram_uart.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram_uart.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram_uart.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram_uart.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram_uart.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram_uart.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 4;
  Timing.AddressHoldTime  = 1;
  Timing.DataSetupTime    = 15;
  Timing.BusTurnAroundDuration = 10;

  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */
  ExtTiming.AddressSetupTime      = 4;
  ExtTiming.AddressHoldTime       =  1;//0으로 하면 assert 발생, 재검토,HAL드라이버 문제
  ExtTiming.DataSetupTime         = 10;
  ExtTiming.BusTurnAroundDuration = 10;
  Timing.CLKDivision              = 2;  //  비동기 모드에서는 무시,0설정시 assert 발생
  Timing.DataLatency              = 2;  //  비동기 모드에서는 무시
 

  if (HAL_SRAM_Init(&hsram_uart, &Timing, &ExtTiming) != HAL_OK)
  {
    ERROR_PRINTF("fsmc quad uart");
  }


}

static uint32_t FSMC_Initialized = 0;

static void HAL_FSMC_MspInit(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (FSMC_Initialized)
  {
    return;
  }
  FSMC_Initialized = 1;

  /* Peripheral clock enable */
  __HAL_RCC_FSMC_CLK_ENABLE();

  /** FSMC GPIO Configuration
  PE3   ------> FSMC_A19
  PE4   ------> FSMC_A20
  PF0   ------> FSMC_A0
  PF1   ------> FSMC_A1
  PF2   ------> FSMC_A2
  PF3   ------> FSMC_A3
  PF4   ------> FSMC_A4
  PF5   ------> FSMC_A5
  PF12   ------> FSMC_A6
  PF13   ------> FSMC_A7
  PF14   ------> FSMC_A8
  PF15   ------> FSMC_A9
  PG0   ------> FSMC_A10
  PG1   ------> FSMC_A11
  PE7   ------> FSMC_D4
  PE8   ------> FSMC_D5
  PE9   ------> FSMC_D6
  PE10   ------> FSMC_D7
  PE11   ------> FSMC_D8
  PE12   ------> FSMC_D9
  PE13   ------> FSMC_D10
  PE14   ------> FSMC_D11
  PE15   ------> FSMC_D12
  PD8   ------> FSMC_D13
  PD9   ------> FSMC_D14
  PD10   ------> FSMC_D15
  PD11   ------> FSMC_A16
  PD12   ------> FSMC_A17
  PD13   ------> FSMC_A18
  PD14   ------> FSMC_D0
  PD15   ------> FSMC_D1
  PG2   ------> FSMC_A12
  PG3   ------> FSMC_A13
  PG4   ------> FSMC_A14
  PG5   ------> FSMC_A15
  PD0   ------> FSMC_D2
  PD1   ------> FSMC_D3
  PD4   ------> FSMC_NOE
  PD5   ------> FSMC_NWE
  PD7   ------> FSMC_NE1
  PG9   ------> FSMC_NE2
  PG10   ------> FSMC_NE3
  */
  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* GPIO_InitStruct */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);


}

void HAL_SRAM_MspInit(SRAM_HandleTypeDef* sramHandle)
{

  HAL_FSMC_MspInit();

}

static uint32_t FSMC_DeInitialized = 0;

static void HAL_FSMC_MspDeInit(void)
{

  if (FSMC_DeInitialized) {
    return;
  }
  FSMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FSMC_CLK_DISABLE();

  /** FSMC GPIO Configuration
  PE3   ------> FSMC_A19
  PE4   ------> FSMC_A20
  PF0   ------> FSMC_A0
  PF1   ------> FSMC_A1
  PF2   ------> FSMC_A2
  PF3   ------> FSMC_A3
  PF4   ------> FSMC_A4
  PF5   ------> FSMC_A5
  PF12   ------> FSMC_A6
  PF13   ------> FSMC_A7
  PF14   ------> FSMC_A8
  PF15   ------> FSMC_A9
  PG0   ------> FSMC_A10
  PG1   ------> FSMC_A11
  PE7   ------> FSMC_D4
  PE8   ------> FSMC_D5
  PE9   ------> FSMC_D6
  PE10   ------> FSMC_D7
  PE11   ------> FSMC_D8
  PE12   ------> FSMC_D9
  PE13   ------> FSMC_D10
  PE14   ------> FSMC_D11
  PE15   ------> FSMC_D12
  PD8   ------> FSMC_D13
  PD9   ------> FSMC_D14
  PD10   ------> FSMC_D15
  PD11   ------> FSMC_A16
  PD12   ------> FSMC_A17
  PD13   ------> FSMC_A18
  PD14   ------> FSMC_D0
  PD15   ------> FSMC_D1
  PG2   ------> FSMC_A12
  PG3   ------> FSMC_A13
  PG4   ------> FSMC_A14
  PG5   ------> FSMC_A15
  PD0   ------> FSMC_D2
  PD1   ------> FSMC_D3
  PD4   ------> FSMC_NOE
  PD5   ------> FSMC_NWE
  PD7   ------> FSMC_NE1
  PG9   ------> FSMC_NE2
  PG10   ------> FSMC_NE3
  */

  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_9|GPIO_PIN_10);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_7);

  /* USER CODE BEGIN FSMC_MspDeInit 1 */

  /* USER CODE END FSMC_MspDeInit 1 */
}

void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* sramHandle){
  /* USER CODE BEGIN SRAM_MspDeInit 0 */

  /* USER CODE END SRAM_MspDeInit 0 */
  HAL_FSMC_MspDeInit();
  /* USER CODE BEGIN SRAM_MspDeInit 1 */

  /* USER CODE END SRAM_MspDeInit 1 */
}
