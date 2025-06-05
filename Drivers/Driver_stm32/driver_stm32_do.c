
#include "driver_stm32_do.h"

#include <string.h>

#include "driver_do.h"
#include "mcu_utile.h"
#include "pcb_define.h"
#include "util_memory.h"

typedef struct  stm32_do_cfg_s
{
  GPIO_TypeDef *port;
  uint16_t pin;
}stm32_do_cfg_t;

void stm32_do_low(driver_t *driver);
void stm32_do_high(driver_t *driver);
void stm32_do_close(driver_t *driver);
void stm32_do_set(driver_t *driver,do_set_option_t cmd,void *opt);
int32_t stm32_do_read(driver_t *driver,uint8_t *err);

const do_api_t do_api = {.low = stm32_do_low,
                         .high = stm32_do_high,
                         .close = stm32_do_close,
                         .set = stm32_do_set,
                         .read = stm32_do_read};

const stm32_do_cfg_t CDMA_PWR_cfg = {.port = OUT_PWR_CDMA_GPIO_Port, .pin = OUT_PWR_CDMA_PIN};
const stm32_do_cfg_t FRAM_CS_cfg = {.port = OUT_SPI1_NSS_GPIO_Port, .pin = OUT_SPI1_NSS_PIN};
const stm32_do_cfg_t RTC_CS_cfg = {.port = OUT_SPI1_CS_RTC_GPIO_Port, .pin = OUT_RV8803_EVI_Pin};
const stm32_do_cfg_t FLASH_CS_cfg = {.port = OUT_FLASH_CS_GPIO_Port, .pin = OUT_FLASH_CS_PIN};
const stm32_do_cfg_t ADC_CS_cfg = {.port = OUT_SPI2_NSS_GPIO_Port, .pin = OUT_SPI2_NSS_PIN};
const stm32_do_cfg_t DIR_RS485_A_cfg = {.port = OUT_DIR_RS485_A_GPIO_Port,
                                        .pin = OUT_DIR_RS485_A_PIN};
const stm32_do_cfg_t DIR_RS485_B_cfg = {.port = OUT_DIR_RS485_B_GPIO_Port,
                                        .pin = OUT_DIR_RS485_B_PIN};
const stm32_do_cfg_t DIR_SDI_cfg = {.port = OUT_DIR_SDI_GPIO_Port, .pin = OUT_DIR_SDI_PIN};
const stm32_do_cfg_t HART_SEL_cfg = {.port = DO_SEL_IF_UART_GPIO_Port, .pin = SEL_IF_UART_Pin};
const stm32_do_cfg_t HART_RTS_cfg = {.port = DO_RTS_H_GPIO_Port, .pin = DO_RTS_H_Pin};
const stm32_do_cfg_t POWER_24V_cfg = {.port = DO_CON_PWR_S24_GPIO_Port, .pin = DO_CON_PWR_S24_Pin};
const stm32_do_cfg_t HART_RESET_cfg = {.port = DO_RESET_H_GPIO_Port, .pin = DO_RESET_H_Pin};
const stm32_do_cfg_t BTM_PWRC_cfg = {.port = DO_BTM_PWRC_GPIO_Port, .pin = DO_BTM_PWRC_Pin};
const stm32_do_cfg_t DIR_RS485_C_cfg = {.port = OUT_RS485_DIR_C_GPIO_Port,
                                        .pin = OUT_RS485_DIR_C_PIN};
const stm32_do_cfg_t DIR_RS485_D_cfg = {.port = OUT_RS485_DIR_D_GPIO_Port,
                                        .pin = OUT_RS485_DIR_D_PIN};

const stm32_do_cfg_t CON_PWR_RAIN_DECT_cfg = {.port = DO_POWER_RAIN_DECT_DIGITAL_GPIO_Port,
                                              .pin = DO_POWER_RAIN_DECT_DIGITAL_PIN};

const stm32_do_cfg_t CON_PWR_RAIN_cfg = {.port = DO_CON_PWR_RAIN_GPIO_Port,
                                         .pin = DO_CON_PWR_RAIN_PIN};

driver_t g_stm32_do_list[STM32_DO_MAX];

const driver_t g_stm[]={
   [0]={.cfg = &(stm32_do_cfg_t){.port=OUT_SPI1_NSS_GPIO_Port,.pin=OUT_SPI1_NSS_PIN}}};




void stm32_do_init(const stm32_do_cfg_t *cfg,void *opt)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  do_config_t *do_config=opt;
  uint32_t mode = GPIO_MODE_OUTPUT_PP;
  uint32_t pull =GPIO_NOPULL;

  if(opt)
  {
    if(do_config->mode == DO_OUT_OD)
    {
      mode = GPIO_MODE_OUTPUT_OD;
    }
    else
    {
      mode = GPIO_MODE_OUTPUT_PP;
    }

    if(do_config->pullup == DO_PULL_UP)
    {
      pull = GPIO_PULLUP;
    }
    else if(do_config->pullup == DO_PULL_DOWN)
    {
      pull = GPIO_PULLDOWN;
    }

  }
  board_clk_gpio(cfg->port);
  GPIO_InitStruct.Pin = cfg->pin;
  GPIO_InitStruct.Mode = mode;
  GPIO_InitStruct.Pull = pull;
  HAL_GPIO_Init(cfg->port ,&GPIO_InitStruct);

}

