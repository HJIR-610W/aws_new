
#include "pcb_define.h"
#include "mcu_utile.h"


#if (AWS_PCB_VE==1)
void driver_stm32_bsp_init(void)
{


  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, OUT_RV8803_EVI_Pin|OUT_EX_UART_RST_A_Pin|OUT_EX_UART_RST_B_Pin, GPIO_PIN_SET);

   HAL_GPIO_WritePin(GPIOE, OUT_RV8803_EVI_Pin, GPIO_PIN_RESET);
    
    
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, OUT_CON_PWR_ASEN_C_PIN|OUT_CON_PWR_ASEN_D_PIN, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, OUT_CON_PWR_ASEN_PIN|OUT_CON_PWR_ASEN_A_PIN|OUT_CON_PWR_ASEN_B_PIN, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, OUT_ETH_RST_PHY_Pin|OUT_PWR_CDMA_PIN|OUT_ADC_EN_RTD_Pin|OUT_ADC_EN_ODD_Pin
                          |OUT_ADC_EN_EVEN_Pin|OUT_ADC_SEL_A1_Pin|OUT_ADC_SEL_A2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, OUT_SYS_RUN_Pin|GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, OUT_DIR_RS485_A_PIN|OUT_DIR_RS485_B_PIN|OUT_DIR_SDI_PIN|OUT_NOR_RESET_Pin
                          |OUT_CON_PWR_232_A_PIN|OUT_CON_PWR_232_B_PIN, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OUT_SPI2_NSS_GPIO_Port, OUT_SPI2_NSS_PIN, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OUT_SPI1_NSS_GPIO_Port, OUT_SPI1_NSS_PIN, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OUT_CON_PWR_DSEN_GPIO_Port, OUT_CON_PWR_DSEN_PIN, GPIO_PIN_SET);


  HAL_GPIO_WritePin(OUT_FLASH_CS_GPIO_Port, OUT_FLASH_CS_PIN, GPIO_PIN_SET);



  HAL_GPIO_WritePin(GPIOB, OUT_CON_PWR_485_PIN|CON_PWR_TC_PIN, GPIO_PIN_SET);




 GPIO_InitStruct.Pin = OUT_FLASH_CS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OUT_FLASH_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PEPin PEPin */
  GPIO_InitStruct.Pin = NOT_USED_PE2_Pin|NOT_USED_PE6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PEPin PEPin PEPin */
  GPIO_InitStruct.Pin = OUT_RV8803_EVI_Pin|OUT_EX_UART_RST_A_Pin|OUT_EX_UART_RST_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PIPin PIPin PIPin PIPin
                           PIPin PIPin PIPin PIPin */
  GPIO_InitStruct.Pin = IN_EX_UART_INT_5_Pin|IN_EX_UART_INT_6_Pin|IN_EX_UART_INT_7_Pin|IN_EX_UART_INT_8_Pin
                          |IN_EX_UART_INT_1_Pin|IN_EX_UART_INT_2_Pin|IN_EX_UART_INT_3_Pin|IN_EX_UART_INT_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = NOT_USED_PC13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(NOT_USED_PC13_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PFPin PFPin */
  GPIO_InitStruct.Pin = OUT_CON_PWR_ASEN_C_PIN|OUT_CON_PWR_ASEN_D_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PCPin PCPin PCPin */
  GPIO_InitStruct.Pin = OUT_CON_PWR_ASEN_PIN|OUT_CON_PWR_ASEN_A_PIN|OUT_CON_PWR_ASEN_B_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PHPin PHPin PHPin PHPin
                           PHPin PHPin PH13 PHPin
                           PHPin */
  GPIO_InitStruct.Pin = OUT_ETH_RST_PHY_Pin|OUT_PWR_CDMA_PIN|OUT_SYS_RUN_Pin|OUT_ADC_EN_RTD_Pin
                          |OUT_ADC_EN_ODD_Pin|OUT_ADC_EN_EVEN_Pin|GPIO_PIN_13|OUT_ADC_SEL_A1_Pin
                          |OUT_ADC_SEL_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : PH4 PH5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin */
  GPIO_InitStruct.Pin = NOT_USED_PA5_Pin|NOT_USED_PA11_Pin|IN_SPI2_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = INT_D_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = IN_BOOT1_Pin|IN_DI_STATUS_BTM_Pin|DO_BTM_PWRC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PH7 PH8 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : PGPin PGPin PGPin PGPin
                           PGPin PGPin */
  GPIO_InitStruct.Pin = OUT_DIR_RS485_A_PIN|OUT_DIR_RS485_B_PIN|OUT_DIR_SDI_PIN|OUT_NOR_RESET_Pin
                          |OUT_CON_PWR_232_A_PIN|OUT_CON_PWR_232_B_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = OUT_SPI2_NSS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OUT_SPI2_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = OUT_SPI1_NSS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OUT_SPI1_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = IN_SDIO_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IN_SDIO_DETECT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = OUT_CON_PWR_DSEN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OUT_CON_PWR_DSEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = INT_RTC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT_RTC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = OUT_CON_PWR_485_PIN|CON_PWR_TC_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
 // HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  //HAL_NVIC_EnableIRQ(EXTI0_IRQn);

 // HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
 // HAL_NVIC_EnableIRQ(EXTI1_IRQn);

 // HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
 // HAL_NVIC_EnableIRQ(EXTI4_IRQn);

 // HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
 // HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

 // HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
 // HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);


     HAL_GPIO_WritePin(GPIOE, OUT_EX_UART_RST_A_Pin|OUT_EX_UART_RST_B_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
  
  HAL_GPIO_WritePin(GPIOE, OUT_EX_UART_RST_A_Pin|OUT_EX_UART_RST_B_Pin, GPIO_PIN_RESET);



}
#endif


#if (AWS_PCB_VER==3)
void driver_stm32_bsp_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();



  //QUAD UART 칩을 리셋해준다. H->L 
  board_set_gpio(EX_UART_RST_A_GPIO_Port, EX_UART_RST_A_PIN, GPIO_PIN_SET);
  board_config_gpio(EX_UART_RST_A_GPIO_Port,EX_UART_RST_A_PIN,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);

  board_set_gpio(EX_UART_RST_B_GPIO_Port, EX_UART_RST_B_PIN, GPIO_PIN_SET);
  board_config_gpio(EX_UART_RST_B_GPIO_Port,EX_UART_RST_B_PIN,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);
  HAL_Delay(10);

  board_set_gpio(EX_UART_RST_A_GPIO_Port, EX_UART_RST_A_PIN, GPIO_PIN_RESET);
  board_set_gpio(EX_UART_RST_B_GPIO_Port, EX_UART_RST_B_PIN, GPIO_PIN_RESET);
 
  //RS485 A,B의 방향을 입력으로 설정한다.

  board_set_gpio(OUT_DIR_RS485_A_GPIO_Port, OUT_DIR_RS485_A_PIN, GPIO_PIN_RESET);
  board_config_gpio(OUT_DIR_RS485_A_GPIO_Port,OUT_DIR_RS485_A_PIN,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);

  board_set_gpio(OUT_DIR_RS485_B_GPIO_Port, OUT_DIR_RS485_B_PIN, GPIO_PIN_RESET);
  board_config_gpio(OUT_DIR_RS485_B_GPIO_Port,OUT_DIR_RS485_B_PIN,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);


  board_set_gpio(OUT_SPI1_NSS_GPIO_Port, OUT_SPI1_NSS_PIN, GPIO_PIN_SET);
  board_config_gpio(OUT_SPI1_NSS_GPIO_Port,OUT_SPI1_NSS_PIN,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);


  board_set_gpio(OUT_SPI1_CS_RTC_GPIO_Port, OUT_RV8803_EVI_Pin, GPIO_PIN_SET);
  board_config_gpio(OUT_SPI1_CS_RTC_GPIO_Port,OUT_RV8803_EVI_Pin,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);

  board_set_gpio(OUT_SPI2_NSS_GPIO_Port, OUT_SPI2_NSS_PIN, GPIO_PIN_RESET);
  board_config_gpio(OUT_SPI2_NSS_GPIO_Port,OUT_SPI2_NSS_PIN,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);
  
  board_set_gpio(OUT_SPI2_NSS_GPIO_Port, OUT_SPI2_NSS_PIN, GPIO_PIN_RESET);
  board_config_gpio(OUT_SPI2_NSS_GPIO_Port,OUT_SPI2_NSS_PIN,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);


  board_set_gpio(DO_SEL_IF_UART_GPIO_Port, SEL_IF_UART_Pin, GPIO_PIN_RESET);//기본은 UART로 사용
  board_config_gpio(DO_SEL_IF_UART_GPIO_Port,SEL_IF_UART_Pin,GPIO_MODE_OUTPUT_OD,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);

  board_set_gpio(DO_CON_PWR_S24_GPIO_Port, DO_CON_PWR_S24_Pin, GPIO_PIN_RESET);//24V
  board_config_gpio(DO_CON_PWR_S24_GPIO_Port,DO_CON_PWR_S24_Pin,GPIO_MODE_OUTPUT_OD,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);


  board_set_gpio(DO_RESET_H_GPIO_Port, DO_RESET_H_Pin, GPIO_PIN_RESET);//24V
  board_config_gpio(DO_RESET_H_GPIO_Port,DO_RESET_H_Pin,GPIO_MODE_OUTPUT_OD,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);

  board_set_gpio(USB_OTG_FS_SOF_GPIO_Port, USB_OTG_FS_SOF_Pin, GPIO_PIN_SET);//VBUS 비활성
  board_config_gpio(USB_OTG_FS_SOF_GPIO_Port,USB_OTG_FS_SOF_Pin,GPIO_MODE_OUTPUT_OD,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);

  //미사용
  board_config_gpio(USB_OTG_PWR_FAIL_GPIO_Port,USB_OTG_PWR_FAIL_Pin,GPIO_MODE_INPUT,GPIO_NOPULL,GPIO_SPEED_FREQ_LOW,0);

 }

