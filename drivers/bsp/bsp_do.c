#include "bsp.h"
#include "bsp_di.h"
#include "pcf8575.h"
#include "util_memory.h"
#include "pcb_define.h"
#include "dev_io.h"




typedef struct bsp_do_inst_s
{
  bool opened;
  GPIO_InitTypeDef init;
  GPIO_TypeDef *port;
  GPIO_PinState init_state;
} bsp_do_inst_t;

bsp_do_inst_t do_inst[BSP_DO_MAX] = {
    [BSP_DO_POWER_CDMA] = {.init = {.Pin = OUT_PWR_CDMA_PIN,
                                    .Mode = GPIO_MODE_OUTPUT_PP,
                                    .Pull = GPIO_NOPULL,
                                    .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = OUT_PWR_CDMA_GPIO_Port,
                           .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [BSP_DO_POWER_HART_24V] = {.init = {.Pin = DO_CON_PWR_S24_Pin,
                                        .Mode = GPIO_MODE_OUTPUT_PP,
                                        .Pull = GPIO_NOPULL,
                                        .Speed = GPIO_SPEED_FREQ_LOW},
                               .port = DO_CON_PWR_S24_GPIO_Port,
                               .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [BSP_DO_LCD_RESET] = {.init = {.Pin = DO_RESET_H_Pin,
                                   .Mode = GPIO_MODE_OUTPUT_PP,
                                   .Pull = GPIO_NOPULL,
                                   .Speed = GPIO_SPEED_FREQ_LOW},
                          .port = DO_RESET_H_GPIO_Port,
                          .init_state = GPIO_PIN_RESET},
    [BSP_DO_POWER_RAIN_DECT_DIGITAL] = {.init = {.Pin = DO_POWER_RAIN_DECT_DIGITAL_PIN,
                                                 .Mode = GPIO_MODE_OUTPUT_PP,
                                                 .Pull = GPIO_NOPULL,
                                                 .Speed = GPIO_SPEED_FREQ_LOW},
                                        .port = DO_POWER_RAIN_DECT_DIGITAL_GPIO_Port,
                                        .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [BSP_DO_POWER_RAIN_DECT_ANALOG] = {.init = {.Pin = DO_CON_PWR_RAIN_PIN,
                                                .Mode = GPIO_MODE_OUTPUT_PP,
                                                .Pull = GPIO_NOPULL,
                                                .Speed = GPIO_SPEED_FREQ_LOW},
                                       .port = DO_CON_PWR_RAIN_GPIO_Port,
                                       .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [BSP_DO_ADC_NCS] = {.init = {.Pin = OUT_SPI2_NSS_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_PULLUP, .Speed = GPIO_SPEED_FREQ_HIGH},
                        .port = OUT_SPI2_NSS_GPIO_Port,
                        .init_state = GPIO_PIN_SET}, /*비활성*/
    [BSP_DO_FRAM_CS] = {.init = {.Pin = DO_SPI1_NSS_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH},
                        .port = DO_SPI1_NSS_GPIO_Port,
                        .init_state = GPIO_PIN_SET}, /*비활성*/
    [BSP_DO_RTC_CS] = {.init = {.Pin = OUT_RV8803_EVI_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH},
                       .port = OUT_SPI1_CS_RTC_GPIO_Port,
                       .init_state = GPIO_PIN_SET}, /*비활성*/
    [BSP_DO_FLASH_CS] = {.init = {.Pin = NOR_RESET_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH},
                         .port = NOR_RESET_GPIO_Port,
                         .init_state = GPIO_PIN_SET}, /*비활성*/
    [BSP_DO_DIR_SDI] = {.init = {.Pin = OUT_DIR_SDI_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                        .port = OUT_DIR_SDI_GPIO_Port,
                        .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [BSP_DO_DIR_RS485_A] = {.init = {.Pin = OUT_DIR_RS485_A_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = OUT_DIR_RS485_A_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [BSP_DO_DIR_RS485_B] = {.init = {.Pin = OUT_DIR_RS485_B_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = OUT_DIR_RS485_B_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [BSP_DO_HART_SEL] = {.init = {.Pin = SEL_IF_UART_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                         .port = DO_SEL_IF_UART_GPIO_Port,
                         .init_state = GPIO_PIN_RESET}, /*RS232모드로 설정*/
    [BSP_DO_HART_RTS] = {.init = {.Pin = DO_RTS_H_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                         .port = DO_RTS_H_GPIO_Port,
                         .init_state = GPIO_PIN_SET},//수신모드
    [BSP_DO_HART_RESET] = {.init = {.Pin = DO_RESET_H_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_RESET_H_GPIO_Port,
                           .init_state = GPIO_PIN_SET},//활성
    [BSP_DO_DIR_RS485_RS232_C] = {.init = {.Pin = OUT_RS485_RS232_DIR_C_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                                  .port = OUT_RS485_RS232_DIR_C_GPIO_Port,
                                  .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [BSP_DO_DIR_RS485_RS232_D] = {.init = {.Pin = OUT_RS485_RS232_DIR_D_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                                  .port = OUT_RS485_RS232_DIR_D_GPIO_Port,
                                  .init_state = GPIO_PIN_RESET}}; /*수신모드*/

void bsp_do_gpio_init(int do_number)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};


   board_clk_gpio(do_inst[do_number].port);

  GPIO_InitStruct.Pin = do_inst[do_number].init.Pin;
  GPIO_InitStruct.Mode = do_inst[do_number].init.Mode;
  GPIO_InitStruct.Pull = do_inst[do_number].init.Pull;
  HAL_GPIO_Init(do_inst[do_number].port, &GPIO_InitStruct);

  HAL_GPIO_WritePin(do_inst[do_number].port, do_inst[do_number].init.Pin, do_inst[do_number].init_state);
}

void bsp_do_init(void)
{
  for (int do_num = 0; do_num < BSP_DO_MAX; do_num++)
  {
    if (do_inst[do_num].opened)
      continue;
      switch (do_num)
      {
        case BSP_DO_POWER_CDMA:
        case BSP_DO_POWER_HART_24V:
        case BSP_DO_LCD_RESET:
        case BSP_DO_POWER_RAIN_DECT_DIGITAL:
        case BSP_DO_POWER_RAIN_DECT_ANALOG:
        case BSP_DO_ADC_NCS:
        case BSP_DO_FRAM_CS:
        case BSP_DO_RTC_CS:
        case BSP_DO_FLASH_CS:
        case BSP_DO_DIR_SDI:
        case BSP_DO_DIR_RS485_A:
        case BSP_DO_DIR_RS485_B:
        case BSP_DO_HART_SEL:
        case BSP_DO_HART_RTS:
        case BSP_DO_HART_RESET:
        case BSP_DO_DIR_RS485_RS232_C:
        case BSP_DO_DIR_RS485_RS232_D:
          bsp_do_gpio_init(do_num);
          do_inst[do_num].opened = true;
          break;
        case BSP_DO_EXT_0:  // App 정의되지 않음
        case BSP_DO_EXT_1:  // App 정의되지 않음
        case BSP_DO_EXT_2:  // App 정의되지 않음
        case BSP_DO_EXT_3:  // App 정의되지 않음
        case BSP_DO_EXT_4:  // App 정의되지 않음
        case BSP_DO_EXT_5:  // App 정의되지 않음
          pcf8575_init();
          do_inst[do_num].opened = true;
          break;

        default:
          break;
      }
  }
}

void bsp_do_low(int num)
{

  switch (num)
  {
    case BSP_DO_POWER_CDMA:
    case BSP_DO_POWER_HART_24V:
    case BSP_DO_LCD_RESET:
    case BSP_DO_POWER_RAIN_DECT_DIGITAL:
    case BSP_DO_POWER_RAIN_DECT_ANALOG:
    case BSP_DO_ADC_NCS:
    case BSP_DO_FRAM_CS:
    case BSP_DO_RTC_CS:
    case BSP_DO_FLASH_CS:
    case BSP_DO_DIR_SDI:
    case BSP_DO_DIR_RS485_A:
    case BSP_DO_DIR_RS485_B:
    case BSP_DO_HART_SEL:
    case BSP_DO_HART_RTS:
    case BSP_DO_HART_RESET:
    case BSP_DO_DIR_RS485_RS232_C:
    case BSP_DO_DIR_RS485_RS232_D:
      HAL_GPIO_WritePin(do_inst[num].port, do_inst[num].init.Pin, GPIO_PIN_RESET);
      break;
    case BSP_DO_EXT_0:  
     pcf8575_write_pin(DO_PCF8575_0, 0);
     break;
    case BSP_DO_EXT_1: 
      pcf8575_write_pin(DO_PCF8575_1, 0);
      break;
    case BSP_DO_EXT_2: 
      pcf8575_write_pin(DO_PCF8575_2, 0);
      break;
    case BSP_DO_EXT_3:  
      pcf8575_write_pin(DO_PCF8575_3, 0);
      break;
    case BSP_DO_EXT_4:  
      pcf8575_write_pin(DO_PCF8575_5, 0);
      break;
    case BSP_DO_EXT_5: 
      pcf8575_write_pin(DO_PCF8575_6, 0);
      break;

    default:
      break;
  }
}

void bsp_do_high(int num)
{

  switch (num)
  {
    case BSP_DO_POWER_CDMA:
    case BSP_DO_POWER_HART_24V:
    case BSP_DO_LCD_RESET:
    case BSP_DO_POWER_RAIN_DECT_DIGITAL:
    case BSP_DO_POWER_RAIN_DECT_ANALOG:
    case BSP_DO_ADC_NCS:
    case BSP_DO_FRAM_CS:
    case BSP_DO_RTC_CS:
    case BSP_DO_FLASH_CS:
    case BSP_DO_DIR_SDI:
    case BSP_DO_DIR_RS485_A:
    case BSP_DO_DIR_RS485_B:
    case BSP_DO_HART_SEL:
    case BSP_DO_HART_RTS:
    case BSP_DO_HART_RESET:
    case BSP_DO_DIR_RS485_RS232_C:
    case BSP_DO_DIR_RS485_RS232_D:
      HAL_GPIO_WritePin(do_inst[num].port, do_inst[num].init.Pin, GPIO_PIN_SET);
      break;
    case BSP_DO_EXT_0:
      pcf8575_write_pin(DO_PCF8575_0, 1);
      break;
    case BSP_DO_EXT_1:
      pcf8575_write_pin(DO_PCF8575_1, 1);
      break;
    case BSP_DO_EXT_2:
      pcf8575_write_pin(DO_PCF8575_2, 1);
      break;
    case BSP_DO_EXT_3:
      pcf8575_write_pin(DO_PCF8575_3, 1);
      break;
    case BSP_DO_EXT_4:
      pcf8575_write_pin(DO_PCF8575_5, 1);
      break;
    case BSP_DO_EXT_5:
      pcf8575_write_pin(DO_PCF8575_6, 1);
      break;

    default:
      break;
  }
}