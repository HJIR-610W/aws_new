
#include <stdint.h>
#include "stm32f4xx_hal.h"

#include "util_memory.h"
#include "mcu_interrupt.h"


typedef struct int_sub_s
{
  int gpio_pin;
  void *handle;
  void (*call)(void *);
}int_sub_t;


typedef struct mcu_interrupt_list_s
{
  int_sub_t *isrList;
}mcu_interrupt_list_t;



int_sub_t g_exti_0_gpio;
int_sub_t g_exti_1_gpio;
int_sub_t g_exti_2_gpio;
int_sub_t g_exti_3_gpio;
int_sub_t g_exti_4_gpio;
int_sub_t g_exti_5_9_gpio[5];
int_sub_t g_exti_10_15_gpio[6];
mcu_interrupt_list_t g_mcu_isr_list[82];//WWDG_IRQn 시작


void mcu_interrupt_init(void)
{

  g_mcu_isr_list[(int)EXTI0_IRQn].isrList = &g_exti_0_gpio;
  g_mcu_isr_list[(int)EXTI1_IRQn].isrList = &g_exti_1_gpio;
  g_mcu_isr_list[(int)EXTI2_IRQn].isrList = &g_exti_2_gpio;
  g_mcu_isr_list[(int)EXTI3_IRQn].isrList = &g_exti_3_gpio;
  g_mcu_isr_list[(int)EXTI4_IRQn].isrList = &g_exti_4_gpio;
  g_mcu_isr_list[(int)EXTI9_5_IRQn].isrList = g_exti_5_9_gpio;
  g_mcu_isr_list[(int)EXTI15_10_IRQn].isrList = g_exti_10_15_gpio;

}



void exti_register(exti_isr_cfg_t *cfg)
{
  int basePin;


  switch (cfg->irq)
  {
    case EXTI0_IRQn:
    basePin = getPinNumber(cfg->gpio_pin);
    g_mcu_isr_list[(int)EXTI0_IRQn].isrList->handle   = cfg->handle;
    g_mcu_isr_list[(int)EXTI0_IRQn].isrList->gpio_pin = cfg->gpio_pin;
    g_mcu_isr_list[(int)EXTI0_IRQn].isrList->call = cfg->call;
    break;
    case EXTI1_IRQn:
          basePin = getPinNumber(cfg->gpio_pin);
    g_mcu_isr_list[(int)EXTI1_IRQn].isrList->handle   = cfg->handle;
    g_mcu_isr_list[(int)EXTI1_IRQn].isrList->gpio_pin = cfg->gpio_pin;
    g_mcu_isr_list[(int)EXTI1_IRQn].isrList->call = cfg->call;
    break;
    case EXTI2_IRQn:
          basePin = getPinNumber(cfg->gpio_pin);
    g_mcu_isr_list[(int)EXTI2_IRQn].isrList->handle   = cfg->handle;
    g_mcu_isr_list[(int)EXTI2_IRQn].isrList->gpio_pin = cfg->gpio_pin;
    g_mcu_isr_list[(int)EXTI2_IRQn].isrList->call = cfg->call;
    break;
    case EXTI3_IRQn:
          basePin = getPinNumber(cfg->gpio_pin);
    g_mcu_isr_list[(int)EXTI3_IRQn].isrList->handle   = cfg->handle;
    g_mcu_isr_list[(int)EXTI3_IRQn].isrList->gpio_pin = cfg->gpio_pin;
    g_mcu_isr_list[(int)EXTI3_IRQn].isrList->call = cfg->call;
    break;
    case EXTI4_IRQn:
    basePin = getPinNumber(cfg->gpio_pin);
    g_mcu_isr_list[(int)EXTI4_IRQn].isrList->handle   = cfg->handle;
    g_mcu_isr_list[(int)EXTI4_IRQn].isrList->gpio_pin = cfg->gpio_pin;
    g_mcu_isr_list[(int)EXTI4_IRQn].isrList->call = cfg->call;
    break;
    case EXTI9_5_IRQn:
    basePin = getPinNumber(cfg->gpio_pin)-5;
    g_mcu_isr_list[(int)EXTI9_5_IRQn].isrList[basePin].handle   = cfg->handle;
    g_mcu_isr_list[(int)EXTI9_5_IRQn].isrList[basePin].gpio_pin = cfg->gpio_pin;
    g_mcu_isr_list[(int)EXTI9_5_IRQn].isrList[basePin].call = cfg->call;

    break;
  case EXTI15_10_IRQn:
  basePin = getPinNumber(cfg->gpio_pin)-10;
    g_mcu_isr_list[(int)EXTI15_10_IRQn].isrList[basePin].handle   = cfg->handle;
    g_mcu_isr_list[(int)EXTI15_10_IRQn].isrList[basePin].gpio_pin = cfg->gpio_pin;
    g_mcu_isr_list[(int)EXTI15_10_IRQn].isrList[basePin].call = cfg->call;
    
    break;
  
  default:
    break;
  }
 // g_mcu_isr_list[(IRQn_Type)cfg->irq].handle = cfg->handle;
 // g_mcu_isr_list[(IRQn_Type)cfg->irq].call = cfg->call;
 
}


