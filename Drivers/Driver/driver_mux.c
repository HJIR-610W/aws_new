
#include "stm32f4xx_hal.h"
#include "main.h"
#include <stdint.h>




void adc_mux_set(uint16_t channel)
{
  uint16_t add;
  uint16_t muxNum;


  add = channel/4;

  muxNum = channel%4;



  if(muxNum==0)
  {
    //odd mux enable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_ODD_GPIO_Port,OUT_ADC_EN_ODD_Pin)==0)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port,OUT_ADC_EN_ODD_Pin,GPIO_PIN_SET);
    }
    //even mux disable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_EVEN_GPIO_Port,OUT_ADC_EN_EVEN_Pin)==1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port,OUT_ADC_EN_EVEN_Pin,GPIO_PIN_RESET);
    }
    //rtd mux disable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_RTD_GPIO_Port,OUT_ADC_EN_RTD_Pin)==1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port,OUT_ADC_EN_RTD_Pin,GPIO_PIN_RESET);
    }
  }

  if(muxNum==1)//even enable
  {
    //odd mux disable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_ODD_GPIO_Port,OUT_ADC_EN_ODD_Pin)==1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port,OUT_ADC_EN_ODD_Pin,GPIO_PIN_RESET);
    }
    //even mux enable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_EVEN_GPIO_Port,OUT_ADC_EN_EVEN_Pin)==0)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port,OUT_ADC_EN_EVEN_Pin,GPIO_PIN_SET);
    }
    //rtd mux disable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_RTD_GPIO_Port,OUT_ADC_EN_RTD_Pin)==1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port,OUT_ADC_EN_RTD_Pin,GPIO_PIN_RESET);
    }
  }

  if(muxNum==2)//even enable
  {
    //odd mux disable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_ODD_GPIO_Port,OUT_ADC_EN_ODD_Pin)==1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port,OUT_ADC_EN_ODD_Pin,GPIO_PIN_RESET);
    }
    //even mux enable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_EVEN_GPIO_Port,OUT_ADC_EN_EVEN_Pin)==1)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port,OUT_ADC_EN_EVEN_Pin,GPIO_PIN_RESET);
    }
    //rtd mux disable
    if(HAL_GPIO_ReadPin(OUT_ADC_EN_RTD_GPIO_Port,OUT_ADC_EN_RTD_Pin)==0)
    {
      HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port,OUT_ADC_EN_RTD_Pin,GPIO_PIN_SET);
    }
  }



GPIO_TypeDef *portList[3] = {OUT_ADC_SEL_A0_GPIO_Port,OUT_ADC_SEL_A1_GPIO_Port,OUT_ADC_SEL_A2_GPIO_Port};
uint16_t pinList[3] = {OUT_ADC_SEL_A0_Pin,OUT_ADC_SEL_A1_Pin,OUT_ADC_SEL_A2_Pin};



  for(int i = 0 ; i< 3;i++)
  {
    if(add&(1<<i))
    {
      if(HAL_GPIO_ReadPin(portList[i],pinList[i])==0)
      HAL_GPIO_WritePin(portList[i],pinList[i],GPIO_PIN_SET);
    }
    else
    {
      if(HAL_GPIO_ReadPin(portList[i],pinList[i])==1)
      HAL_GPIO_WritePin(portList[i],pinList[i],GPIO_PIN_RESET);
    }
  }



}



void adc_mux_init(void)
{
    HAL_GPIO_WritePin(OUT_ADC_EN_RTD_GPIO_Port,OUT_ADC_SEL_A0_Pin,OUT_ADC_EN_RTD_Pin);
    HAL_GPIO_WritePin(OUT_ADC_EN_ODD_GPIO_Port,OUT_ADC_SEL_A1_Pin,OUT_ADC_EN_ODD_Pin);
    HAL_GPIO_WritePin(OUT_ADC_EN_EVEN_GPIO_Port,OUT_ADC_SEL_A2_Pin,OUT_ADC_EN_EVEN_Pin);
}