driver_t *stm32_do_open(int num,void *opt)
{

  if(g_stm32_do_list[num].opened)
  {
    return &g_stm32_do_list[num];
  }
  g_stm32_do_list[num].opened = true;
  g_stm32_do_list[num].api = &do_api;

  switch(num)
  {
    case STM32_DO_POWER_CDMA:
    g_stm32_do_list[num].cfg = (void *)&CDMA_PWR_cfg;
    stm32_do_init(&CDMA_PWR_cfg,opt);
    break;
    case STM32_DO_ADC_NCS:
      g_stm32_do_list[num].cfg = (void *)&ADC_CS_cfg;
      stm32_do_init(&ADC_CS_cfg,opt);
    break;
    case STM32_DO_FRAM_CS:
      g_stm32_do_list[num].cfg = (void *)&FRAM_CS_cfg;
      stm32_do_init(&FRAM_CS_cfg,opt);
    break;
    case STM32_DO_RTC_CS:
      g_stm32_do_list[num].cfg = (void *)&RTC_CS_cfg;
      stm32_do_init(&RTC_CS_cfg,opt);
    break;
    case STM32_DO_FLASH_CS:
      g_stm32_do_list[num].cfg = (void *)&FLASH_CS_cfg;
      stm32_do_init(&FLASH_CS_cfg,opt);
    break;
    case STM32_DO_HART_SEL:
      g_stm32_do_list[num].cfg = (void *)&HART_SEL_cfg;
      stm32_do_init(&HART_SEL_cfg, opt);
      break;
    case STM32_DO_HART_RTS:
      g_stm32_do_list[num].cfg = (void *)&HART_RTS_cfg;
      stm32_do_init(&HART_RTS_cfg, opt);
      break;
    case STM32_DO_POWER_24V:
      g_stm32_do_list[num].cfg = (void *)&POWER_24V_cfg;
      stm32_do_init(&POWER_24V_cfg, opt);
      break;
    case STM32_DO_HART_RESET:
      g_stm32_do_list[num].cfg = (void *)&HART_RESET_cfg;
      stm32_do_init(&HART_RESET_cfg, opt);
      break;
    case STM32_DO_BTM_PWRC:
      g_stm32_do_list[num].cfg = (void *)&BTM_PWRC_cfg;
      stm32_do_init(&BTM_PWRC_cfg, opt);

      break;
    case STM32_DO_DIR_SDI:
      g_stm32_do_list[num].cfg = (void *)&DIR_SDI_cfg;
      stm32_do_init(&DIR_SDI_cfg,opt);
      break;
      break;
      case STM32_DO_DIR_RS485_A:
      g_stm32_do_list[num].cfg = (void *)&DIR_RS485_A_cfg;
      stm32_do_init(&DIR_RS485_A_cfg,opt);
    break;
      case STM32_DO_DIR_RS485_B:
      g_stm32_do_list[num].cfg = (void *)&DIR_RS485_B_cfg;
      stm32_do_init(&DIR_RS485_B_cfg,opt);
      break;
    case STM32_DO_DIR_RS485_C:
      g_stm32_do_list[num].cfg = (void *)&DIR_RS485_C_cfg;
      stm32_do_init(&DIR_RS485_C_cfg, opt);
      break;
    case STM32_DO_DIR_RS485_D:
      g_stm32_do_list[num].cfg = (void *)&DIR_RS485_D_cfg;
      stm32_do_init(&DIR_RS485_D_cfg, opt);
      break;
    case STM32_DO_POWER_RAIN_DECT_DIGITAL:
      g_stm32_do_list[num].cfg = (void *)&CON_PWR_RAIN_DECT_cfg;
      stm32_do_init(&CON_PWR_RAIN_DECT_cfg, opt);
      break;
    case STM32_DO_POWER_RAIN_DECT_ANALOG:
      g_stm32_do_list[num].name = "STM32_DO_POWER_RAIN_DECT_ANALOG";
       g_stm32_do_list[num].cfg = (void *)&CON_PWR_RAIN_cfg;
      stm32_do_init(&CON_PWR_RAIN_cfg, opt);
      break;
  }
  return &g_stm32_do_list[num];
}

void stm32_do_low(driver_t *driver)
{
  stm32_do_cfg_t *cfg = driver->cfg;

  HAL_GPIO_WritePin(cfg->port,cfg->pin,GPIO_PIN_RESET);
}

void stm32_do_high(driver_t *driver)
{
  stm32_do_cfg_t *cfg = driver->cfg;

  HAL_GPIO_WritePin(cfg->port,cfg->pin,GPIO_PIN_SET);
}

void stm32_do_close(driver_t *driver)
{

}


void stm32_do_set(driver_t *driver,do_set_option_t cmd,void *opt)
{

}

int32_t stm32_do_read(driver_t *driver,uint8_t *err)
{
  *err =0;
  int32_t pin;
  stm32_do_cfg_t *cfg = driver->cfg;

  pin = (int32_t)HAL_GPIO_ReadPin(cfg->port,cfg->pin);

  return pin;
}

#if 0 


typedef struct driver_ex_s
{
  const char *name;
  struct driver_ex_s *driver;
  void *cfg;
}driver_ex_t;
const driver_ex_t *stm32_open(const char *name)
{
  int cnt;
  gpio_cfg_t *cfg;

  cnt = _countof(g_gpio_list);

  for(int i=0;i<cnt;i++)
  {
    if(strncmp(name,g_gpio_list[i].name,strlen(name))==0)
    {
      cfg = (gpio_cfg_t *)g_gpio_list[i].cfg;
      if(cfg->opened==false)
      {
        cfg->opened = true;

      }

      return &g_gpio_list[i];
    }
  }

  return 0;
}
#endif