extern UART_HandleTypeDef huart1 ;
extern UART_HandleTypeDef huart3 ;
extern UART_HandleTypeDef huart6 ;

extern DMA_HandleTypeDef hdma_usart3_tx;
extern DMA_HandleTypeDef hdma_usart6_tx;



extern DMA_HandleTypeDef hdma_sdio_rx;
extern DMA_HandleTypeDef hdma_sdio_tx;
extern SD_HandleTypeDef hsd;

void DMA2_Stream7_IRQHandler(void) 
{
  HAL_DMA_IRQHandler(&hdma_usart6_tx);
}

void DMA1_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
}



extern DMA_HandleTypeDef hdma_memtomem;;

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_memtomem);

}

void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_sdio_tx);
}


void DMA2_Stream3_IRQHandler(void)
{

  HAL_DMA_IRQHandler(&hdma_sdio_rx);

}


void SDIO_IRQHandler(void)
{
  HAL_SD_IRQHandler(&hsd);
}


void EXTI0_IRQHandler(void)
{
  void *handle=NULL;;
  void (*call)(void *)=NULL;
   
  
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);  // 인터럽트 플래그 클리어

    handle =  g_mcu_isr_list[EXTI0_IRQn].isrList->handle;
    call   =  g_mcu_isr_list[EXTI0_IRQn].isrList->call;

    if(call)
    {
      call(handle);
    }
  }
  

}
void EXTI1_IRQHandler(void)
{
  void *handle=NULL;;
  void (*call)(void *)=NULL;
   
  
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);  // 인터럽트 플래그 클리어

    handle = g_mcu_isr_list[EXTI1_IRQn].isrList->handle;
    call   =  g_mcu_isr_list[EXTI1_IRQn].isrList->call;

    if(call)
    {
      call(handle);
    }
  }
}
void EXTI2_IRQHandler(void)
{
  void *handle=NULL;;
  void (*call)(void *)=NULL;
   
  
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);  // 인터럽트 플래그 클리어

    handle = g_mcu_isr_list[EXTI2_IRQn].isrList->handle;
    call   =  g_mcu_isr_list[EXTI2_IRQn].isrList->call;

    if(call)
    {
      call(handle);
    }
  }
}

void EXTI3_IRQHandler(void)
{
  void *handle=NULL;;
  void (*call)(void *)=NULL;
   
  
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);  // 인터럽트 플래그 클리어

    handle = g_mcu_isr_list[EXTI3_IRQn].isrList->handle;
    call   =  g_mcu_isr_list[EXTI3_IRQn].isrList->call;

    if(call)
    {
      call(handle);
    }
  }
}

void EXTI4_IRQHandler(void)
{
  void *handle=NULL;;
  void (*call)(void *)=NULL;
   
  
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);  // 인터럽트 플래그 클리어

    handle = g_mcu_isr_list[EXTI4_IRQn].isrList->handle;
    call   =  g_mcu_isr_list[EXTI4_IRQn].isrList->call;

    if(call)
    {
      call(handle);
    }
  }
}

