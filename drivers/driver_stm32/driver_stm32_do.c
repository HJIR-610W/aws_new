#include "bsp.h"
#include "driver_stm32_do.h"
#include "pcf8575.h"
#include "util_memory.h"
#include "pcb_define.h"
#include "dev_io.h"


typedef struct stm32_do_inst_s
{
  bool opened;
  GPIO_InitTypeDef init;
  GPIO_TypeDef *port;
  GPIO_PinState init_state;
} stm32_do_inst_t;

#ifdef AWS_PCB_0_5
stm32_do_inst_t do_inst[STM32_DO_MAX] = {
    [STM32_DO_POWER_CDMA] = {.init = {.Pin = DO_CON_PWR_CDMA_Pin,
                                    .Mode = GPIO_MODE_OUTPUT_PP,
                                    .Pull = GPIO_NOPULL,
                                    .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_CON_PWR_CDMA_GPIO_Port,
                           .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [STM32_DO_POWER_HART_24V] = {.init = {.Pin = DO_CON_PWR_S24_Pin,
                                        .Mode = GPIO_MODE_OUTPUT_PP,
                                        .Pull = GPIO_NOPULL,
                                        .Speed = GPIO_SPEED_FREQ_LOW},
                               .port = DO_CON_PWR_S24_GPIO_Port,
                               .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [STM32_DO_POWER_RAIN_DECT_DIGITAL] = {.init = {.Pin = DO_CON_PWR_RAIN_DIGITAL_Pin,
                                                 .Mode = GPIO_MODE_OUTPUT_PP,
                                                 .Pull = GPIO_NOPULL,
                                                 .Speed = GPIO_SPEED_FREQ_LOW},
                                        .port = DO_CON_PWR_RAIN_DIGITAL_GPIO_Port,
                                        .init_state = GPIO_PIN_RESET}, /*전원 차단*/

    [STM32_DO_POWER_RAIN_DECT_ANALOG] = {.init = {.Pin = DO_CON_PWR_RAIN_Pin,
                                                .Mode = GPIO_MODE_OUTPUT_PP,
                                                .Pull = GPIO_NOPULL,
                                                .Speed = GPIO_SPEED_FREQ_LOW},
                                       .port = DO_CON_PWR_RAIN_GPIO_Port,
                                       .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [STM32_DO_LCD_RESET] = {.init = {.Pin = DO_BTM_PWRC_Pin,
                                   .Mode = GPIO_MODE_OUTPUT_PP,
                                   .Pull = GPIO_NOPULL,
                                   .Speed = GPIO_SPEED_FREQ_LOW},
                          .port = DO_BTM_PWRC_GPIO_Port,
                          .init_state = GPIO_PIN_RESET},
    [STM32_DO_ADC_CS] = {.init = {.Pin = DO_SPI2_NSS_Pin,
                                .Mode = GPIO_MODE_OUTPUT_PP,
                                .Pull = GPIO_PULLUP,
                                .Speed = GPIO_SPEED_FREQ_HIGH},
                       .port = DO_SPI2_NSS_GPIO_Port,
                       .init_state = GPIO_PIN_SET}, /*비활성*/
    [STM32_DO_FRAM_CS] = {.init = {.Pin = DO_SPI1_NSS_Pin,
                                 .Mode = GPIO_MODE_OUTPUT_PP,
                                 .Pull = GPIO_NOPULL,
                                 .Speed = GPIO_SPEED_FREQ_HIGH},
                        .port = DO_SPI1_NSS_GPIO_Port,
                        .init_state = GPIO_PIN_SET}, /*비활성*/
    [STM32_DO_RTC_CS] = {.init = {.Pin = DO_RV8803_EVI_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH},
                       .port = DO_RV8803_EVI_GPIO_Port,
                       .init_state = GPIO_PIN_SET}, /*비활성*/
    [STM32_DO_DIR_SDI] = {.init = {.Pin = DO_DIR_SDI_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                        .port = DO_DIR_SDI_GPIO_Port,
                        .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_HART_SEL] = {.init = {.Pin = DO_SEL_IF_UART_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                         .port = DO_SEL_IF_UART_GPIO_Port,
                         .init_state = GPIO_PIN_RESET}, /*RS232모드로 설정*/
    [STM32_DO_HART_RTS] = {.init = {.Pin = DO_RTS_H_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                         .port = DO_RTS_H_GPIO_Port,
                         .init_state = GPIO_PIN_SET}, // 수신모드
    [STM32_DO_HART_RESET] = {.init = {.Pin = DO_RESET_H_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_RESET_H_GPIO_Port,
                           .init_state = GPIO_PIN_SET}, // 활성
    [STM32_DO_DIR_RS485_A] = {.init = {.Pin = DO_DIR_RS485_A_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = DO_DIR_RS485_A_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_DIR_RS485_B] = {.init = {.Pin = DO_DIR_RS485_B_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = DO_DIR_RS485_B_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_DIR_RS485_C] = {.init = {.Pin = DO_RS485_DIR_C_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = DO_RS485_DIR_C_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_DIR_RS485_D] = {.init = {.Pin = DO_RS485_DIR_D_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = DO_RS485_DIR_D_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_POWER_LCD] = {.init = {.Pin = DO_POWER_LCD_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_POWER_LCD_GPIO_Port,
                           .init_state = GPIO_PIN_SET},
    [STM32_DO_POWER_BTM] = {.init = {.Pin = DO_BTM_PWRC_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                          .port = DO_BTM_PWRC_GPIO_Port,
                          .init_state = GPIO_PIN_SET},

    [STM32_DO_QUAD_A_RST] = {.init = {.Pin = DO_EX_UART_RST_A_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_EX_UART_RST_A_GPIO_Port,
                           .init_state = GPIO_PIN_RESET},

    [STM32_DO_QUAD_B_RST] = {.init = {.Pin = DO_EX_UART_RST_B_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_EX_UART_RST_B_GPIO_Port,
                           .init_state = GPIO_PIN_RESET},
};
#endif


#ifdef AWS_PCB_0_6
stm32_do_inst_t do_inst[STM32_DO_MAX] = {
    [STM32_DO_POWER_CDMA] = {.init = {.Pin = DO_CON_PWR_CDMA_Pin,
                                    .Mode = GPIO_MODE_OUTPUT_PP,
                                    .Pull = GPIO_NOPULL,
                                    .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_CON_PWR_CDMA_GPIO_Port,
                           .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [STM32_DO_POWER_HART_24V] = {.init = {.Pin = DO_CON_PWR_S24_Pin,
                                        .Mode = GPIO_MODE_OUTPUT_PP,
                                        .Pull = GPIO_NOPULL,
                                        .Speed = GPIO_SPEED_FREQ_LOW},
                               .port = DO_CON_PWR_S24_GPIO_Port,
                               .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [STM32_DO_POWER_RAIN_DECT_DIGITAL] = {.init = {.Pin = DO_CON_PWR_RAIN_DIGITAL_Pin,
                                                 .Mode = GPIO_MODE_OUTPUT_PP,
                                                 .Pull = GPIO_NOPULL,
                                                 .Speed = GPIO_SPEED_FREQ_LOW},
                                        .port = DO_CON_PWR_RAIN_DIGITAL_GPIO_Port,
                                        .init_state = GPIO_PIN_RESET}, /*전원 차단*/

    [STM32_DO_POWER_RAIN_DECT_ANALOG] = {.init = {.Pin = DO_CON_PWR_RAIN_Pin,
                                                .Mode = GPIO_MODE_OUTPUT_PP,
                                                .Pull = GPIO_NOPULL,
                                                .Speed = GPIO_SPEED_FREQ_LOW},
                                       .port = DO_CON_PWR_RAIN_GPIO_Port,
                                       .init_state = GPIO_PIN_RESET}, /*전원 차단*/
    [STM32_DO_LCD_RESET] = {.init = {.Pin = DO_BTM_PWRC_Pin,
                                   .Mode = GPIO_MODE_OUTPUT_PP,
                                   .Pull = GPIO_NOPULL,
                                   .Speed = GPIO_SPEED_FREQ_LOW},
                          .port = DO_BTM_PWRC_GPIO_Port,
                          .init_state = GPIO_PIN_RESET},
    [STM32_DO_ADC_CS] = {.init = {.Pin = DO_SPI2_NSS_Pin,
                                .Mode = GPIO_MODE_OUTPUT_PP,
                                .Pull = GPIO_PULLUP,
                                .Speed = GPIO_SPEED_FREQ_HIGH},
                       .port = DO_SPI2_NSS_GPIO_Port,
                       .init_state = GPIO_PIN_SET}, /*비활성*/
    [STM32_DO_FRAM_CS] = {.init = {.Pin = DO_SPI1_NSS_Pin,
                                 .Mode = GPIO_MODE_OUTPUT_PP,
                                 .Pull = GPIO_NOPULL,
                                 .Speed = GPIO_SPEED_FREQ_HIGH},
                        .port = DO_SPI1_NSS_GPIO_Port,
                        .init_state = GPIO_PIN_SET}, /*비활성*/
    [STM32_DO_RTC_CS] = {.init = {.Pin = DO_RV8803_EVI_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_HIGH},
                       .port = DO_RV8803_EVI_GPIO_Port,
                       .init_state = GPIO_PIN_SET}, /*비활성*/
    [STM32_DO_DIR_SDI] = {.init = {.Pin = DO_DIR_SDI_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                        .port = DO_DIR_SDI_GPIO_Port,
                        .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_HART_SEL] = {.init = {.Pin = DO_SEL_IF_UART_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                         .port = DO_SEL_IF_UART_GPIO_Port,
                         .init_state = GPIO_PIN_RESET}, /*RS232모드로 설정*/
    [STM32_DO_HART_RTS] = {.init = {.Pin = DO_RTS_H_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                         .port = DO_RTS_H_GPIO_Port,
                         .init_state = GPIO_PIN_SET}, // 수신모드
    [STM32_DO_HART_RESET] = {.init = {.Pin = DO_RESET_H_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_RESET_H_GPIO_Port,
                           .init_state = GPIO_PIN_SET}, // 활성
    [STM32_DO_DIR_RS485_A] = {.init = {.Pin = DO_DIR_RS485_A_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = DO_DIR_RS485_A_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_DIR_RS485_B] = {.init = {.Pin = DO_DIR_RS485_B_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = DO_DIR_RS485_B_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_DIR_RS485_C] = {.init = {.Pin = DO_RS485_DIR_C_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = DO_RS485_DIR_C_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_DIR_RS485_D] = {.init = {.Pin = DO_RS485_DIR_D_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                            .port = DO_RS485_DIR_D_GPIO_Port,
                            .init_state = GPIO_PIN_RESET}, /*수신모드*/
    [STM32_DO_POWER_LCD] = {.init = {.Pin = DO_POWER_LCD_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_POWER_LCD_GPIO_Port,
                           .init_state = GPIO_PIN_SET},

    [STM32_DO_QUAD_A_RST] = {.init = {.Pin = DO_EX_UART_RST_A_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_EX_UART_RST_A_GPIO_Port,
                           .init_state = GPIO_PIN_RESET},

    [STM32_DO_QUAD_B_RST] = {.init = {.Pin = DO_EX_UART_RST_B_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
                           .port = DO_EX_UART_RST_B_GPIO_Port,
                           .init_state = GPIO_PIN_RESET},
         [STM32_DO_FLASH_CS] = {.init = {.Pin = DO_CS_S_FLASH_Pin, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW},
     .port = DO_CS_S_FLASH_GPIO_Port,
     .init_state = GPIO_PIN_SET},
};
#endif

void stm32_do_gpio_init(int do_number)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};


   board_clk_gpio(do_inst[do_number].port);

   HAL_GPIO_WritePin(do_inst[do_number].port, do_inst[do_number].init.Pin, do_inst[do_number].init_state);

   GPIO_InitStruct.Pin = do_inst[do_number].init.Pin;
   GPIO_InitStruct.Mode = do_inst[do_number].init.Mode;
   GPIO_InitStruct.Pull = do_inst[do_number].init.Pull;
   HAL_GPIO_Init(do_inst[do_number].port, &GPIO_InitStruct);


}

void stm32_do_init(void)
{
  for (int do_num = 0; do_num < STM32_DO_MAX; do_num++)
  {
    if (do_inst[do_num].opened)
      continue;
      switch (do_num)
      {
        case STM32_DO_POWER_CDMA:
        case STM32_DO_POWER_HART_24V:
        case STM32_DO_LCD_RESET:
        case STM32_DO_POWER_RAIN_DECT_DIGITAL:
        case STM32_DO_POWER_RAIN_DECT_ANALOG:
        case STM32_DO_ADC_CS:
        case STM32_DO_FRAM_CS:
        case STM32_DO_RTC_CS:
        case STM32_DO_DIR_SDI:
        case STM32_DO_DIR_RS485_A:
        case STM32_DO_DIR_RS485_B:
        case STM32_DO_HART_SEL:
        case STM32_DO_HART_RTS:
        case STM32_DO_HART_RESET:
        case STM32_DO_DIR_RS485_C:
        case STM32_DO_DIR_RS485_D:
        case STM32_DO_POWER_LCD:
        case STM32_DO_QUAD_A_RST:
        case STM32_DO_QUAD_B_RST:
#ifdef AWS_PCB_0_6
    case STM32_DO_FLASH_CS:
#endif
          stm32_do_gpio_init(do_num);
            do_inst[do_num].opened = true;
            break;


      }
  }
}

void stm32_do_low(int num)
{

  switch (num)
  {
    case STM32_DO_POWER_CDMA:
    case STM32_DO_POWER_HART_24V:
    case STM32_DO_LCD_RESET:
    case STM32_DO_POWER_RAIN_DECT_DIGITAL:
    case STM32_DO_POWER_RAIN_DECT_ANALOG:
    case STM32_DO_ADC_CS:
    case STM32_DO_FRAM_CS:
    case STM32_DO_RTC_CS:
    case STM32_DO_DIR_SDI:
    case STM32_DO_DIR_RS485_A:
    case STM32_DO_DIR_RS485_B:
    case STM32_DO_HART_SEL:
    case STM32_DO_HART_RTS:
    case STM32_DO_HART_RESET:
    case STM32_DO_DIR_RS485_C:
    case STM32_DO_DIR_RS485_D:
    case STM32_DO_POWER_LCD:
    case STM32_DO_QUAD_A_RST:
    case STM32_DO_QUAD_B_RST:
#ifdef AWS_PCB_0_6
    case STM32_DO_FLASH_CS:
#endif
      HAL_GPIO_WritePin(do_inst[num].port, do_inst[num].init.Pin, GPIO_PIN_RESET);
      break;


  }
}

void stm32_do_high(int num)
{

  switch (num)
  {
    case STM32_DO_POWER_CDMA:
    case STM32_DO_POWER_HART_24V:
    case STM32_DO_LCD_RESET:
    case STM32_DO_POWER_RAIN_DECT_DIGITAL:
    case STM32_DO_POWER_RAIN_DECT_ANALOG:
    case STM32_DO_ADC_CS:
    case STM32_DO_FRAM_CS:
    case STM32_DO_RTC_CS:
    case STM32_DO_DIR_SDI:
    case STM32_DO_DIR_RS485_A:
    case STM32_DO_DIR_RS485_B:
    case STM32_DO_HART_SEL:
    case STM32_DO_HART_RTS:
    case STM32_DO_HART_RESET:
    case STM32_DO_DIR_RS485_C:
    case STM32_DO_DIR_RS485_D:
    case STM32_DO_POWER_LCD:
    case STM32_DO_QUAD_A_RST:
    case STM32_DO_QUAD_B_RST:
#ifdef AWS_PCB_0_6
    case STM32_DO_FLASH_CS:
#endif
      HAL_GPIO_WritePin(do_inst[num].port, do_inst[num].init.Pin, GPIO_PIN_SET);
      break;


  }
}