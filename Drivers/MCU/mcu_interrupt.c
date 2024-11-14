
#include <stdint.h>
#include "stm32f4xx_hal.h"

#include "utile.h"


typedef struct mcu_interrupt_list_s
{
  int init;
  int num;
  void (*call)(void *);
  void *handle;
}mcu_interrupt_list_t;

mcu_interrupt_list_t g_mcu_isr_list[82];//WWDG_IRQn 시작







void exti_register(uint16_t pin,void *handle,void (*call)(void *))
{
  int pos;
  pin = getPinNumber(pin);

  if(pos !=-1)
  {
    g_mcu_isr_list[pin].handle = handle;
    g_mcu_isr_list[pin].call = call;
  }
}



extern UART_HandleTypeDef huart1 ;
extern UART_HandleTypeDef huart3 ;
extern UART_HandleTypeDef huart6 ;

extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern DMA_HandleTypeDef hdma_usart6_tx;


extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_tx;
extern DMA_HandleTypeDef hdma_rx;


void DMA2_Stream7_IRQHandler(void) 
{
    HAL_DMA_IRQHandler(&hdma_usart1_tx);
}

void DMA1_Stream3_IRQHandler(void) {
    // DMA 상태 레지스터에서 전송 완료 인터럽트 플래그 확인
    if (__HAL_DMA_GET_FLAG(&hdma_usart3_tx, DMA_FLAG_TCIF3_7)) {
        // USART3 TX DMA 전송 완료 인터럽트
        HAL_DMA_IRQHandler(&hdma_usart3_tx);
    } 
    else if (__HAL_DMA_GET_FLAG(&hdma_tx, DMA_FLAG_TCIF3_7)) {
        // SPI1 TX DMA 전송 완료 인터럽트
        HAL_DMA_IRQHandler(&hdma_tx);
         HAL_DMA_IRQHandler(hspi1.hdmatx);
    }
}



extern DMA_HandleTypeDef hdma_sdio_tx;
extern DMA_HandleTypeDef hdma_sdio_rx;
extern SD_HandleTypeDef hsd;


void DMA2_Stream6_IRQHandler(void) {
  
  
      /* DMA 스트림 3의 인터럽트 상태 플래그 확인 */
    if (__HAL_DMA_GET_FLAG(&hdma_sdio_tx, DMA_FLAG_TCIF2_6))
    {
      // __HAL_DMA_CLEAR_FLAG(&hdma_sdio_tx, DMA_FLAG_TCIF2_6);
        HAL_DMA_IRQHandler(&hdma_sdio_tx);
    }
        else if (__HAL_DMA_GET_FLAG(&hdma_sdio_tx, DMA_FLAG_TCIF2_6)) {
        /* 인터럽트 플래그 클리어 */
        __HAL_DMA_CLEAR_FLAG(&hdma_sdio_tx, DMA_FLAG_TCIF2_6);
//
        /* 전송 오류 처리 콜백 호출 */
        HAL_DMA_IRQHandler(&hdma_sdio_tx);
    }
        else if (__HAL_DMA_GET_FLAG(&hdma_sdio_tx, DMA_FLAG_TCIF2_6)) {
        /* 인터럽트 플래그 클리어 */
      //  __HAL_DMA_CLEAR_FLAG(&hdma_sdio_tx, DMA_FLAG_TCIF2_6);

        /* 반전송 완료 처리 콜백 호출 */
        HAL_DMA_IRQHandler(&hdma_sdio_tx);
    }
      /* USART6 전송 완료 플래그 확인 */
    else if (__HAL_DMA_GET_FLAG(&hdma_usart6_tx, DMA_FLAG_TCIF2_6))
    {
        /* USART6의 전송 완료 인터럽트 발생 */
        /* 인터럽트 플래그 클리어 */
       // __HAL_DMA_CLEAR_FLAG(&hdma_usart6_tx, DMA_FLAG_TCIF2_6);

        /* USART6용 DMA 핸들러 호출 */
        HAL_DMA_IRQHandler(&hdma_usart6_tx);
    }
}



void DMA2_Stream3_IRQHandler(void)
{

  HAL_DMA_IRQHandler(&hdma_sdio_rx);

}




void SDIO_IRQHandler(void)
{
  /* USER CODE BEGIN SDIO_IRQn 0 */

  /* USER CODE END SDIO_IRQn 0 */
  HAL_SD_IRQHandler(&hsd);
  /* USER CODE BEGIN SDIO_IRQn 1 */

  /* USER CODE END SDIO_IRQn 1 */
}


void EXTI0_IRQHandler(void)
{

}
void EXTI1_IRQHandler(void)
{

}
void EXTI2_IRQHandler(void)
{

}

void EXTI3_IRQHandler(void)
{

}

void EXTI4_IRQHandler(void)
{

}

void EXTI9_5_IRQHandler(void)
{

}



// 인터럽트 핸들러 구현
void EXTI15_10_IRQHandler(void)
{
  void *handle=NULL;;
void (*call)(void *)=NULL;


    // PA10 ~ PA15의 인터럽트 확인 및 클리어
    for (uint16_t pin = GPIO_PIN_10; pin <= GPIO_PIN_15; pin <<= 1)
    {
        if (__HAL_GPIO_EXTI_GET_IT(pin) != RESET)
        {
            __HAL_GPIO_EXTI_CLEAR_IT(pin);  // 인터럽트 플래그 클리어

            // 인터럽트 발생 시 핀별로 수행할 작업
            if (pin == GPIO_PIN_10)
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            }
            else if (pin == GPIO_PIN_11)
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            } 
            else if (pin == GPIO_PIN_12)
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_13)
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_14) 
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
                if(call)
                {
                  call(handle);
                }
            } else if (pin == GPIO_PIN_15) 
            {
                handle = g_mcu_isr_list[getPinNumber(pin)].handle;
                call =  g_mcu_isr_list[getPinNumber(pin)].call;
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