#endif

#if (AWS_PCB_VER == 5)
 void driver_stm32_bsp_init(void)
 {
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   __HAL_RCC_GPIOA_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_GPIOE_CLK_ENABLE();
   __HAL_RCC_GPIOF_CLK_ENABLE();
   __HAL_RCC_GPIOG_CLK_ENABLE();
   __HAL_RCC_GPIOH_CLK_ENABLE();
   __HAL_RCC_GPIOI_CLK_ENABLE();

   // QUAD UART 칩을 리셋해준다. H->L
   board_set_gpio(EX_UART_RST_A_GPIO_Port, EX_UART_RST_A_PIN, GPIO_PIN_SET);
   board_config_gpio(EX_UART_RST_A_GPIO_Port, EX_UART_RST_A_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);

   board_set_gpio(EX_UART_RST_B_GPIO_Port, EX_UART_RST_B_PIN, GPIO_PIN_SET);
   board_config_gpio(EX_UART_RST_B_GPIO_Port, EX_UART_RST_B_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);
   HAL_Delay(10);

   board_set_gpio(EX_UART_RST_A_GPIO_Port, EX_UART_RST_A_PIN, GPIO_PIN_RESET);
   board_set_gpio(EX_UART_RST_B_GPIO_Port, EX_UART_RST_B_PIN, GPIO_PIN_RESET);

   //[RS485] 방향을 입력으로 설정한다.
   board_set_gpio(OUT_DIR_RS485_A_GPIO_Port, OUT_DIR_RS485_A_PIN, GPIO_PIN_RESET);
   board_config_gpio(OUT_DIR_RS485_A_GPIO_Port, OUT_DIR_RS485_A_PIN, GPIO_MODE_OUTPUT_PP,
                     GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

   board_set_gpio(OUT_DIR_RS485_B_GPIO_Port, OUT_DIR_RS485_B_PIN, GPIO_PIN_RESET);
   board_config_gpio(OUT_DIR_RS485_B_GPIO_Port, OUT_DIR_RS485_B_PIN, GPIO_MODE_OUTPUT_PP,
                     GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

   board_set_gpio(OUT_RS485_DIR_C_GPIO_Port, OUT_RS485_DIR_C_PIN, GPIO_PIN_RESET);
   board_config_gpio(OUT_RS485_DIR_C_GPIO_Port, OUT_RS485_DIR_C_PIN, GPIO_MODE_OUTPUT_PP,
                     GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

   board_set_gpio(OUT_RS485_DIR_D_GPIO_Port, OUT_RS485_DIR_D_PIN, GPIO_PIN_RESET);
   board_config_gpio(OUT_RS485_DIR_D_GPIO_Port, OUT_RS485_DIR_D_PIN, GPIO_MODE_OUTPUT_PP,
                     GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

  //[SPI CS] HIGH로 한다.
   board_set_gpio(OUT_SPI1_NSS_GPIO_Port, OUT_SPI1_NSS_PIN, GPIO_PIN_SET);
   board_config_gpio(OUT_SPI1_NSS_GPIO_Port, OUT_SPI1_NSS_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);

   board_set_gpio(OUT_SPI1_CS_RTC_GPIO_Port, OUT_RV8803_EVI_Pin, GPIO_PIN_SET);
   board_config_gpio(OUT_SPI1_CS_RTC_GPIO_Port, OUT_RV8803_EVI_Pin, GPIO_MODE_OUTPUT_PP,
                     GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

   board_set_gpio(OUT_SPI2_NSS_GPIO_Port, OUT_SPI2_NSS_PIN, GPIO_PIN_RESET);
   board_config_gpio(OUT_SPI2_NSS_GPIO_Port, OUT_SPI2_NSS_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);

   // UART_5_EXT_D 는 HART와 RS232 선택사용 포트이다. RS232를 기본설정한다.
   board_set_gpio(DO_SEL_IF_UART_GPIO_Port, SEL_IF_UART_Pin, GPIO_PIN_RESET);
   board_config_gpio(DO_SEL_IF_UART_GPIO_Port, SEL_IF_UART_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);

   //[전원]CDMA 12V 전원은 차단한다.
   board_set_gpio(OUT_PWR_CDMA_GPIO_Port, OUT_PWR_CDMA_PIN, GPIO_PIN_RESET);
   board_config_gpio(OUT_PWR_CDMA_GPIO_Port, OUT_PWR_CDMA_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);

   //[전원]HART 24V는 차단한다.
   board_set_gpio(DO_CON_PWR_S24_GPIO_Port, DO_CON_PWR_S24_Pin, GPIO_PIN_RESET);
   board_config_gpio(DO_CON_PWR_S24_GPIO_Port, DO_CON_PWR_S24_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);
   //[전원]강우감지 전원은 항상 출력
   board_set_gpio(DO_CON_PWR_RAIN_DECT_ACTIVE_H_GPIO_Port, DO_CON_PWR_RAIN_DECT_ACTIVE_H_PIN,
                  GPIO_PIN_SET);
   board_config_gpio(DO_CON_PWR_RAIN_DECT_ACTIVE_H_GPIO_Port, DO_CON_PWR_RAIN_DECT_ACTIVE_H_PIN,
                     GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

   // HART IC를 RESET 상태로 만든다.
   board_set_gpio(DO_RESET_H_GPIO_Port, DO_RESET_H_Pin, GPIO_PIN_RESET);
   board_config_gpio(DO_RESET_H_GPIO_Port, DO_RESET_H_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);

   board_set_gpio(USB_OTG_FS_SOF_GPIO_Port, USB_OTG_FS_SOF_Pin, GPIO_PIN_SET);  // VBUS 비활성
   board_config_gpio(USB_OTG_FS_SOF_GPIO_Port, USB_OTG_FS_SOF_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);

   // 미사용
   board_config_gpio(USB_OTG_PWR_FAIL_GPIO_Port, USB_OTG_PWR_FAIL_Pin, GPIO_MODE_INPUT, GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW, 0);
 }

#endif