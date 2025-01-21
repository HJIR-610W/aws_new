
#include <string.h>
#include "driver_stm32_do.h"
#include "driver_digitalOut.h"
#include "pcb_define.h"


#include "mcu_utile.h"
#include "utile.h"

typedef struct  stm32_do_cfg_s
{
  GPIO_TypeDef *port;
  uint16_t pin;
}stm32_do_cfg_t;




const stm32_do_cfg_t CDMA_PWR_cfg  ={.port=OUT_DO_PWR_CDMA_GPIO_Port,   .pin = OUT_DO_PWR_CDMA_Pin};
const stm32_do_cfg_t FRAM_CS_cfg  ={.port=OUT_SPI1_NSS_GPIO_Port,   .pin = OUT_SPI1_NSS_Pin};
const stm32_do_cfg_t RTC_CS_cfg   ={.port=OUT_SPI1_CS_RTC_GPIO_Port,.pin = OUT_SPI1_CS_RTC_Pin};
const stm32_do_cfg_t FLASH_CS_cfg ={.port=OUT_FLASH_CS_GPIO_Port,   .pin = OUT_FLASH_CS_Pin};
const stm32_do_cfg_t ADC_CS_cfg   ={.port=OUT_SPI2_NSS_GPIO_Port,   .pin = OUT_SPI2_NSS_Pin};
const stm32_do_cfg_t CON_PWR_232_A_cfg  = {.port=OUT_CON_PWR_232_A_GPIO_Port, .pin = OUT_CON_PWR_232_A_Pin};
const stm32_do_cfg_t CON_PWR_232_B_cfg  = {.port=OUT_CON_PWR_232_B_GPIO_Port, .pin = OUT_CON_PWR_232_B_Pin};
const stm32_do_cfg_t CON_PWR_485_cfg    = {.port=OUT_CON_PWR_485_GPIO_Port,   .pin = OUT_CON_PWR_485_Pin};
const stm32_do_cfg_t CON_PWR_TC_cfg     = {.port=CON_PWR_TC_GPIO_Port,        .pin = CON_PWR_TC_Pin};
const stm32_do_cfg_t CON_PWR_DSEN_cfg   = {.port=OUT_CON_PWR_DSEN_GPIO_Port,    .pin = OUT_CON_PWR_DSEN_Pin};
const stm32_do_cfg_t CON_PWR_ASEN_cfg   = {.port=OUT_CON_PWR_ASEN_GPIO_Port,    .pin = OUT_CON_PWR_ASEN_Pin};
const stm32_do_cfg_t CON_PWR_ASEN_A_cfg = {.port=OUT_CON_PWR_ASEN_GPIO_Port,   .pin = OUT_CON_PWR_ASEN_A_Pin};
const stm32_do_cfg_t CON_PWR_ASEN_B_cfg = {.port=OUT_CON_PWR_ASEN_B_GPIO_Port, .pin = OUT_CON_PWR_ASEN_B_Pin};
const stm32_do_cfg_t CON_PWR_ASEN_C_cfg = {.port=OUT_CON_PWR_ASEN_C_GPIO_Port, .pin = OUT_CON_PWR_ASEN_C_Pin};
const stm32_do_cfg_t CON_PWR_ASEN_D_cfg = {.port=OUT_CON_PWR_ASEN_D_GPIO_Port, .pin = OUT_CON_PWR_ASEN_D_Pin};


const stm32_do_cfg_t DIR_RS485_A_cfg = {.port=OUT_DIR_RS485_A_GPIO_Port, .pin = OUT_DIR_RS485_A_Pin};
const stm32_do_cfg_t DIR_RS485_B_cfg = {.port=OUT_DIR_RS485_B_GPIO_Port, .pin = OUT_DIR_RS485_B_Pin};
const stm32_do_cfg_t DIR_SDI_cfg     = {.port=OUT_DIR_SDI_GPIO_Port, .pin = OUT_DIR_SDI_Pin};






driver_t g_stm32_do_list[STM32_DO_MAX];