void EXTI9_5_IRQHandler(void)
{
  void *handle=NULL;;
  void (*call)(void *)=NULL;


    // PA10 ~ PA15의 인터럽트 확인 및 클리어
    for (uint32_t pin = GPIO_PIN_5; pin <= GPIO_PIN_9; pin <<= 1)
    {
        if (__HAL_GPIO_EXTI_GET_IT(pin) != RESET)
        {
            __HAL_GPIO_EXTI_CLEAR_IT(pin);  // 인터럽트 플래그 클리어
            if (pin == GPIO_PIN_5)
            {
                handle = g_mcu_isr_list[EXTI9_5_IRQn].isrList[0].handle;
                call   =  g_mcu_isr_list[EXTI9_5_IRQn].isrList[0].call;
                if(call)
                {
                  call(handle);
                }
            }
            else if (pin == GPIO_PIN_6)
            {
                handle = g_mcu_isr_list[EXTI9_5_IRQn].isrList[1].handle;
                call   =  g_mcu_isr_list[EXTI9_5_IRQn].isrList[1].call;
                if(call)
                {
                  call(handle);
                }
            }
                        else if (pin == GPIO_PIN_7)
            {
                handle = g_mcu_isr_list[EXTI9_5_IRQn].isrList[2].handle;
                call   =  g_mcu_isr_list[EXTI9_5_IRQn].isrList[2].call;
                if(call)
                {
                  call(handle);
                }
            } 
            else if (pin == GPIO_PIN_8)
            {
                handle = g_mcu_isr_list[EXTI9_5_IRQn].isrList[3].handle;
                call   =  g_mcu_isr_list[EXTI9_5_IRQn].isrList[3].call;
                if(call)
                {
                  call(handle);
                }
            } 
             else if (pin == GPIO_PIN_9)
            {
                handle = g_mcu_isr_list[EXTI9_5_IRQn].isrList[4].handle;
                call   =  g_mcu_isr_list[EXTI9_5_IRQn].isrList[4].call;
                if(call)
                {
                  call(handle);
                }
            } 
        }
    }
}



// 인터럽트 핸들러 구현
void EXTI15_10_IRQHandler(void)
{
  void *handle=NULL;;
  void (*call)(void *)=NULL;


    // PA10 ~ PA15의 인터럽트 확인 및 클리어
    for (uint32_t pin = GPIO_PIN_10; pin <= GPIO_PIN_15; pin <<= 1)
    {
        if (__HAL_GPIO_EXTI_GET_IT(pin) != RESET)
        {
            __HAL_GPIO_EXTI_CLEAR_IT(pin);  // 인터럽트 플래그 클리어

            // 인터럽트 발생 시 핀별로 수행할 작업
            if (pin == GPIO_PIN_10)
            {
                handle = g_mcu_isr_list[EXTI15_10_IRQn].isrList[0].handle;
                call   =  g_mcu_isr_list[EXTI15_10_IRQn].isrList[0].call;
                if(call)
                {
                  call(handle);
                }
            }
            else if (pin == GPIO_PIN_11)
            {
                handle = g_mcu_isr_list[EXTI15_10_IRQn].isrList[1].handle;
                call   =  g_mcu_isr_list[EXTI15_10_IRQn].isrList[1].call;
                if(call)
                {
                  call(handle);
                }
            } 
            else if (pin == GPIO_PIN_12)
            {
                handle = g_mcu_isr_list[EXTI15_10_IRQn].isrList[2].handle;
                call   =  g_mcu_isr_list[EXTI15_10_IRQn].isrList[2].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_13)
            {
                handle = g_mcu_isr_list[EXTI15_10_IRQn].isrList[3].handle;
                call   =  g_mcu_isr_list[EXTI15_10_IRQn].isrList[3].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_14) 
            {
                handle = g_mcu_isr_list[EXTI15_10_IRQn].isrList[4].handle;
                call   =  g_mcu_isr_list[EXTI15_10_IRQn].isrList[4].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_15) 
            {
                handle = g_mcu_isr_list[EXTI15_10_IRQn].isrList[5].handle;
                call   =  g_mcu_isr_list[EXTI15_10_IRQn].isrList[5].call;
                if(call)
                {
                  call(handle);
                }
            }
        }


    }
    
      /* USER CODE END EXTI15_10_IRQn 0 */
 // HAL_GPIO_EXTI_IRQHandler(IN_EX_UART_INT_7_Pin);
  //HAL_GPIO_EXTI_IRQHandler(IN_EX_UART_INT_8_Pin);
  //HAL_GPIO_EXTI_IRQHandler(INT_RTC_Pin);
}



extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}