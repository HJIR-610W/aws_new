#include <stdbool.h>
#include <stdint.h>

#include "mcu_utile.h"
#include "pcb_define.h"

#define IS_ODD_MUX_ACTIVE() \
  (is_gpio_set(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin) == 1)
#define IS_EVEN_MUX_ACTIVE() \
  (is_gpio_set(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin) == 1)
#define IS_RTD_MUX_ACTIVE() \
  (is_gpio_set(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin) == 1)

#define ODD_MUX_DEACTIVE()                                        \
  HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin, \
                    GPIO_PIN_RESET);
#define EVEN_MUX_DEACTIVE()                                         \
  HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin, \
                    GPIO_PIN_RESET);
#define RTD_MUX_DEACTIVE()                                        \
  HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin, \
                    GPIO_PIN_RESET);

#define ODD_MUX_ACTIVE() \
  HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin, GPIO_PIN_SET);
#define EVEN_MUX_ACTIVE()                                           \
  HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin, \
                    GPIO_PIN_SET);
#define RTD_MUX_ACTIVE() \
  HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin, GPIO_PIN_SET);

#include "stm32f4xx.h"

uint8_t is_gpio_set(GPIO_TypeDef *GPIOx, uint16_t pin)
{
  if (GPIOx->ODR & pin)
    return 1;  // ODR이 1이면 1 반환
  else
    return 0;  // ODR이 0이면 0 반환
}

void adc_single_mux_set(uint16_t channel)
{
  uint16_t add;
  uint16_t muxNum;

  add = channel / 4;

  muxNum = channel % 4;

  if (muxNum == 0)
  {
    // odd mux enable
    if (is_gpio_set(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin) == 0)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin,
                        GPIO_PIN_SET);
    }
    // even mux disable
    if (is_gpio_set(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin) == 1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin,
                        GPIO_PIN_RESET);
    }
    // rtd mux disable
    if (is_gpio_set(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin) == 1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin,
                        GPIO_PIN_RESET);
    }
  }

  if (muxNum == 1)  // even enable
  {
    // odd mux disable
    if (is_gpio_set(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin) == 1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin,
                        GPIO_PIN_RESET);
    }
    // even mux enable
    if (is_gpio_set(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin) == 0)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin,
                        GPIO_PIN_SET);
    }
    // rtd mux disable
    if (is_gpio_set(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin) == 1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin,
                        GPIO_PIN_RESET);
    }
  }

  if (muxNum == 2)  // rtd enable
  {
    // odd mux disable
    if (is_gpio_set(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin) == 1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin,
                        GPIO_PIN_RESET);
    }
    // even mux enable
    if (is_gpio_set(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin) == 1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin,
                        GPIO_PIN_RESET);
    }
    // rtd mux disable
    if (is_gpio_set(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin) == 0)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin,
                        GPIO_PIN_SET);
    }
  }

  // 채널 선택
  GPIO_TypeDef *portList[3] = {OUT_ADC_SEL_A0_GPIO_Port,
                               OUT_ADC_SEL_A1_GPIO_Port,
                               OUT_ADC_SEL_A2_GPIO_Port};
  uint16_t pinList[3] = {OUT_ADC_SEL_A0_Pin, OUT_ADC_SEL_A1_Pin,
                         OUT_ADC_SEL_A2_Pin};

  for (int i = 0; i < 3; i++)
  {
    if (add & (1 << i))
    {
      if (is_gpio_set(portList[i], pinList[i]) == GPIO_PIN_RESET)
        HAL_GPIO_WritePin(portList[i], pinList[i], GPIO_PIN_SET);
    }
    else
    {
      if (is_gpio_set(portList[i], pinList[i]) == GPIO_PIN_SET)
        HAL_GPIO_WritePin(portList[i], pinList[i], GPIO_PIN_RESET);
    }
  }
}

/**
 * @brief
 *
 * sel2 sel1 sel0 주소
 * 0    0    0    0
 * 0    0    1    1
 * 0    1    0    2
 * 0    1    1    3
 * 1    0    0    4
 * 1    0    1    5
 * 1    1    0    6
 * 1    1    1    7
 */
void mux_set(uint8_t add)
{
  GPIO_TypeDef *portList[3] = {OUT_ADC_SEL_A0_GPIO_Port,
                               OUT_ADC_SEL_A1_GPIO_Port,
                               OUT_ADC_SEL_A2_GPIO_Port};
  uint16_t pinList[3] = {OUT_ADC_SEL_A0_Pin, OUT_ADC_SEL_A1_Pin,
                         OUT_ADC_SEL_A2_Pin};

  for (int i = 0; i < 3; i++)
  {
    if (add & (1 << i))
    {
      if (is_gpio_set(portList[i], pinList[i]) == 0)
        HAL_GPIO_WritePin(portList[i], pinList[i], GPIO_PIN_SET);
    }
    else
    {
      if (is_gpio_set(portList[i], pinList[i]) == 1)
        HAL_GPIO_WritePin(portList[i], pinList[i], GPIO_PIN_RESET);
    }
  }
}

/**
 * @brief 채널 0,1,2,3,4,5,6,7
 *
 * sel2 sel1 sel0 채널
 * 0    0    0    0
 * 0    0    1    1
 * 0    1    0    2
 * 0    1    1    3
 * 1    0    0    4
 * 1    0    1    5
 * 1    1    0    6
 * 1    1    1    7
 */
void adc_diff_mux_set(uint16_t channel)
{
  uint16_t add;

  add = channel % 8;

  if (!IS_ODD_MUX_ACTIVE())
  {
    ODD_MUX_ACTIVE();
  }

  if (!IS_EVEN_MUX_ACTIVE())
  {
    EVEN_MUX_ACTIVE();
  }

  if (IS_RTD_MUX_ACTIVE())
  {
    RTD_MUX_DEACTIVE();
  }

  mux_set(add);
}

void adc_mux_init(void)
{
  // ADC 채널 MUX 초기화화

  board_config_gpio(OUT_ADC_SEL_A0_GPIO_Port, OUT_ADC_SEL_A0_Pin,
                    GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);
  board_set_gpio(OUT_ADC_SEL_A0_GPIO_Port, OUT_ADC_SEL_A0_Pin, GPIO_PIN_RESET);

  board_config_gpio(OUT_ADC_SEL_A1_GPIO_Port, OUT_ADC_SEL_A1_Pin,
                    GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);
  board_set_gpio(OUT_ADC_SEL_A1_GPIO_Port, OUT_ADC_SEL_A1_Pin, GPIO_PIN_RESET);

  board_config_gpio(OUT_ADC_SEL_A2_GPIO_Port, OUT_ADC_SEL_A2_Pin,
                    GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);
  board_set_gpio(OUT_ADC_SEL_A2_GPIO_Port, OUT_ADC_SEL_A2_Pin, GPIO_PIN_RESET);

  // odd even rtc 중 선택 mux 초기화
  board_set_gpio(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin, GPIO_PIN_SET);
  board_config_gpio(OUT_ADC_EN_ODD_GPIO_Port, OUT_ADC_EN_ODD_Pin,
                    GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

  board_set_gpio(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin, GPIO_PIN_SET);
  board_config_gpio(OUT_ADC_EN_EVEN_GPIO_Port, OUT_ADC_EN_EVEN_Pin,
                    GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

  board_set_gpio(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin, GPIO_PIN_SET);
  board_config_gpio(OUT_ADC_EN_RTD_GPIO_Port, OUT_ADC_EN_RTD_Pin,
                    GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);
}