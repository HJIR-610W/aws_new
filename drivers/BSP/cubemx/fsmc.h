/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : FSMC.h
  * Description        : This file provides code for the configuration
  *                      of the FSMC peripheral.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FSMC_H
#define __FSMC_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pcb_define.h"
#include "system_err.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SRAM_HandleTypeDef hsram1;
extern SRAM_HandleTypeDef hsram2;
extern SRAM_HandleTypeDef hsram_lcd;  // ST7920 LCD Controller

/* USER CODE BEGIN Private defines */

// ST7920 LCD Controller FSMC Address Definitions
#define ST7920_LCD_BASE_ADDR    0x60000000UL    // NE1 Bank base address
#define ST7920_LCD_CMD_ADDR     (ST7920_LCD_BASE_ADDR | 0x00)  // Command/Status register
#define ST7920_LCD_DATA_ADDR    (ST7920_LCD_BASE_ADDR | 0x02)  // Data register

/* USER CODE END Private defines */

void MX_FSMC_Init(void);
void HAL_SRAM_MspInit(SRAM_HandleTypeDef* hsram);
void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* hsram);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__FSMC_H */

/**
  * @}
  */

/**
  * @}
  */