const driver_t g_stm[]={
   [0]={.cfg = &(stm32_do_cfg_t){.port=OUT_SPI1_NSS_GPIO_Port,.pin=OUT_SPI1_NSS_Pin}}};




void stm32_do_init(const stm32_do_cfg_t *cfg)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  board_clk_gpio(cfg->port);
  GPIO_InitStruct.Pin = cfg->pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(cfg->port ,&GPIO_InitStruct);

}

driver_t *stm32_do_open(int num)
{

  if(g_stm32_do_list[num].opened)
  {
    return &g_stm32_do_list[num];
  }
  g_stm32_do_list[num].opened = true;

  switch(num)
  {
    case STM32_DO_PWR_CDMA:
    g_stm32_do_list[num].cfg = (void *)&CDMA_PWR_cfg;
    stm32_do_init(&CDMA_PWR_cfg);
    break;
    case STM32_DO_ADC_NCS:
    g_stm32_do_list[num].cfg = (void *)&ADC_CS_cfg;
    stm32_do_init(&ADC_CS_cfg);
    break;
    case STM32_DO_FRAM_CS:
    g_stm32_do_list[num].cfg = (void *)&FRAM_CS_cfg;
    stm32_do_init(&FRAM_CS_cfg);
    break;
    case STM32_DO_RTC_CS:
    g_stm32_do_list[num].cfg = (void *)&RTC_CS_cfg;
    stm32_do_init(&RTC_CS_cfg);
    break;
    case STM32_DO_FLASH_CS:
    g_stm32_do_list[num].cfg = (void *)&FLASH_CS_cfg;
    stm32_do_init(&FLASH_CS_cfg);
    break;
    case STM32_DO_CON_PWR_232_A:
    g_stm32_do_list[num].cfg = (void *)&CON_PWR_232_A_cfg;
    stm32_do_init(&CON_PWR_232_A_cfg);
       break;
    case STM32_DO_CON_PWR_232_B:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_232_B_cfg;
    stm32_do_init(&CON_PWR_232_B_cfg);
       break;
    case STM32_DO_CON_PWR_485:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_485_cfg;
    stm32_do_init(&CON_PWR_485_cfg);
       break;
    case STM32_DO_CON_PWR_TC:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_TC_cfg;
    stm32_do_init(&CON_PWR_TC_cfg);
       break;
    case STM32_DO_CON_PWR_DSEN:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_DSEN_cfg;
    stm32_do_init(&CON_PWR_DSEN_cfg);
       break;
    case STM32_DO_CON_PWR_ASEN:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_ASEN_cfg;
    stm32_do_init(&CON_PWR_ASEN_cfg);
       break;
    case STM32_DO_CON_PWR_ASEN_A:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_ASEN_A_cfg;
    stm32_do_init(&CON_PWR_ASEN_A_cfg);
       break;
    case STM32_DO_CON_PWR_ASEN_B:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_ASEN_B_cfg;
    stm32_do_init(&CON_PWR_ASEN_B_cfg);
       break;
    case STM32_DO_CON_PWR_ASEN_C:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_ASEN_C_cfg;
    stm32_do_init(&CON_PWR_ASEN_C_cfg);
    break;
    case STM32_DO_CON_PWR_ASEN_D:
        g_stm32_do_list[num].cfg = (void *)&CON_PWR_ASEN_D_cfg;
      stm32_do_init(&CON_PWR_ASEN_D_cfg);
    break;



    case STM32_DO_DIR_SDI:
        g_stm32_do_list[num].cfg = (void *)&DIR_SDI_cfg;
      stm32_do_init(&DIR_SDI_cfg);
    break;

        case STM32_DO_DIR_RS485_A:
        g_stm32_do_list[num].cfg = (void *)&DIR_RS485_A_cfg;
      stm32_do_init(&DIR_RS485_A_cfg);
    break;


        case STM32_DO_DIR_RS485_B:
        g_stm32_do_list[num].cfg = (void *)&DIR_RS485_B_cfg;
      stm32_do_init(&DIR_RS485_B_cfg);
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
