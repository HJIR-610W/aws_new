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
  /*
  ST7920 parallel mode
  RS     : 0:명령어(0x60000000) ,1:데이타  (0x60000001)  주소 0bit를 RS신호로 사용
  R/W    : 읽기 쓰기 현재 회로는 쓰기만 사용 하드웨어로  GND에 연결되어있음
  E      : Enable trigger NE1 OR WE신호를 NOT해서 E신호로 사용
  DB0-DB7: 데이터

 FSMC MODE A write accesses
 A[25:0]
 NBL[1:0]
 NEx    ─┐                      ┌────
          └───────────┘
 NOE
 NWE    ─────────┐    ┌────
                          └──┘

 D[15:0] ─────────########───



  HCLK가 168MHZ로 동작한다. 따라서 fmsc 1클럭은 5.95ns 이다
  NE address setup time + data setup time 160ns  실측
  WE data setup time 119ns

   ST7920 LCD 타이밍 설정 (비동기 모드) */

  // 어드레스 설정 시간: NEx 신호가 active된 후 읽기/쓰기 신호(NOE/NWE)가 active되기 전까지의 시간
  // 값: 5 HCLK 사이클 = 3 × 5.95ns = 17.85ns
  Timing.AddressSetupTime = 3;

  // 어드레스 유지 시간: 읽기/쓰기 신호가 inactive된 후 어드레스가 유지되는 시간
  // 비동기 모드에서는 사용되지 않지만, 0으로 설정 시 HAL 드라이버에서 assert 발생
  // 값: 15 HCLK 사이클 (실제 동작에는 영향 없음)
  Timing.AddressHoldTime = 15;

  // 데이터 설정 시간: NOE/NWE 신호가 active된 후 데이터가 유효하게 유지되어야 하는 시간
  // 값: 20 HCLK 사이클 = 4 × 5.95ns = 23.8ns
  // ST7920의 데이터 읽기/쓰기 타이밍을 충족시키기 위한 설정
  Timing.DataSetupTime = 4;

  // 버스 턴어라운드 시간: 연속된 읽기/쓰기 동작 사이의 대기 시간
  // 값: 5 HCLK 사이클 = 14 × 5.95ns = 83.8ns
  // 버스 신호가 안정화되기 위한 여유 시간
  Timing.BusTurnAroundDuration = 14;

  // 클록 분주비: 동기 모드에서만 사용되며, 비동기 모드에서는 무시됨
  // 0으로 설정 시 assert 발생하므로 유효한 값 설정
  Timing.CLKDivision = 16;

  // 데이터 지연: 동기 모드에서 클록과 데이터 간의 지연 시간
  // 비동기 모드에서는 무시됨
  Timing.DataLatency = 17;

  // 액세스 모드: FSMC가 외부 메모리에 접근하는 방식 정의
  // Mode A: 기본 비동기 모드 (가장 일반적)
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

  // 외부 SRAM 타이밍 측정값:
  // NE address setup time + data setup time: 22ns 실측 (1+1) × 5.95ns
  // WE data setup time: 11.9ns 실측 (2 × 5.95ns)
  // OE (Output Enable): 18ns 실측 (1+2) × 5.95ns = 18ns

  /* 외부 SRAM 타이밍 설정 (비동기 모드) - 고속 액세스를 위한 최적화 */

  // 어드레스 설정 시간: SRAM이 어드레스를 인식하기까지의 시간
  // 값: 1 HCLK 사이클 = 1 × 5.95ns = 5.95ns
  // 고속 SRAM이므로 최소값 사용
  Timing.AddressSetupTime       = 1;

  // 어드레스 유지 시간: 읽기/쓰기 신호 해제 후 어드레스 유지 시간
  // 값: 1 HCLK 사이클 = 1 × 5.95ns = 5.95ns
  // 고속 SRAM의 빠른 응답 특성에 맞춰 최소값 설정
  Timing.AddressHoldTime        = 1;

  // 데이터 설정 시간: 데이터 읽기/쓰기에 필요한 시간
  // 값: 3 HCLK 사이클 = 3 × 5.95ns = 17.85ns
  // SRAM의 액세스 타임을 충족시키는 값
  Timing.DataSetupTime          = 3;

  // 버스 턴어라운드 시간: 연속된 메모리 액세스 사이의 대기 시간
  // 값: 1 HCLK 사이클 = 1 × 5.95ns = 5.95ns
  // 고속 연속 액세스를 위해 최소값 설정
  Timing.BusTurnAroundDuration  = 1;

  // 클록 분주비: 동기 모드에서만 사용되며, 비동기 모드에서는 무시됨
  // 0 설정 시 assert 발생하므로 유효한 값 설정
  Timing.CLKDivision            = 2;

  // 데이터 지연: 동기 모드에서만 사용되며, 비동기 모드에서는 무시됨
  Timing.DataLatency            = 2;

  // 액세스 모드: FSMC가 외부 메모리에 접근하는 방식 정의
  // Mode A: 기본 비동기 모드
  Timing.AccessMode             = FSMC_ACCESS_MODE_A;

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

  /* QUAD UART 타이밍 설정 (비동기 모드) */

  /* 읽기(Read) 타이밍 설정 */

  // 어드레스 설정 시간: UART 컨트롤러가 어드레스를 인식하기까지의 시간
  // 값: 4 HCLK 사이클 = 4 × 5.95ns = 23.8ns
  Timing.AddressSetupTime = 4;

  // 어드레스 유지 시간: 읽기 신호 해제 후 어드레스 유지 시간
  // 값: 1 HCLK 사이클 = 1 × 5.95ns = 5.95ns
  Timing.AddressHoldTime  = 1;

  // 데이터 설정 시간: UART 레지스터 읽기에 필요한 시간
  // 값: 15 HCLK 사이클 = 15 × 5.95ns = 89.25ns
  // UART 컨트롤러의 응답 시간을 고려한 설정
  Timing.DataSetupTime    = 15;

  // 버스 턴어라운드 시간: 연속된 UART 레지스터 액세스 사이의 대기 시간
  // 값: 10 HCLK 사이클 = 10 × 5.95ns = 59.5ns
  // UART의 안정적인 동작을 위한 충분한 대기 시간
  Timing.BusTurnAroundDuration = 10;

  // 액세스 모드: 기본 비동기 모드
  Timing.AccessMode = FSMC_ACCESS_MODE_A;

  /* 쓰기(Write) 타이밍 설정 - ExtTiming */
  // 확장 타이밍을 사용하여 읽기와 쓰기 타이밍을 독립적으로 설정

  // 어드레스 설정 시간 (쓰기): UART 레지스터 쓰기 시 어드레스 인식 시간
  // 값: 4 HCLK 사이클 = 4 × 5.95ns = 23.8ns
  ExtTiming.AddressSetupTime      = 4;

  // 어드레스 유지 시간 (쓰기): 쓰기 신호 해제 후 어드레스 유지 시간
  // 값: 1 HCLK 사이클 = 1 × 5.95ns = 5.95ns
  // 0으로 설정 시 HAL 드라이버에서 assert 발생
  ExtTiming.AddressHoldTime       =  1;

  // 데이터 설정 시간 (쓰기): UART 레지스터 쓰기에 필요한 시간
  // 값: 10 HCLK 사이클 = 10 × 5.95ns = 59.5ns
  // 읽기(15)보다 짧게 설정하여 쓰기 성능 최적화
  ExtTiming.DataSetupTime         = 10;

  // 버스 턴어라운드 시간 (쓰기): 연속된 쓰기 동작 사이의 대기 시간
  // 값: 10 HCLK 사이클 = 10 × 5.95ns = 59.5ns
  ExtTiming.BusTurnAroundDuration = 10;

  // 클록 분주비: 비동기 모드에서는 무시됨, 0 설정 시 assert 발생
  Timing.CLKDivision              = 2;

  // 데이터 지연: 비동기 모드에서는 무시됨
  Timing.DataLatency              = 2;
 

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
