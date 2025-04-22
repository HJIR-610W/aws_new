#include "system_err.h"

#include "crc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CRC_HandleTypeDef hcrc;

/* CRC init function */
void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

void HAL_CRC_MspInit(CRC_HandleTypeDef* crcHandle)
{

  if(crcHandle->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspInit 0 */

  /* USER CODE END CRC_MspInit 0 */
    /* CRC clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
  /* USER CODE BEGIN CRC_MspInit 1 */

  /* USER CODE END CRC_MspInit 1 */
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* crcHandle)
{

  if(crcHandle->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspDeInit 0 */

  /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
  /* USER CODE BEGIN CRC_MspDeInit 1 */

  /* USER CODE END CRC_MspDeInit 1 */
  }
}



uint32_t crc32_hw_with_padding(const uint8_t *data, size_t len)
{
  size_t word_count = len / 4;
  size_t remain = len % 4;

  uint32_t crc;

  // Step 1: 정렬된 부분 (4바이트 단위)
  crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)data, word_count);

  // Step 2: 남은 바이트 처리 (0패딩)
  if (remain > 0) {
    uint32_t last = 0;
    for (size_t i = 0; i < remain; i++)
    {
      ((uint8_t*)&last)[i] = data[word_count * 4 + i];
    }

    crc = HAL_CRC_Accumulate(&hcrc, &last, 1);
  }

  return crc;
}
