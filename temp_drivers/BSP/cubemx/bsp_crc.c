#include "system_err.h"
#include "bsp_crc.h"
#include "bsp.h"

CRC_HandleTypeDef hcrc;


void bsp_crc_init(void)
{
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    ERROR_PRINTF("bsp_crc_init");
  }

}

void HAL_CRC_MspInit(CRC_HandleTypeDef* crcHandle)
{

  if(crcHandle->Instance==CRC)
  {
    __HAL_RCC_CRC_CLK_ENABLE();
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* crcHandle)
{

  if(crcHandle->Instance==CRC)
  {
    __HAL_RCC_CRC_CLK_DISABLE();

  }
}


uint32_t bsp_crc32_hw_with_padding(const uint8_t *data, size_t len)
{
  size_t word_count = len / 4;
  size_t remain = len % 4;

  uint32_t crc;

  // Step 1: 정렬된 부분 (4바이트 단위)
  crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)data, word_count);

  // Step 2: 남은 바이트 처리 (0패딩)
  if (remain > 0)
  {
    uint32_t last = 0;
    for (size_t i = 0; i < remain; i++)
    {
      ((uint8_t *)&last)[i] = data[word_count * 4 + i];
    }

    crc = HAL_CRC_Accumulate(&hcrc, &last, 1);
  }

  return crc;
}
