

#include "bsp_spi.h"
#include "pcb_define.h"
#include "system_err.h"
#include "os_user_def.h"

#define SPI_TIME_OUT 0x1000

typedef struct spi_instance_s
{
  SPI_HandleTypeDef spi;
  void *sem;
  bool opened;
} spi_instance_t;

spi_instance_t spi_inst[BSP_SPI_MAX] = {[BSP_SPI_1] = {.spi.Instance = SPI1}, [BSP_SPI_2]={.spi.Instance = SPI2}};

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  

  if (hspi->Instance == SPI1)
  {
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(SPI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
  }
  else if (hspi->Instance == SPI2)
  {
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{

  if (hspi->Instance == SPI1)
  {
    __HAL_RCC_SPI1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
  }
  else if (hspi->Instance == SPI2)
  {
    __HAL_RCC_SPI2_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
  }
}

uint32_t getSPI1ClockFrequency(void)
{
  uint32_t systemClock = HAL_RCC_GetSysClockFreq();             // 시스템 클럭 가져오기
  uint32_t apb2Prescaler = (RCC->CFGR & RCC_CFGR_PPRE2) >> 13;  // APB2 프리스케일러 추출

  // APB2 프리스케일러 값에 따라 나눗셈 설정
  uint32_t apb2Divider;
  if (apb2Prescaler < 4)
  {
    apb2Divider = 1;  // 프리스케일러 값이 0b000(분주 없음)일 때
  }
  else
  {
    apb2Divider = 2 << (apb2Prescaler - 4);  // 프리스케일러 값이 0b100(분주 시작)부터
  }

  uint32_t apb2Clock = systemClock / apb2Divider;  // APB2 클럭 계산
  return apb2Clock;                                // SPI1의 메인 클럭 속도 반환
}

uint32_t getSPI2ClockFrequency(void)
{
  uint32_t systemClock = HAL_RCC_GetSysClockFreq();             // 시스템 클럭 가져오기
  uint32_t apb1Prescaler = (RCC->CFGR & RCC_CFGR_PPRE1) >> 10;  // APB1 프리스케일러 추출

  // APB1 프리스케일러 값에 따라 나눗셈 설정
  uint32_t apb1Divider;
  if (apb1Prescaler < 4)
  {
    apb1Divider = 1;  // 프리스케일러 값이 0b000(분주 없음)일 때
  }
  else
  {
    apb1Divider = 2 << (apb1Prescaler - 4);  // 프리스케일러 값이 0b100(분주 시작)부터
  }

  uint32_t apb1Clock = systemClock / apb1Divider;  // APB1 클럭 계산
  return apb1Clock;                                // SPI2의 메인 클럭 속도 반환
}

// 사용자가 원하는 SPI 클럭에 맞는 baudRatePrescaler 값을 반환하는 함수
uint32_t getBaudRatePrescaler(uint32_t desiredSpiClock, uint32_t pclk)
{
  // 프리스케일러 값 테이블 (STM32 HAL에서 사용되는 값)
  const uint32_t prescalers[] = {SPI_BAUDRATEPRESCALER_2,   SPI_BAUDRATEPRESCALER_4,
                                 SPI_BAUDRATEPRESCALER_8,   SPI_BAUDRATEPRESCALER_16,
                                 SPI_BAUDRATEPRESCALER_32,  SPI_BAUDRATEPRESCALER_64,
                                 SPI_BAUDRATEPRESCALER_128, SPI_BAUDRATEPRESCALER_256};

  // 실제 분주 값 테이블
  const uint32_t actualDivisors[] = {2, 4, 8, 16, 32, 64, 128, 256};

  for (int i = 0; i < 8; i++)
  {
    // PCLK를 현재 분주 값으로 나눈 SPI 클럭 속도
    uint32_t calculatedSpiClock = pclk / actualDivisors[i];
    if (calculatedSpiClock <= desiredSpiClock)
    {
      return prescalers[i];  // 적절한 prescaler 반환
    }
  }

  // 원하는 SPI 클럭 속도가 너무 낮을 경우 최대 프리스케일러 반환
  return SPI_BAUDRATEPRESCALER_256;
}

uint32_t get_spi_prescaler(SPI_HandleTypeDef *hspi, uint32_t freq)
{
  uint32_t pclk;
  uint32_t prescale;

  if (hspi->Instance == SPI1)
  {
    pclk = getSPI1ClockFrequency();
    prescale = getBaudRatePrescaler(freq, pclk);
  }
  else if (hspi->Instance == SPI2)
  {
    pclk = getSPI2ClockFrequency();
    prescale = getBaudRatePrescaler(freq, pclk);
  }

  return prescale;
}

// SPI최대 클럭은 동작클럭의 절반
void stm32_spi_init(int num)
{
  SPI_HandleTypeDef *hspi;

  hspi = &spi_inst[num].spi;


  if (hspi->Instance == SPI1)
  {
    hspi->Init.Mode = SPI_MODE_MASTER;
    hspi->Init.Direction = SPI_DIRECTION_2LINES;
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi->Init.NSS = SPI_NSS_SOFT;
    hspi->Init.BaudRatePrescaler = get_spi_prescaler(hspi, 1312500);
    ;  // 고정된 분주비라서 원하는데오 딱 안떨어짐1312500
    hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi->Init.TIMode = SPI_TIMODE_DISABLE;
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi->Init.CRCPolynomial = 10;

  }
  else if (hspi->Instance == SPI2)
  {
    hspi->Instance = SPI2;
    hspi->Init.Mode = SPI_MODE_MASTER;
    hspi->Init.Direction = SPI_DIRECTION_2LINES;
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi->Init.NSS = SPI_NSS_SOFT;
    hspi->Init.BaudRatePrescaler = get_spi_prescaler(hspi, 1000000);
    ;
    hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi->Init.TIMode = SPI_TIMODE_DISABLE;
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi->Init.CRCPolynomial = 10;
  }
  if (HAL_SPI_Init(hspi) != HAL_OK)
  {
    ERROR_PRINTF("spi %d",num);
  }
}



void bsp_spi_init(int num)
{
  if(spi_inst[num].opened)
  {
    return ;
  }

  stm32_spi_init(num);

  OS_CREATE_BINARY_SEM(spi_inst[num].sem);

  spi_inst[num].opened = true;
  

}

void bsp_spi_send_byte(int num, uint8_t value)
{
  HAL_StatusTypeDef status = HAL_OK;
  SPI_HandleTypeDef *spiHandle;
  spiHandle = &spi_inst[num].spi;

  status = HAL_SPI_Transmit(spiHandle, (uint8_t *)&value, 1, SPI_TIME_OUT);

  if (status != HAL_OK)
  {
    ERROR_PRINTF("stm32_spi_send_byte %d", status);
  }
}

void bsp_spi_send_bytes(int num, uint8_t *data, uint16_t dataLen)
{
  HAL_StatusTypeDef status;
  SPI_HandleTypeDef *spiHandle;
  spiHandle = &spi_inst[num].spi;

  status =
      HAL_SPI_Transmit(spiHandle, (uint8_t *)data, dataLen, SPI_TIME_OUT);

  if (status != HAL_OK)
  {
    ERROR_PRINTF("stm32_spi_send_bytes %d", status);
  }
}

uint8_t bsp_spi_read_byte(int num)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t readvalue = 0x00;
  SPI_HandleTypeDef *spiHandle;
  spiHandle = &spi_inst[num].spi;

  status =
      HAL_SPI_Receive(spiHandle, (uint8_t *)&readvalue, 1, SPI_TIME_OUT);

  if (status != HAL_OK)
  {
    ERROR_PRINTF("stm32_spi_read_byte %d", status);
  }

  return readvalue;
}

uint8_t bsp_spi_read_bytes(int num, uint8_t *p_buff, uint16_t read_len)
{
  HAL_StatusTypeDef status = HAL_OK;
  SPI_HandleTypeDef *spiHandle;
  spiHandle = &spi_inst[num].spi;

  status =
      HAL_SPI_Receive(spiHandle, (uint8_t *)p_buff, read_len, SPI_TIME_OUT);

  if (status != HAL_OK)
  {
    ERROR_PRINTF("stm32_spi_read_bytes %d", status);
    return 1;
  }

  return 0;
}

void bsp_spi_pend_sem(int num)
{ 
  OS_PEND_SEM(spi_inst[num].sem, osWaitForever); 
}

void bsp_spi_post_sem(int num)
{ 
  OS_POST_SEM(spi_inst[num].sem);
}

void SPI1_IRQHandler(void) { HAL_SPI_IRQHandler(&spi_inst[BSP_SPI_1].spi); }

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_RxCpltCallback should be implemented in the user file
   */
}
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_TxRxCpltCallback should be implemented in the user file
   */
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_TxCpltCallback should be implemented in the user file
   */
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_ErrorCallback should be implemented in the user file
   */
  /* NOTE : The ErrorCode parameter in the hspi handle is updated by the SPI processes
            and user can use HAL_SPI_GetError() API to check the latest error occurred
   */
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hspi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_AbortCpltCallback can be implemented in the user file.
   */
}