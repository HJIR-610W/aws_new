/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : fatfs_platform.c
  * @brief          : fatfs_platform source file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
*/
/* USER CODE END Header */
#include "fatfs_platform.h"

uint8_t g_sd_inserted=0;
uint8_t	BSP_PlatformIsDetected(void)
{
    uint8_t status = SD_PRESENT;

    if (HAL_GPIO_ReadPin(DI_SDIO_DETECT_GPIO_Port, DI_SDIO_DETECT_Pin) != GPIO_PIN_RESET)
    {
      g_sd_inserted = 0;
      status = SD_NOT_PRESENT;
    }
    else
    {
      g_sd_inserted = 1;
    }

    return status;
}
