#if 0
#include <stdio.h>
#include <string.h>

#include "dev_io.h"
#include "stm32f4xx.h"


/* IRQ 테이블 정의 */
typedef struct
{
  int irq_num;
  const char* name;
} IRQ_Info;

/* IRQ 이름 테이블 */
IRQ_Info IRQ_Table[] = {{0, "WWDG"},
                        {1, "PVD"},
                        {2, "TAMP_STAMP"},
                        {3, "RTC_WKUP"},
                        {4, "FLASH"},
                        {5, "RCC"},
                        {6, "EXTI0"},
                        {7, "EXTI1"},
                        {8, "EXTI2"},
                        {9, "EXTI3"},
                        {10, "EXTI4"},
                        {11, "DMA1_Stream0"},
                        {12, "DMA1_Stream1"},
                        {13, "DMA1_Stream2"},
                        {14, "DMA1_Stream3"},
                        {15, "DMA1_Stream4"},
                        {16, "DMA1_Stream5"},
                        {17, "DMA1_Stream6"},
                        {18, "ADC"},
                        {19, "CAN1_TX"},
                        {20, "CAN1_RX0"},
                        {21, "CAN1_RX1"},
                        {22, "CAN1_SCE"},
                        {23, "EXTI9_5"},
                        {24, "TIM1_BRK_TIM9"},
                        {25, "TIM1_UP_TIM10"},
                        {26, "TIM1_TRG_COM_TIM11"},
                        {27, "TIM1_CC"},
                        {28, "TIM2"},
                        {29, "TIM3"},
                        {30, "TIM4"},
                        {31, "I2C1_EV"},
                        {32, "I2C1_ER"},
                        {33, "I2C2_EV"},
                        {34, "I2C2_ER"},
                        {35, "SPI1"},
                        {36, "SPI2"},
                        {37, "USART1"},
                        {38, "USART2"},
                        {39, "USART3"},
                        {40, "EXTI15_10"},
                        {41, "RTC_Alarm"},
                        {42, "OTG_FS_WKUP"},
                        {43, "TIM8_BRK_TIM12"},
                        {44, "TIM8_UP_TIM13"},
                        {45, "TIM8_TRG_COM_TIM14"},
                        {46, "TIM8_CC"},
                        {47, "DMA1_Stream7"},
                        {48, "FSMC"},
                        {49, "SDIO"},
                        {50, "TIM5"},
                        {51, "SPI3"},
                        {52, "UART4"},
                        {53, "UART5"},
                        {54, "TIM6_DAC"},
                        {55, "TIM7"},
                        {56, "DMA2_Stream0"},
                        {57, "DMA2_Stream1"},
                        {58, "DMA2_Stream2"},
                        {59, "DMA2_Stream3"},
                        {60, "DMA2_Stream4"},
                        {61, "ETH"},
                        {62, "ETH_WKUP"},
                        {63, "CAN2_TX"},
                        {64, "CAN2_RX0"},
                        {65, "CAN2_RX1"},
                        {66, "CAN2_SCE"},
                        {67, "OTG_FS"},
                        {68, "DMA2_Stream5"},
                        {69, "DMA2_Stream6"},
                        {70, "DMA2_Stream7"},
                        {71, "USART6"},
                        {72, "I2C3_EV"},
                        {73, "I2C3_ER"},
                        {74, "OTG_HS_EP1_OUT"},
                        {75, "OTG_HS_EP1_IN"},
                        {76, "OTG_HS_WKUP"},
                        {77, "OTG_HS"},
                        {78, "DCMI"},
                        {79, "CRYP"},
                        {80, "HASH_RNG"},
                        {81, "FPU"}};
#define NUM_IRQS (sizeof(IRQ_Table) / sizeof(IRQ_Info))

/* EXTI 라인의 GPIO 매핑 확인 */
const char* GetEXTIPortPinMapping(uint8_t exti_line)
{
  static char buffer[32];
  uint8_t port_index;

  if (exti_line <= 3)
  {
    port_index = (SYSCFG->EXTICR[0] >> (exti_line * 4)) & 0xF;
  }
  else if (exti_line <= 7)
  {
    port_index = (SYSCFG->EXTICR[1] >> ((exti_line - 4) * 4)) & 0xF;
  }
  else if (exti_line <= 11)
  {
    port_index = (SYSCFG->EXTICR[2] >> ((exti_line - 8) * 4)) & 0xF;
  }
  else if (exti_line <= 15)
  {
    port_index = (SYSCFG->EXTICR[3] >> ((exti_line - 12) * 4)) & 0xF;
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "Invalid EXTI");
    return buffer;
  }

  const char* port_name = "UNKNOWN";
  switch (port_index)
  {
    case 0:
      port_name = "GPIOA";
      break;
    case 1:
      port_name = "GPIOB";
      break;
    case 2:
      port_name = "GPIOC";
      break;
    case 3:
      port_name = "GPIOD";
      break;
    case 4:
      port_name = "GPIOE";
      break;
    case 5:
      port_name = "GPIOF";
      break;
    case 6:
      port_name = "GPIOG";
      break;
    case 7:
      port_name = "GPIOH";
      break;
    case 8:
      port_name = "GPIOI";
      break;
    default:
      port_name = "UNKNOWN";
      break;
  }

  snprintf(buffer, sizeof(buffer), "%s.PIN%d", port_name, exti_line);
  return buffer;
}

/* EXTI 그룹 인터럽트 매핑 (예: EXTI9_5, EXTI15_10) */
const char* GetEXTIGroupMapping(uint16_t exti_mask, uint8_t start_line)
{
  static char buffer[128];
  char temp[32];
  buffer[0] = '\0';  // 초기화

  for (uint8_t line = start_line; line < start_line + 5; line++)
  {
    if (exti_mask & (1 << line))
    {
      const char* mapping = GetEXTIPortPinMapping(line);
      snprintf(temp, sizeof(temp), "%s,", mapping);
      strncat(buffer, temp, sizeof(buffer) - strlen(buffer) - 1);
    }
  }

  // 마지막 쉼표 제거
  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == ',')
  {
    buffer[len - 1] = '\0';
  }

  return buffer;
}

#include <stdio.h>

#include "stm32f4xx.h"

// DMA1 스트림 요청 매핑 테이블
const char* dma1_mapping[8][8] = {
    {"SPI3_RX", "I2C1_RX", "TIM4_CH1", "I2S3_EXT_RX", "UART5_RX", "UART8_TX",
     "TIM5_CH3", "-"},  // Stream 0
    {"-", "-", "-", "TIM2_UP", "USART3_RX", "UART7_TX", "TIM5_CH4",
     "-"},  // Stream 1
    {"SPI3_RX", "TIM7_UP", "I2S3_EXT_TX", "I2C3_RX", "UART4_RX", "-",
     "TIM5_CH1", "-"},  // Stream 2
    {"SPI2_RX", "-", "TIM4_CH2", "I2S3_EXT_RX", "USART3_TX", "UART4_TX", "-",
     "-"},  // Stream 3
    {"SPI2_TX", "TIM7_UP", "I2S2_EXT_TX", "I2C3_TX", "UART4_TX", "-",
     "TIM5_CH2", "-"},  // Stream 4
    {"SPI3_TX", "I2C1_RX", "I2S3_EXT_TX", "-", "USART3_TX", "DAC1", "-",
     "-"},  // Stream 5
    {"-", "I2C1_TX", "TIM4_UP", "-", "TIM3_CH1", "DAC2", "TIM5_UP",
     "-"},  // Stream 6
    {"SPI3_TX", "I2C1_TX", "TIM4_CH3", "TIM2_CH4", "USART2_RX", "-", "-",
     "-"}  // Stream 7
};

// DMA2 스트림 요청 매핑 테이블
const char* dma2_mapping[8][8] = {
    {"ADC1", "-", "ADC3", "SPI1_RX", "SPI4_RX", "-", "TIM5_CH3",
     "-"},  // Stream 0
    {"-", "DCMI", "ADC3", "-", "-", "USART6_RX", "TIM5_CH4",
     "TIM6_UP"},                               // Stream 1
    {"-", "-", "-", "-", "-", "-", "-", "-"},  // Stream 2 (No usable mapping)
    {"-", "-", "-", "SPI1_TX", "-", "-", "-", "-"},           // Stream 3
    {"ADC1", "-", "-", "-", "USART1_RX", "-", "-", "-"},      // Stream 4
    {"-", "-", "-", "-", "USART1_RX", "-", "-", "-"},         // Stream 5
    {"-", "-", "TIM1_CH1", "-", "-", "USART6_TX", "-", "-"},  // Stream 6
    {"-", "-", "TIM1_CH4", "-", "USART1_TX", "-", "-", "-"}   // Stream 7
};

// DMA 스트림 정보를 반환하는 함수
void Print_DMA_Stream_Peripherals(char* buff, uint16_t buffSize, uint8_t dmaNum,
                                  uint8_t stream)
{
  // 매핑 테이블 선택
  const char*(*mapping)[8] = (dmaNum == 1) ? dma1_mapping : dma2_mapping;

  // DMA 스트림의 채널 선택 확인
  DMA_Stream_TypeDef* dmaStream =
      (dmaNum == 1)
          ? ((DMA_Stream_TypeDef*)((uint32_t)DMA1_Stream0 + stream * 0x18))
          : ((DMA_Stream_TypeDef*)((uint32_t)DMA2_Stream0 + stream * 0x18));
  uint8_t channel = (dmaStream->CR & DMA_SxCR_CHSEL) >> DMA_SxCR_CHSEL_Pos;

  // 스트림 및 채널 정보 출력
  snprintf(buff, buffSize, "%s (Ch:%d)", mapping[stream][channel], channel);
}

/* 소스 디테일 가져오기 */
const char* GetInterruptSourceDetails(IRQn_Type irq_num)
{
  uint32_t reg1;
  uint32_t reg;
  
  static char buffer[256];
  char temp[50] = {0, 0};
  buffer[0] = '\0';
  DMA_Stream_TypeDef* stream;
  switch (irq_num)
  {
    // EXTI (External Interrupts)
    case EXTI0_IRQn:
    case EXTI1_IRQn:
    case EXTI2_IRQn:
    case EXTI3_IRQn:
    case EXTI4_IRQn:
    {
      snprintf(buffer, sizeof(buffer), "IMR=%d,%s",
               (EXTI->IMR & (1 << (irq_num - 6))) != 0,
               GetEXTIPortPinMapping(irq_num - 6));
      break;
    }
    case EXTI9_5_IRQn:
      reg1 = EXTI->IMR;
      snprintf(buffer, sizeof(buffer), "IMR=%04x,%s", (reg1 & 0x03E0),
               GetEXTIGroupMapping((reg1 & 0x03E0), 5));
      break;
    case EXTI15_10_IRQn:
            reg1 = EXTI->IMR;
      snprintf(buffer, sizeof(buffer), "IMR=%04x,%s", (reg1 & 0xFC00),
               GetEXTIGroupMapping((reg1 & 0xFC00), 10));
      break;

    // USART
    case USART1_IRQn:
    case USART2_IRQn:
    case USART3_IRQn:
    case USART6_IRQn:
    {
      USART_TypeDef* usart = (irq_num == USART1_IRQn)   ? USART1
                             : (irq_num == USART2_IRQn) ? USART2
                             : (irq_num == USART3_IRQn) ? USART3
                                                        : USART6;
                                                        
        reg = usart->CR1;
      snprintf(
          buffer, sizeof(buffer), "TE=%d, RE=%d, IDLEIE=%d, TCIE=%d, RXNEIE=%d",
          (reg & USART_CR1_TE) != 0, (reg & USART_CR1_RE) != 0,
          (reg & USART_CR1_IDLEIE) != 0,
          (reg & USART_CR1_TCIE) != 0,
          (reg & USART_CR1_RXNEIE) != 0);
      break;
    }

    // SPI
    case SPI1_IRQn:
    case SPI2_IRQn:
    case SPI3_IRQn:
    {
      SPI_TypeDef* spi = (irq_num == SPI1_IRQn)   ? SPI1
                         : (irq_num == SPI2_IRQn) ? SPI2
                                                  : SPI3;
                                                  
       reg = spi->CR2;
      snprintf(buffer, sizeof(buffer), "TXEIE=%d, RXNEIE=%d, ERRIE=%d",
               (reg & SPI_CR2_TXEIE) != 0,
               (reg & SPI_CR2_RXNEIE) != 0,
               (reg & SPI_CR2_ERRIE) != 0);
      break;
    }

    // DMA
    case DMA1_Stream0_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 0);
      goto DMA_PRINT;
    case DMA1_Stream1_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 1);
      goto DMA_PRINT;
    case DMA1_Stream2_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 2);
      goto DMA_PRINT;
    case DMA1_Stream3_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 3);
      goto DMA_PRINT;
    case DMA1_Stream4_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 4);
      goto DMA_PRINT;
    case DMA1_Stream5_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 5);
      goto DMA_PRINT;
    case DMA1_Stream6_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 6);
      goto DMA_PRINT;
    case DMA1_Stream7_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 7);
      goto DMA_PRINT;
    case DMA2_Stream0_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 0);
      goto DMA_PRINT;
    case DMA2_Stream1_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 1);
      goto DMA_PRINT;
    case DMA2_Stream2_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 2);
      goto DMA_PRINT;
    case DMA2_Stream3_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 3);
      goto DMA_PRINT;
    case DMA2_Stream4_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 4);
      goto DMA_PRINT;
    case DMA2_Stream5_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 5);
      goto DMA_PRINT;
    case DMA2_Stream6_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 6);
      goto DMA_PRINT;
    case DMA2_Stream7_IRQn:
    {
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 7);
      goto DMA_PRINT;
    DMA_PRINT:
      stream = (irq_num >= DMA2_Stream0_IRQn)
                   ? (DMA2_Stream0 + (irq_num - DMA2_Stream0_IRQn))
                   : (DMA1_Stream0 + (irq_num - DMA1_Stream0_IRQn));
      reg = stream->CR;
      snprintf(buffer, sizeof(buffer), "%s,TCIE=%d, HTIE=%d, TEIE=%d", temp,
               (reg & DMA_SxCR_TCIE) != 0,
               (reg & DMA_SxCR_HTIE) != 0,
               (reg & DMA_SxCR_TEIE) != 0);
      break;
    }

    // TIMERS
    case TIM2_IRQn:
    case TIM3_IRQn:
    case TIM4_IRQn:
    case TIM5_IRQn:
    {
      TIM_TypeDef* tim = (irq_num == TIM2_IRQn)   ? TIM2
                         : (irq_num == TIM3_IRQn) ? TIM3
                         : (irq_num == TIM4_IRQn) ? TIM4
                                                  : TIM5;
        reg = tim->DIER;
      snprintf(buffer, sizeof(buffer), "UIE=%d, CC1IE=%d, CC2IE=%d",
               (reg & TIM_DIER_UIE) != 0,
               (reg & TIM_DIER_CC1IE) != 0,
               (reg & TIM_DIER_CC2IE) != 0);
      break;
    }
    case I2C1_EV_IRQn:
    case I2C2_EV_IRQn:
    case I2C3_EV_IRQn:
    {
      I2C_TypeDef* i2c = (irq_num == I2C1_EV_IRQn)   ? I2C1
                         : (irq_num == I2C2_EV_IRQn) ? I2C2
                                                     : I2C3;
       reg = i2c->CR2;
       reg1= i2c->SR1;
      snprintf(buffer, sizeof(buffer),
               "ITBUFEN=%d, ITEVTEN=%d, ADDR=%d, STOPF=%d, RXNE=%d, TXE=%d",
               (reg & I2C_CR2_ITBUFEN) !=
                   0,  // Buffer interrupt enable (TXE/RXNE)
               (reg & I2C_CR2_ITEVTEN) != 0,  // Event interrupt enable
               (reg1 & I2C_SR1_ADDR) != 0,     // Address matched
               (reg1 & I2C_SR1_STOPF) != 0,    // Stop condition detected
               (reg1 & I2C_SR1_RXNE) != 0,     // Receive buffer not empty
               (reg1 & I2C_SR1_TXE) != 0       // Transmit buffer empty
      );
      break;
    }

    case I2C1_ER_IRQn:
    case I2C2_ER_IRQn:
    case I2C3_ER_IRQn:
    {
      I2C_TypeDef* i2c = ((IRQn_Type)irq_num == I2C1_ER_IRQn)   ? I2C1
                         : ((IRQn_Type)irq_num == I2C2_ER_IRQn) ? I2C2
                                                                : I2C3;
       reg = i2c->SR1;
      snprintf(buffer, sizeof(buffer), "BERR=%d, ARLO=%d, AF=%d, OVR=%d",
               (reg & I2C_SR1_BERR) != 0,  // Bus error
               (reg & I2C_SR1_ARLO) != 0,  // Arbitration lost
               (reg & I2C_SR1_AF) != 0,    // Acknowledge failure
               (reg & I2C_SR1_OVR) != 0    // Overrun/Underrun
      );
      break;
    }

    case SDIO_IRQn:
    {
      reg = SDIO->MASK;
      snprintf(
          buffer, sizeof(buffer),
          "CMDRENDIE=%d, CMDSENTIE=%d, DATAENDIE=%d, RXOVERRIE=%d, "
          "TXUNDERRIE=%d, STBITERRIE=%d",
          (reg & SDIO_MASK_CMDRENDIE) != 0,  // Command response received
          (reg & SDIO_MASK_CMDSENTIE) != 0,  // Command sent
          (reg & SDIO_MASK_DATAENDIE) != 0,  // Data transfer end
          (reg & SDIO_MASK_RXOVERRIE) != 0,  // Receive FIFO overrun
          (reg & SDIO_MASK_TXUNDERRIE) != 0,  // Transmit FIFO underrun
          (reg & SDIO_MASK_STBITERRIE) != 0   // Start bit error
      );
      break;
    }
    case TAMP_STAMP_IRQn:
    {
      
      reg = RTC->ISR;
      reg1 = RTC->TAFCR ;
      snprintf(
          buffer, sizeof(buffer),
          "TAMP1IE=%d, TAMP2IE=%d, TIMESTAMPIE=%d, TAMP1F=%d, TAMP2F=%d, "
          "TSF=%d",
          (reg1& RTC_TAFCR_TAMPIE) !=
              0,  // General Tamper interrupt enable
          (reg1 & RTC_TAFCR_TAMP2E) != 0,  // Tamper 2 interrupt enable
          (RTC->CR & RTC_CR_TSIE) != 0,          // Timestamp interrupt enable
          (reg & RTC_ISR_TAMP1F) != 0,      // Tamper 1 flag
          (reg & RTC_ISR_TAMP2F) != 0,      // Tamper 2 flag
          (reg & RTC_ISR_TSF) != 0          // Timestamp flag
      );
      break;
    }

    case RCC_IRQn:
    {
      reg = RCC->CIR;
      snprintf(
          buffer, sizeof(buffer),
          "CSSIE=%d, PLLRDYIE=%d, HSE_RDYIE=%d, HSI_RDYIE=%d, LSE_RDYIE=%d, "
          "LSI_RDYIE=%d",
          (reg & RCC_CIR_CSSC) !=
              0,  // Clock Security System interrupt enable
          (reg& RCC_CIR_PLLRDYIE) != 0,  // PLL Ready interrupt enable
          (reg & RCC_CIR_HSERDYIE) != 0,  // HSE Ready interrupt enable
          (reg & RCC_CIR_HSIRDYIE) != 0,  // HSI Ready interrupt enable
          (reg& RCC_CIR_LSERDYIE) != 0,  // LSE Ready interrupt enable
          (reg & RCC_CIR_LSIRDYIE) != 0   // LSI Ready interrupt enable
      );
      break;
    }
    case RTC_WKUP_IRQn:
    {
      reg = RTC->CR ;
      snprintf(buffer, sizeof(buffer), "WKUPIE=%d, WUTF=%d",
               (reg& RTC_CR_WUTIE) != 0,  // Wakeup Timer interrupt enable
               (RTC->ISR & RTC_ISR_WUTF) != 0  // Wakeup Timer flag
      );
      break;
    }
    case ADC_IRQn:
    {
      reg = ADC1->CR1;
      snprintf(
          buffer, sizeof(buffer), "EOCIE=%d, JEOCIE=%d, AWDIE=%d, OVRIE=%d",
          (reg & ADC_CR1_EOCIE) !=
              0,  // End of Conversion interrupt enable
          (reg & ADC_CR1_JEOCIE) !=
              0,  // Injected End of Conversion interrupt enable
          (reg & ADC_CR1_AWDIE) != 0,  // Analog Watchdog interrupt enable
          (reg & ADC_CR1_OVRIE) != 0   // Overrun interrupt enable
      );
      break;
    }
    case RTC_Alarm_IRQn:
    {
      reg= RTC->CR;
      reg1 = RTC->ISR;
      snprintf(buffer, sizeof(buffer),
               "ALRAIE=%d, ALRBIE=%d, ALRAF=%d, ALRBF=%d",
               (reg & RTC_CR_ALRAIE) != 0,   // Alarm A interrupt enable
               (reg & RTC_CR_ALRBIE) != 0,   // Alarm B interrupt enable
               (reg1 & RTC_ISR_ALRAF) != 0,  // Alarm A flag
               (reg1 & RTC_ISR_ALRBF) != 0   // Alarm B flag
      );
      break;
    }

    default:
      snprintf(buffer, sizeof(buffer), "N/A");
      break;
  }

  return buffer;
}

/* 모든 인터럽트 출력 */
void PrintAllInterrupts(void)
{
  uint32_t iser_value;
  uint8_t priority;
  int irq_num;

  io_printf("Interrupt Vector Table:\r\n");
  io_printf(
      "------------------------------------------------------------------------"
      "-------------------\r\n");
  io_printf(
      "| IRQ Num | Priority | Name                 | Source Details\r\n");
  io_printf(
      "------------------------------------------------------------------------"
      "-------------------\r\n");

  for (irq_num = 0; irq_num < NUM_IRQS; irq_num++)
  {
    // NVIC 활성화 확인
    if (irq_num < 32)
    {
      iser_value = NVIC->ISER[0];
    }
    else if (irq_num < 64)
    {
      iser_value = NVIC->ISER[1];
    }
    else
    {
      iser_value = NVIC->ISER[2];
    }

    int is_enabled = (iser_value & (1 << (irq_num % 32))) ? 1 : 0;
    priority = NVIC->IP[irq_num] >> 4;

    if (is_enabled)
    {
      const char* source_details =
          GetInterruptSourceDetails((IRQn_Type)irq_num);

      io_printf("| %7d | %8d | %-20s | %-80s \r\n", irq_num, priority,
                   IRQ_Table[irq_num].name, source_details);
    }
  }

  io_printf(
      "------------------------------------------------------------------------"
      "-------------------\r\n");
}


#else
#if 0
#include <stdio.h>
#include <string.h>

#include "dev_io.h"
#include "stm32f4xx.h"

/* IRQ 테이블 구조체 */
typedef struct
{
  int irq_num;
  const char* name;
} IRQ_Info;

/* STM32F407 정확한 IRQ 이름 테이블 (82개 인터럽트) */
IRQ_Info IRQ_Table[] = {{0, "WWDG"},
                        {1, "PVD"},
                        {2, "TAMP_STAMP"},
                        {3, "RTC_WKUP"},
                        {4, "FLASH"},
                        {5, "RCC"},
                        {6, "EXTI0"},
                        {7, "EXTI1"},
                        {8, "EXTI2"},
                        {9, "EXTI3"},
                        {10, "EXTI4"},
                        {11, "DMA1_Stream0"},
                        {12, "DMA1_Stream1"},
                        {13, "DMA1_Stream2"},
                        {14, "DMA1_Stream3"},
                        {15, "DMA1_Stream4"},
                        {16, "DMA1_Stream5"},
                        {17, "DMA1_Stream6"},
                        {18, "ADC"},
                        {19, "CAN1_TX"},
                        {20, "CAN1_RX0"},
                        {21, "CAN1_RX1"},
                        {22, "CAN1_SCE"},
                        {23, "EXTI9_5"},
                        {24, "TIM1_BRK_TIM9"},
                        {25, "TIM1_UP_TIM10"},
                        {26, "TIM1_TRG_COM_TIM11"},
                        {27, "TIM1_CC"},
                        {28, "TIM2"},
                        {29, "TIM3"},
                        {30, "TIM4"},
                        {31, "I2C1_EV"},
                        {32, "I2C1_ER"},
                        {33, "I2C2_EV"},
                        {34, "I2C2_ER"},
                        {35, "SPI1"},
                        {36, "SPI2"},
                        {37, "USART1"},
                        {38, "USART2"},
                        {39, "USART3"},
                        {40, "EXTI15_10"},
                        {41, "RTC_Alarm"},
                        {42, "OTG_FS_WKUP"},
                        {43, "TIM8_BRK_TIM12"},
                        {44, "TIM8_UP_TIM13"},
                        {45, "TIM8_TRG_COM_TIM14"},
                        {46, "TIM8_CC"},
                        {47, "DMA1_Stream7"},
                        {48, "FSMC"},
                        {49, "SDIO"},
                        {50, "TIM5"},
                        {51, "SPI3"},
                        {52, "UART4"},
                        {53, "UART5"},
                        {54, "TIM6_DAC"},
                        {55, "TIM7"},
                        {56, "DMA2_Stream0"},
                        {57, "DMA2_Stream1"},
                        {58, "DMA2_Stream2"},
                        {59, "DMA2_Stream3"},
                        {60, "DMA2_Stream4"},
                        {61, "ETH"},
                        {62, "ETH_WKUP"},
                        {63, "CAN2_TX"},
                        {64, "CAN2_RX0"},
                        {65, "CAN2_RX1"},
                        {66, "CAN2_SCE"},
                        {67, "OTG_FS"},
                        {68, "DMA2_Stream5"},
                        {69, "DMA2_Stream6"},
                        {70, "DMA2_Stream7"},
                        {71, "USART6"},
                        {72, "I2C3_EV"},
                        {73, "I2C3_ER"},
                        {74, "OTG_HS_EP1_OUT"},
                        {75, "OTG_HS_EP1_IN"},
                        {76, "OTG_HS_WKUP"},
                        {77, "OTG_HS"},
                        {78, "DCMI"},
                        {79, "CRYP"},
                        {80, "HASH_RNG"},
                        {81, "FPU"}};
#define NUM_IRQS (sizeof(IRQ_Table) / sizeof(IRQ_Info))

/* EXTI 라인의 GPIO 매핑 확인 */
const char* GetEXTIPortPinMapping(uint8_t exti_line)
{
  static char buffer[32];
  uint8_t port_index;

  if (exti_line <= 3)
  {
    port_index = (SYSCFG->EXTICR[0] >> (exti_line * 4)) & 0xF;
  }
  else if (exti_line <= 7)
  {
    port_index = (SYSCFG->EXTICR[1] >> ((exti_line - 4) * 4)) & 0xF;
  }
  else if (exti_line <= 11)
  {
    port_index = (SYSCFG->EXTICR[2] >> ((exti_line - 8) * 4)) & 0xF;
  }
  else if (exti_line <= 15)
  {
    port_index = (SYSCFG->EXTICR[3] >> ((exti_line - 12) * 4)) & 0xF;
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "Invalid EXTI");
    return buffer;
  }

  const char* port_name = "UNKNOWN";
  switch (port_index)
  {
    case 0:
      port_name = "GPIOA";
      break;
    case 1:
      port_name = "GPIOB";
      break;
    case 2:
      port_name = "GPIOC";
      break;
    case 3:
      port_name = "GPIOD";
      break;
    case 4:
      port_name = "GPIOE";
      break;
    case 5:
      port_name = "GPIOF";
      break;
    case 6:
      port_name = "GPIOG";
      break;
    case 7:
      port_name = "GPIOH";
      break;
    case 8:
      port_name = "GPIOI";
      break;
    default:
      port_name = "UNKNOWN";
      break;
  }

  snprintf(buffer, sizeof(buffer), "%s.PIN%d", port_name, exti_line);
  return buffer;
}

/* EXTI 그룹 인터럽트 매핑 */
const char* GetEXTIGroupMapping(uint16_t exti_mask, uint8_t start_line)
{
  static char buffer[128];
  char temp[32];
  buffer[0] = '\0';

  for (uint8_t line = start_line; line < start_line + 5; line++)
  {
    if (exti_mask & (1 << line))
    {
      const char* mapping = GetEXTIPortPinMapping(line);
      snprintf(temp, sizeof(temp), "%s,", mapping);
      strncat(buffer, temp, sizeof(buffer) - strlen(buffer) - 1);
    }
  }

  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == ',')
  {
    buffer[len - 1] = '\0';
  }

  return buffer;
}

/* STM32F407 전용 DMA1 스트림 채널 매핑 테이블 */
const char* dma1_mapping[8][8] = {
    {"SPI3_RX", "I2C1_RX", "TIM4_CH1", "I2S3_EXT_RX", "UART5_RX", "RESERVED", "TIM5_CH3",
     "TIM5_UP"},
    {"RESERVED", "TIM2_UP", "RESERVED", "TIM2_UP", "USART3_RX", "RESERVED", "TIM5_CH4", "TIM6_UP"},
    {"SPI3_RX", "TIM7_UP", "I2S3_EXT_TX", "I2C3_RX", "UART4_RX", "TIM3_CH4", "TIM5_CH1", "I2C2_RX"},
    {"SPI2_RX", "TIM4_CH2", "I2S2_EXT_RX", "I2S3_EXT_TX", "USART3_TX", "UART4_TX", "TIM5_CH4",
     "I2C2_RX"},
    {"SPI2_TX", "TIM7_UP", "I2S2_EXT_TX", "I2C3_TX", "UART4_TX", "TIM3_CH1", "TIM5_CH2",
     "USART3_TX"},
    {"SPI3_TX", "I2C1_RX", "I2S3_EXT_TX", "TIM2_CH1", "USART2_TX", "TIM3_CH2", "DAC1", "I2C3_TX"},
    {"I2C1_TX", "TIM2_CH2", "TIM4_UP", "TIM2_CH1", "TIM3_CH1", "USART2_TX", "TIM5_UP", "DAC2"},
    {"SPI3_TX", "I2C1_TX", "TIM4_CH3", "TIM2_CH4", "UART5_TX", "TIM3_CH3", "I2C2_TX", "TIM4_CC"}};

/* STM32F407 정확한 DMA2 스트림 채널 매핑 테이블 */
const char* dma2_mapping[8][8] = {
    {"ADC1", "RESERVED", "ADC3", "SPI1_RX", "RESERVED", "USART6_RX", "TIM1_CH1/CH2/CH3", "TIM8_UP"},
    {"RESERVED", "DCMI", "ADC3", "RESERVED", "RESERVED", "USART6_RX", "TIM1_CH1", "TIM8_CH1"},
    {"TIM8_CH1/CH2/CH3", "ADC2", "ADC3", "SPI1_RX", "RESERVED", "SPI1_TX", "TIM1_CH1", "TIM8_CH2"},
    {"SPI1_TX", "RESERVED", "RESERVED", "SPI1_TX", "SDIO", "RESERVED", "TIM1_CH4/TRIG/COM",
     "TIM8_CH3"},
    {"ADC1", "RESERVED", "RESERVED", "RESERVED", "USART1_RX", "USART1_RX", "TIM1_UP", "USART1_TX"},
    {"RESERVED", "USART6_RX", "RESERVED", "RESERVED", "USART1_RX", "RESERVED", "TIM1_TRIG",
     "RESERVED"},
    {"TIM1_CH1/CH2/CH3", "TIM1_CH1", "TIM1_CH2", "RESERVED", "SDIO", "USART6_TX", "TIM1_CH3",
     "RESERVED"},
    {"RESERVED", "TIM8_UP", "TIM8_CH4/TRIG/COM", "TIM1_CH4/TRIG/COM", "USART1_TX", "TIM1_UP",
     "USART6_TX", "USART1_TX"}};

/* DMA 스트림 정보를 반환하는 함수 */
void Print_DMA_Stream_Peripherals(char* buff, uint16_t buffSize, uint8_t dmaNum, uint8_t stream)
{
  const char*(*mapping)[8] = (dmaNum == 1) ? dma1_mapping : dma2_mapping;

  DMA_Stream_TypeDef* dmaStream =
      (dmaNum == 1) ? ((DMA_Stream_TypeDef*)((uint32_t)DMA1_Stream0 + stream * 0x18))
                    : ((DMA_Stream_TypeDef*)((uint32_t)DMA2_Stream0 + stream * 0x18));

  uint8_t channel = (dmaStream->CR & DMA_SxCR_CHSEL) >> DMA_SxCR_CHSEL_Pos;

  snprintf(buff, buffSize, "%s (Ch:%d)", mapping[stream][channel], channel);
}

/* 인터럽트 소스 디테일 가져오기 */
const char* GetInterruptSourceDetails(IRQn_Type irq_num)
{
  uint32_t reg1, reg;
  static char buffer[256];
  char temp[50] = {0};
  buffer[0] = '\0';
  DMA_Stream_TypeDef* stream;

  switch (irq_num)
  {
    // EXTI (External Interrupts)
    case EXTI0_IRQn:
    case EXTI1_IRQn:
    case EXTI2_IRQn:
    case EXTI3_IRQn:
    case EXTI4_IRQn:
    {
      snprintf(buffer, sizeof(buffer), "IMR=%d,%s", (EXTI->IMR & (1 << (irq_num - 6))) != 0,
               GetEXTIPortPinMapping(irq_num - 6));
      break;
    }
    case EXTI9_5_IRQn:
      reg1 = EXTI->IMR;
      snprintf(buffer, sizeof(buffer), "IMR=%04x,%s", (reg1 & 0x03E0),
               GetEXTIGroupMapping((reg1 & 0x03E0), 5));
      break;
    case EXTI15_10_IRQn:
      reg1 = EXTI->IMR;
      snprintf(buffer, sizeof(buffer), "IMR=%04x,%s", (reg1 & 0xFC00),
               GetEXTIGroupMapping((reg1 & 0xFC00), 10));
      break;

    // USART/UART (STM32F407에서 지원하는 것만)
    case USART1_IRQn:
    case USART2_IRQn:
    case USART3_IRQn:
    case USART6_IRQn:
    case UART4_IRQn:
    case UART5_IRQn:
    {
      USART_TypeDef* usart = (irq_num == USART1_IRQn)   ? USART1
                             : (irq_num == USART2_IRQn) ? USART2
                             : (irq_num == USART3_IRQn) ? USART3
                             : (irq_num == USART6_IRQn) ? USART6
                             : (irq_num == UART4_IRQn)  ? UART4
                                                        : UART5;
      reg = usart->CR1;
      snprintf(buffer, sizeof(buffer), "TE=%d, RE=%d, IDLEIE=%d, TCIE=%d, RXNEIE=%d",
               (reg & USART_CR1_TE) != 0, (reg & USART_CR1_RE) != 0, (reg & USART_CR1_IDLEIE) != 0,
               (reg & USART_CR1_TCIE) != 0, (reg & USART_CR1_RXNEIE) != 0);
      break;
    }

    // SPI (STM32F407에서 지원하는 것만)
    case SPI1_IRQn:
    case SPI2_IRQn:
    case SPI3_IRQn:
    {
      SPI_TypeDef* spi = (irq_num == SPI1_IRQn) ? SPI1 : (irq_num == SPI2_IRQn) ? SPI2 : SPI3;
      reg = spi->CR2;
      snprintf(buffer, sizeof(buffer), "TXEIE=%d, RXNEIE=%d, ERRIE=%d", (reg & SPI_CR2_TXEIE) != 0,
               (reg & SPI_CR2_RXNEIE) != 0, (reg & SPI_CR2_ERRIE) != 0);
      break;
    }

    // DMA 스트림들
    case DMA1_Stream0_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 0);
      goto DMA_PRINT;
    case DMA1_Stream1_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 1);
      goto DMA_PRINT;
    case DMA1_Stream2_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 2);
      goto DMA_PRINT;
    case DMA1_Stream3_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 3);
      goto DMA_PRINT;
    case DMA1_Stream4_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 4);
      goto DMA_PRINT;
    case DMA1_Stream5_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 5);
      goto DMA_PRINT;
    case DMA1_Stream6_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 6);
      goto DMA_PRINT;
    case DMA1_Stream7_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 7);
      goto DMA_PRINT;
    case DMA2_Stream0_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 0);
      goto DMA_PRINT;
    case DMA2_Stream1_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 1);
      goto DMA_PRINT;
    case DMA2_Stream2_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 2);
      goto DMA_PRINT;
    case DMA2_Stream3_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 3);
      goto DMA_PRINT;
    case DMA2_Stream4_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 4);
      goto DMA_PRINT;
    case DMA2_Stream5_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 5);
      goto DMA_PRINT;
    case DMA2_Stream6_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 6);
      goto DMA_PRINT;
    case DMA2_Stream7_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 7);
      goto DMA_PRINT;

    DMA_PRINT:
      stream = (irq_num >= DMA2_Stream0_IRQn) ? (DMA2_Stream0 + (irq_num - DMA2_Stream0_IRQn))
                                              : (DMA1_Stream0 + (irq_num - DMA1_Stream0_IRQn));
      reg = stream->CR;
      snprintf(buffer, sizeof(buffer), "%s,TCIE=%d, HTIE=%d, TEIE=%d", temp,
               (reg & DMA_SxCR_TCIE) != 0, (reg & DMA_SxCR_HTIE) != 0, (reg & DMA_SxCR_TEIE) != 0);
      break;

    // 타이머들
    case TIM1_BRK_TIM9_IRQn:
      reg = TIM1->DIER;
      reg1 = TIM9->DIER;
      snprintf(buffer, sizeof(buffer), "TIM1_BIE=%d, TIM9_UIE=%d", (reg & TIM_DIER_BIE) != 0,
               (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM1_UP_TIM10_IRQn:
      reg = TIM1->DIER;
      reg1 = TIM10->DIER;
      snprintf(buffer, sizeof(buffer), "TIM1_UIE=%d, TIM10_UIE=%d", (reg & TIM_DIER_UIE) != 0,
               (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM1_TRG_COM_TIM11_IRQn:
      reg = TIM1->DIER;
      reg1 = TIM11->DIER;
      snprintf(buffer, sizeof(buffer), "TIM1_TIE=%d, TIM1_COMIE=%d, TIM11_UIE=%d",
               (reg & TIM_DIER_TIE) != 0, (reg & TIM_DIER_COMIE) != 0, (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM1_CC_IRQn:
      reg = TIM1->DIER;
      snprintf(buffer, sizeof(buffer), "CC1IE=%d, CC2IE=%d, CC3IE=%d, CC4IE=%d",
               (reg & TIM_DIER_CC1IE) != 0, (reg & TIM_DIER_CC2IE) != 0,
               (reg & TIM_DIER_CC3IE) != 0, (reg & TIM_DIER_CC4IE) != 0);
      break;
    case TIM2_IRQn:
    case TIM3_IRQn:
    case TIM4_IRQn:
    case TIM5_IRQn:
    {
      TIM_TypeDef* tim = (irq_num == TIM2_IRQn)   ? TIM2
                         : (irq_num == TIM3_IRQn) ? TIM3
                         : (irq_num == TIM4_IRQn) ? TIM4
                                                  : TIM5;
      reg = tim->DIER;
      snprintf(buffer, sizeof(buffer), "UIE=%d, CC1IE=%d, CC2IE=%d, CC3IE=%d, CC4IE=%d",
               (reg & TIM_DIER_UIE) != 0, (reg & TIM_DIER_CC1IE) != 0, (reg & TIM_DIER_CC2IE) != 0,
               (reg & TIM_DIER_CC3IE) != 0, (reg & TIM_DIER_CC4IE) != 0);
      break;
    }
    case TIM6_DAC_IRQn:
      reg = TIM6->DIER;
      reg1 = DAC->CR;
      snprintf(buffer, sizeof(buffer), "TIM6_UIE=%d, DAC_DMAUDR1IE=%d, DAC_DMAUDR2IE=%d",
               (reg & TIM_DIER_UIE) != 0, (reg1 & DAC_CR_DMAUDRIE1) != 0,
               (reg1 & DAC_CR_DMAUDRIE2) != 0);
      break;
    case TIM7_IRQn:
      reg = TIM7->DIER;
      snprintf(buffer, sizeof(buffer), "UIE=%d", (reg & TIM_DIER_UIE) != 0);
      break;
    case TIM8_BRK_TIM12_IRQn:
      reg = TIM8->DIER;
      reg1 = TIM12->DIER;
      snprintf(buffer, sizeof(buffer), "TIM8_BIE=%d, TIM12_UIE=%d", (reg & TIM_DIER_BIE) != 0,
               (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM8_UP_TIM13_IRQn:
      reg = TIM8->DIER;
      reg1 = TIM13->DIER;
      snprintf(buffer, sizeof(buffer), "TIM8_UIE=%d, TIM13_UIE=%d", (reg & TIM_DIER_UIE) != 0,
               (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM8_TRG_COM_TIM14_IRQn:
      reg = TIM8->DIER;
      reg1 = TIM14->DIER;
      snprintf(buffer, sizeof(buffer), "TIM8_TIE=%d, TIM8_COMIE=%d, TIM14_UIE=%d",
               (reg & TIM_DIER_TIE) != 0, (reg & TIM_DIER_COMIE) != 0, (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM8_CC_IRQn:
      reg = TIM8->DIER;
      snprintf(buffer, sizeof(buffer), "CC1IE=%d, CC2IE=%d, CC3IE=%d, CC4IE=%d",
               (reg & TIM_DIER_CC1IE) != 0, (reg & TIM_DIER_CC2IE) != 0,
               (reg & TIM_DIER_CC3IE) != 0, (reg & TIM_DIER_CC4IE) != 0);
      break;

    // I2C
    case I2C1_EV_IRQn:
    case I2C2_EV_IRQn:
    case I2C3_EV_IRQn:
    {
      I2C_TypeDef* i2c = (irq_num == I2C1_EV_IRQn) ? I2C1 : (irq_num == I2C2_EV_IRQn) ? I2C2 : I2C3;
      reg = i2c->CR2;
      reg1 = i2c->SR1;
      snprintf(buffer, sizeof(buffer), "ITBUFEN=%d, ITEVTEN=%d, ADDR=%d, STOPF=%d, RXNE=%d, TXE=%d",
               (reg & I2C_CR2_ITBUFEN) != 0, (reg & I2C_CR2_ITEVTEN) != 0,
               (reg1 & I2C_SR1_ADDR) != 0, (reg1 & I2C_SR1_STOPF) != 0, (reg1 & I2C_SR1_RXNE) != 0,
               (reg1 & I2C_SR1_TXE) != 0);
      break;
    }
    case I2C1_ER_IRQn:
    case I2C2_ER_IRQn:
    case I2C3_ER_IRQn:
    {
      I2C_TypeDef* i2c = (irq_num == I2C1_ER_IRQn) ? I2C1 : (irq_num == I2C2_ER_IRQn) ? I2C2 : I2C3;
      reg = i2c->SR1;
      snprintf(buffer, sizeof(buffer), "BERR=%d, ARLO=%d, AF=%d, OVR=%d", (reg & I2C_SR1_BERR) != 0,
               (reg & I2C_SR1_ARLO) != 0, (reg & I2C_SR1_AF) != 0, (reg & I2C_SR1_OVR) != 0);
      break;
    }

    // CAN
    case CAN1_TX_IRQn:
    case CAN2_TX_IRQn:
    {
      CAN_TypeDef* can = (irq_num == CAN1_TX_IRQn) ? CAN1 : CAN2;
      reg = can->IER;
      snprintf(buffer, sizeof(buffer), "TMEIE=%d", (reg & CAN_IER_TMEIE) != 0);
      break;
    }
    case CAN1_RX0_IRQn:
    case CAN2_RX0_IRQn:
    {
      CAN_TypeDef* can = (irq_num == CAN1_RX0_IRQn) ? CAN1 : CAN2;
      reg = can->IER;
      snprintf(buffer, sizeof(buffer), "FMPIE0=%d", (reg & CAN_IER_FMPIE0) != 0);
      break;
    }
    case CAN1_RX1_IRQn:
    case CAN2_RX1_IRQn:
    {
      CAN_TypeDef* can = (irq_num == CAN1_RX1_IRQn) ? CAN1 : CAN2;
      reg = can->IER;
      snprintf(buffer, sizeof(buffer), "FMPIE1=%d", (reg & CAN_IER_FMPIE1) != 0);
      break;
    }
    case CAN1_SCE_IRQn:
    case CAN2_SCE_IRQn:
    {
      CAN_TypeDef* can = (irq_num == CAN1_SCE_IRQn) ? CAN1 : CAN2;
      reg = can->IER;
      snprintf(buffer, sizeof(buffer), "EWGIE=%d, EPVIE=%d, BOFIE=%d, LECIE=%d, ERRIE=%d",
               (reg & CAN_IER_EWGIE) != 0, (reg & CAN_IER_EPVIE) != 0, (reg & CAN_IER_BOFIE) != 0,
               (reg & CAN_IER_LECIE) != 0, (reg & CAN_IER_ERRIE) != 0);
      break;
    }


    case SDIO_IRQn:
    {
        reg = SDIO->MASK;
        snprintf(buffer, sizeof(buffer),
                 "CMDRENDIE=%d, CMDSENTIE=%d, DATAENDIE=%d, RXOVERRIE=%d, TXUNDERRIE=%d",
                 (reg & SDIO_MASK_CMDRENDIE) != 0, (reg & SDIO_MASK_CMDSENTIE) != 0,
                 (reg & SDIO_MASK_DATAENDIE) != 0, (reg & SDIO_MASK_RXOVERRIE) != 0,
                 (reg & SDIO_MASK_TXUNDERRIE) != 0);
        break;
    }

    // RTC
    case TAMP_STAMP_IRQn:
    {
        reg = RTC->ISR;
        reg1 = RTC->TAFCR;
        snprintf(buffer, sizeof(buffer),
                 "TAMP1IE=%d, TAMP2IE=%d, TIMESTAMPIE=%d, TAMP1F=%d, TAMP2F=%d, TSF=%d",
                 (reg1 & RTC_TAFCR_TAMPIE) != 0, (reg1 & RTC_TAFCR_TAMP2E) != 0,
                 (RTC->CR & RTC_CR_TSIE) != 0, (reg & RTC_ISR_TAMP1F) != 0,
                 (reg & RTC_ISR_TAMP2F) != 0, (reg & RTC_ISR_TSF) != 0);
        break;
    }
    case RTC_WKUP_IRQn:
    {
        reg = RTC->CR;
        snprintf(buffer, sizeof(buffer), "WKUPIE=%d, WUTF=%d", (reg & RTC_CR_WUTIE) != 0,
                 (RTC->ISR & RTC_ISR_WUTF) != 0);
        break;
    }
    case RTC_Alarm_IRQn:
    {
        reg = RTC->CR;
        reg1 = RTC->ISR;
        snprintf(buffer, sizeof(buffer), "ALRAIE=%d, ALRBIE=%d, ALRAF=%d, ALRBF=%d",
                 (reg & RTC_CR_ALRAIE) != 0, (reg & RTC_CR_ALRBIE) != 0,
                 (reg1 & RTC_ISR_ALRAF) != 0, (reg1 & RTC_ISR_ALRBF) != 0);
        break;
    }

    // System
    case RCC_IRQn:
    {
        reg = RCC->CIR;
        snprintf(buffer, sizeof(buffer),
                 "CSSIE=%d, PLLRDYIE=%d, HSE_RDYIE=%d, HSI_RDYIE=%d, LSE_RDYIE=%d, LSI_RDYIE=%d",
                 (reg & RCC_CIR_CSSC) != 0, (reg & RCC_CIR_PLLRDYIE) != 0,
                 (reg & RCC_CIR_HSERDYIE) != 0, (reg & RCC_CIR_HSIRDYIE) != 0,
                 (reg & RCC_CIR_LSERDYIE) != 0, (reg & RCC_CIR_LSIRDYIE) != 0);
        break;
    }
    case FLASH_IRQn:
    {
        reg = FLASH->CR;
        snprintf(buffer, sizeof(buffer), "EOPIE=%d, ERRIE=%d", (reg & FLASH_CR_EOPIE) != 0,
                 (reg & FLASH_CR_ERRIE) != 0);
        break;
    }
    case PVD_IRQn:
    {
        reg = PWR->CR;
        snprintf(buffer, sizeof(buffer), "PVDE=%d", (reg & PWR_CR_PVDE) != 0);
        break;
    }
    case WWDG_IRQn:
    {
        reg = WWDG->CFR;
        snprintf(buffer, sizeof(buffer), "EWI=%d", (reg & WWDG_CFR_EWI) != 0);
        break;
    }

    // ADC
    case ADC_IRQn:
    {
        reg = ADC1->CR1;
        snprintf(buffer, sizeof(buffer), "EOCIE=%d, JEOCIE=%d, AWDIE=%d, OVRIE=%d",
                 (reg & ADC_CR1_EOCIE) != 0, (reg & ADC_CR1_JEOCIE) != 0,
                 (reg & ADC_CR1_AWDIE) != 0, (reg & ADC_CR1_OVRIE) != 0);
        break;
    }

    // USB OTG
    case OTG_FS_WKUP_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_FS wakeup interrupt");
        break;
    }
    case OTG_FS_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_FS global interrupt");
        break;
    }
    case OTG_HS_EP1_OUT_IRQn:
    case OTG_HS_EP1_IN_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_HS endpoint interrupt");
        break;
    }
    case OTG_HS_WKUP_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_HS wakeup interrupt");
        break;
    }
    case OTG_HS_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_HS global interrupt");
        break;
    }

    // Ethernet
    case ETH_IRQn:
    {
        reg = ETH->DMAIER;
        snprintf(buffer, sizeof(buffer), "ETH_DMA interrupts: TIE=%d, RIE=%d, NIE=%d",
                 (reg & ETH_DMAIER_TIE) != 0, (reg & ETH_DMAIER_RIE) != 0,
                 (reg & ETH_DMAIER_NISE) != 0);
        break;
    }
    case ETH_WKUP_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "Ethernet wakeup interrupt");
        break;
    }

   

    // FSMC
    case FSMC_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "FSMC interrupt");
        break;
    }

    // FPU
    case FPU_IRQn:
    {
        snprintf(buffer, sizeof(buffer),
                 "FPU exceptions: DIV_BY_ZERO, OVERFLOW, UNDERFLOW, INEXACT");
        break;
    }

    default:
      snprintf(buffer, sizeof(buffer), "N/A");
      break;
    }

      return buffer;
  }

  /* 모든 인터럽트 출력 */
  void PrintAllInterrupts(void)
  {
    uint32_t iser_value;
    uint8_t priority;
    int irq_num;

    io_printf("STM32F407 Interrupt Vector Table:\r\n");
    io_printf(
        "===================================================================================\r\n");
    io_printf("| IRQ Num | Priority | Name                 | Source Details\r\n");
    io_printf(
        "===================================================================================\r\n");

    for (irq_num = 0; irq_num < NUM_IRQS; irq_num++)
    {
      // NVIC 활성화 확인
      if (irq_num < 32)
      {
        iser_value = NVIC->ISER[0];
      }
      else if (irq_num < 64)
      {
        iser_value = NVIC->ISER[1];
      }
      else
      {
        iser_value = NVIC->ISER[2];
      }

      int is_enabled = (iser_value & (1 << (irq_num % 32))) ? 1 : 0;
      priority = NVIC->IP[irq_num] >> 4;

      if (is_enabled)
      {
        const char* source_details = GetInterruptSourceDetails((IRQn_Type)irq_num);
        io_printf("| %7d | %8d | %-20s | %-80s \r\n", irq_num, priority, IRQ_Table[irq_num].name,
                  source_details);
      }
    }

    io_printf(
        "===================================================================================\r\n");
    io_printf("STM32F407 Interrupt Vector Table (82 interrupts total):\r\n");
    io_printf("DMA1: General purpose DMA streams for most peripherals\r\n");
    io_printf("DMA2: High-speed DMA streams (ADC, SDIO, SPI1, TIM1/8, USART1/6, DCMI)\r\n");
    io_printf("SDIO: DMA2_Stream3/6 Channel 4, FSMC: External memory, ETH: Gigabit Ethernet\r\n");
    io_printf(
        "===================================================================================\r\n");
  }
#else
#include <stdio.h>
#include <string.h>

#include "dev_io.h"
#include "stm32f4xx.h"

/* IRQ 테이블 구조체 */
typedef struct
{
  int irq_num;
  const char* name;
} IRQ_Info;

/* STM32F407 정확한 IRQ 이름 테이블 (82개 인터럽트) */
IRQ_Info IRQ_Table[] = {{0, "WWDG"},
                        {1, "PVD"},
                        {2, "TAMP_STAMP"},
                        {3, "RTC_WKUP"},
                        {4, "FLASH"},
                        {5, "RCC"},
                        {6, "EXTI0"},
                        {7, "EXTI1"},
                        {8, "EXTI2"},
                        {9, "EXTI3"},
                        {10, "EXTI4"},
                        {11, "DMA1_Stream0"},
                        {12, "DMA1_Stream1"},
                        {13, "DMA1_Stream2"},
                        {14, "DMA1_Stream3"},
                        {15, "DMA1_Stream4"},
                        {16, "DMA1_Stream5"},
                        {17, "DMA1_Stream6"},
                        {18, "ADC"},
                        {19, "CAN1_TX"},
                        {20, "CAN1_RX0"},
                        {21, "CAN1_RX1"},
                        {22, "CAN1_SCE"},
                        {23, "EXTI9_5"},
                        {24, "TIM1_BRK_TIM9"},
                        {25, "TIM1_UP_TIM10"},
                        {26, "TIM1_TRG_COM_TIM11"},
                        {27, "TIM1_CC"},
                        {28, "TIM2"},
                        {29, "TIM3"},
                        {30, "TIM4"},
                        {31, "I2C1_EV"},
                        {32, "I2C1_ER"},
                        {33, "I2C2_EV"},
                        {34, "I2C2_ER"},
                        {35, "SPI1"},
                        {36, "SPI2"},
                        {37, "USART1"},
                        {38, "USART2"},
                        {39, "USART3"},
                        {40, "EXTI15_10"},
                        {41, "RTC_Alarm"},
                        {42, "OTG_FS_WKUP"},
                        {43, "TIM8_BRK_TIM12"},
                        {44, "TIM8_UP_TIM13"},
                        {45, "TIM8_TRG_COM_TIM14"},
                        {46, "TIM8_CC"},
                        {47, "DMA1_Stream7"},
                        {48, "FSMC"},
                        {49, "SDIO"},
                        {50, "TIM5"},
                        {51, "SPI3"},
                        {52, "UART4"},
                        {53, "UART5"},
                        {54, "TIM6_DAC"},
                        {55, "TIM7"},
                        {56, "DMA2_Stream0"},
                        {57, "DMA2_Stream1"},
                        {58, "DMA2_Stream2"},
                        {59, "DMA2_Stream3"},
                        {60, "DMA2_Stream4"},
                        {61, "ETH"},
                        {62, "ETH_WKUP"},
                        {63, "CAN2_TX"},
                        {64, "CAN2_RX0"},
                        {65, "CAN2_RX1"},
                        {66, "CAN2_SCE"},
                        {67, "OTG_FS"},
                        {68, "DMA2_Stream5"},
                        {69, "DMA2_Stream6"},
                        {70, "DMA2_Stream7"},
                        {71, "USART6"},
                        {72, "I2C3_EV"},
                        {73, "I2C3_ER"},
                        {74, "OTG_HS_EP1_OUT"},
                        {75, "OTG_HS_EP1_IN"},
                        {76, "OTG_HS_WKUP"},
                        {77, "OTG_HS"},
                        {78, "DCMI"},
                        {79, "CRYP"},
                        {80, "HASH_RNG"},
                        {81, "FPU"}};
#define NUM_IRQS (sizeof(IRQ_Table) / sizeof(IRQ_Info))

/* EXTI 라인의 GPIO 매핑 확인 */
const char* GetEXTIPortPinMapping(uint8_t exti_line)
{
  static char buffer[32];
  uint8_t port_index;

  if (exti_line <= 3)
  {
    port_index = (SYSCFG->EXTICR[0] >> (exti_line * 4)) & 0xF;
  }
  else if (exti_line <= 7)
  {
    port_index = (SYSCFG->EXTICR[1] >> ((exti_line - 4) * 4)) & 0xF;
  }
  else if (exti_line <= 11)
  {
    port_index = (SYSCFG->EXTICR[2] >> ((exti_line - 8) * 4)) & 0xF;
  }
  else if (exti_line <= 15)
  {
    port_index = (SYSCFG->EXTICR[3] >> ((exti_line - 12) * 4)) & 0xF;
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "Invalid EXTI");
    return buffer;
  }

  const char* port_name = "UNKNOWN";
  switch (port_index)
  {
    case 0:
      port_name = "GPIOA";
      break;
    case 1:
      port_name = "GPIOB";
      break;
    case 2:
      port_name = "GPIOC";
      break;
    case 3:
      port_name = "GPIOD";
      break;
    case 4:
      port_name = "GPIOE";
      break;
    case 5:
      port_name = "GPIOF";
      break;
    case 6:
      port_name = "GPIOG";
      break;
    case 7:
      port_name = "GPIOH";
      break;
    case 8:
      port_name = "GPIOI";
      break;
    default:
      port_name = "UNKNOWN";
      break;
  }

  snprintf(buffer, sizeof(buffer), "%s.PIN%d", port_name, exti_line);
  return buffer;
}

/* EXTI 그룹 인터럽트 매핑 */
const char* GetEXTIGroupMapping(uint16_t exti_mask, uint8_t start_line)
{
  static char buffer[128];
  char temp[32];
  buffer[0] = '\0';

  for (uint8_t line = start_line; line < start_line + 5; line++)
  {
    if (exti_mask & (1 << line))
    {
      const char* mapping = GetEXTIPortPinMapping(line);
      snprintf(temp, sizeof(temp), "%s,", mapping);
      strncat(buffer, temp, sizeof(buffer) - strlen(buffer) - 1);
    }
  }

  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == ',')
  {
    buffer[len - 1] = '\0';
  }

  return buffer;
}

/* STM32F407 공식 DMA1 스트림 채널 매핑 테이블 (RM0090 Table 43) */
const char* dma1_mapping[8][8] = {
    {"SPI3_RX", "I2C1_RX", "TIM4_CH1", "I2S3_EXT_RX", "UART5_RX", "UART8_TX", "TIM5_CH3/TIM5_UP",
     "SPI3_TX"},
    {"-", "-", "TIM7_UP", "-", "TIM7_UP", "I2C1_RX", "I2C1_TX", "I2C1_TX"},
    {"SPI3_RX", "-", "I2S3_EXT_TX", "TIM4_CH2", "I2S2_EXT_TX", "I2S3_EXT_TX", "TIM4_UP",
     "TIM4_CH3"},
    {"I2S3_EXT_RX", "TIM2_UP/TIM2_CH3", "I2C3_RX", "I2S2_EXT_RX", "I2C3_TX", "TIM2_CH1",
     "TIM2_CH2/TIM2_CH4", "TIM2_UP/TIM2_CH4"},
    {"UART5_RX", "USART3_RX", "UART4_RX", "USART3_TX", "UART4_TX", "USART2_RX", "USART2_TX",
     "UART5_TX"},
    {"UART8_TX", "UART7_TX", "TIM3_CH4/TIM3_UP", "UART7_RX", "TIM3_CH1/TIM3_TRIG", "TIM3_CH2",
     "UART8_RX", "TIM3_CH3"},
    {"TIM5_CH3/TIM5_UP", "TIM5_CH4/TIM5_TRIG", "TIM5_CH1", "TIM5_CH4/TIM5_TRIG", "TIM5_CH2", "-",
     "TIM5_UP", "-"},
    {"-", "TIM6_UP", "I2C2_RX", "I2C2_RX", "USART3_TX", "DAC1", "DAC2", "I2C2_TX"}};

/* STM32F407 공식 DMA2 스트림 채널 매핑 테이블 (RM0090 Table 44) */
const char* dma2_mapping[8][8] = {
    {"ADC1", "-", "ADC3", "SPI1_RX", "SPI4_RX", "-", "TIM1_CH1/TIM1_CH2/TIM1_CH3", "-"},
    {"-", "DCMI", "ADC3", "-", "-", "USART6_RX", "SPI6_RX", "DCMI"},
    {"TIM8_CH1/TIM8_CH2/TIM8_CH3", "ADC2", "-", "SPI1_RX", "-", "USART6_RX", "CRYP_OUT", "HASH_IN"},
    {"SPI1_RX", "-", "SPI1_RX", "SPI1_TX", "-", "SPI1_TX", "-", "-"},
    {"SPI4_RX", "SPI4_TX", "USART1_RX", "SDIO_RX", "-", "USART1_RX", "SDIO_TX", "USART1_TX"},
    {"-", "USART6_RX", "USART6_RX", "SPI4_RX", "SPI4_TX", "-", "USART6_TX", "USART6_TX"},
    {"TIM1_TRIG", "TIM1_CH1", "TIM1_CH2", "TIM1_CH1", "TIM1_CH4/TIM1_TRIG/TIM1_COM", "TIM1_UP",
     "TIM1_CH3", "-"},
    {"-", "TIM8_UP", "TIM8_CH1", "TIM8_CH2", "TIM8_CH3", "SPI5_RX", "SPI5_TX",
     "TIM8_CH4/TIM8_TRIG/TIM8_COM"}};

/* DMA 스트림 정보를 반환하는 함수 (상세 정보 포함) */
void Print_DMA_Stream_Peripherals(char* buff, uint16_t buffSize, uint8_t dmaNum, uint8_t stream)
{
  const char*(*mapping)[8] = (dmaNum == 1) ? dma1_mapping : dma2_mapping;

  DMA_Stream_TypeDef* dmaStream =
      (dmaNum == 1) ? ((DMA_Stream_TypeDef*)((uint32_t)DMA1_Stream0 + stream * 0x18))
                    : ((DMA_Stream_TypeDef*)((uint32_t)DMA2_Stream0 + stream * 0x18));

  uint8_t channel = (dmaStream->CR & DMA_SxCR_CHSEL) >> DMA_SxCR_CHSEL_Pos;
  uint32_t cr = dmaStream->CR;

  // 방향 확인
  const char* direction = "";
  if (cr & DMA_SxCR_DIR_1)
  {
    direction = "_M2M";  // Memory to Memory
  }
  else if (cr & DMA_SxCR_DIR_0)
  {
    direction = "_M2P";  // Memory to Peripheral
  }
  else
  {
    direction = "_P2M";  // Peripheral to Memory
  }

  // 활성화 상태 확인
  const char* status = (cr & DMA_SxCR_EN) ? "ACTIVE" : "IDLE";

  snprintf(buff, buffSize, "%s (Ch:%d%s,%s)", mapping[stream][channel], channel, direction, status);
}

/* 인터럽트 소스 디테일 가져오기 */
const char* GetInterruptSourceDetails(IRQn_Type irq_num)
{
  uint32_t reg1, reg;
  static char buffer[256];
  char temp[50] = {0};
  buffer[0] = '\0';
  DMA_Stream_TypeDef* stream;

  switch (irq_num)
  {
    // EXTI (External Interrupts)
    case EXTI0_IRQn:
    case EXTI1_IRQn:
    case EXTI2_IRQn:
    case EXTI3_IRQn:
    case EXTI4_IRQn:
    {
      snprintf(buffer, sizeof(buffer), "IMR=%d,%s", (EXTI->IMR & (1 << (irq_num - 6))) != 0,
               GetEXTIPortPinMapping(irq_num - 6));
      break;
    }
    case EXTI9_5_IRQn:
      reg1 = EXTI->IMR;
      snprintf(buffer, sizeof(buffer), "IMR=%04x,%s", (reg1 & 0x03E0),
               GetEXTIGroupMapping((reg1 & 0x03E0), 5));
      break;
    case EXTI15_10_IRQn:
      reg1 = EXTI->IMR;
      snprintf(buffer, sizeof(buffer), "IMR=%04x,%s", (reg1 & 0xFC00),
               GetEXTIGroupMapping((reg1 & 0xFC00), 10));
      break;

    // USART/UART (STM32F407에서 지원하는 것만)
    case USART1_IRQn:
    case USART2_IRQn:
    case USART3_IRQn:
    case USART6_IRQn:
    case UART4_IRQn:
    case UART5_IRQn:
    {
      USART_TypeDef* usart = (irq_num == USART1_IRQn)   ? USART1
                             : (irq_num == USART2_IRQn) ? USART2
                             : (irq_num == USART3_IRQn) ? USART3
                             : (irq_num == USART6_IRQn) ? USART6
                             : (irq_num == UART4_IRQn)  ? UART4
                                                        : UART5;
      reg = usart->CR1;
      snprintf(buffer, sizeof(buffer), "TE=%d, RE=%d, IDLEIE=%d, TCIE=%d, RXNEIE=%d",
               (reg & USART_CR1_TE) != 0, (reg & USART_CR1_RE) != 0, (reg & USART_CR1_IDLEIE) != 0,
               (reg & USART_CR1_TCIE) != 0, (reg & USART_CR1_RXNEIE) != 0);
      break;
    }

    // SPI (STM32F407에서 지원하는 것만)
    case SPI1_IRQn:
    case SPI2_IRQn:
    case SPI3_IRQn:
    {
      SPI_TypeDef* spi = (irq_num == SPI1_IRQn) ? SPI1 : (irq_num == SPI2_IRQn) ? SPI2 : SPI3;
      reg = spi->CR2;
      snprintf(buffer, sizeof(buffer), "TXEIE=%d, RXNEIE=%d, ERRIE=%d", (reg & SPI_CR2_TXEIE) != 0,
               (reg & SPI_CR2_RXNEIE) != 0, (reg & SPI_CR2_ERRIE) != 0);
      break;
    }

    // DMA 스트림들
    case DMA1_Stream0_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 0);
      goto DMA_PRINT;
    case DMA1_Stream1_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 1);
      goto DMA_PRINT;
    case DMA1_Stream2_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 2);
      goto DMA_PRINT;
    case DMA1_Stream3_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 3);
      goto DMA_PRINT;
    case DMA1_Stream4_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 4);
      goto DMA_PRINT;
    case DMA1_Stream5_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 5);
      goto DMA_PRINT;
    case DMA1_Stream6_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 6);
      goto DMA_PRINT;
    case DMA1_Stream7_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 1, 7);
      goto DMA_PRINT;
    case DMA2_Stream0_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 0);
      goto DMA_PRINT;
    case DMA2_Stream1_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 1);
      goto DMA_PRINT;
    case DMA2_Stream2_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 2);
      goto DMA_PRINT;
    case DMA2_Stream3_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 3);
      goto DMA_PRINT;
    case DMA2_Stream4_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 4);
      goto DMA_PRINT;
    case DMA2_Stream5_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 5);
      goto DMA_PRINT;
    case DMA2_Stream6_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 6);
      goto DMA_PRINT;
    case DMA2_Stream7_IRQn:
      Print_DMA_Stream_Peripherals(temp, sizeof(temp), 2, 7);
      goto DMA_PRINT;

    DMA_PRINT:
      stream = (irq_num >= DMA2_Stream0_IRQn) ? (DMA2_Stream0 + (irq_num - DMA2_Stream0_IRQn))
                                              : (DMA1_Stream0 + (irq_num - DMA1_Stream0_IRQn));
      reg = stream->CR;
      snprintf(buffer, sizeof(buffer), "%s,TCIE=%d, HTIE=%d, TEIE=%d", temp,
               (reg & DMA_SxCR_TCIE) != 0, (reg & DMA_SxCR_HTIE) != 0, (reg & DMA_SxCR_TEIE) != 0);
      break;

    // 타이머들
    case TIM1_BRK_TIM9_IRQn:
      reg = TIM1->DIER;
      reg1 = TIM9->DIER;
      snprintf(buffer, sizeof(buffer), "TIM1_BIE=%d, TIM9_UIE=%d", (reg & TIM_DIER_BIE) != 0,
               (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM1_UP_TIM10_IRQn:
      reg = TIM1->DIER;
      reg1 = TIM10->DIER;
      snprintf(buffer, sizeof(buffer), "TIM1_UIE=%d, TIM10_UIE=%d", (reg & TIM_DIER_UIE) != 0,
               (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM1_TRG_COM_TIM11_IRQn:
      reg = TIM1->DIER;
      reg1 = TIM11->DIER;
      snprintf(buffer, sizeof(buffer), "TIM1_TIE=%d, TIM1_COMIE=%d, TIM11_UIE=%d",
               (reg & TIM_DIER_TIE) != 0, (reg & TIM_DIER_COMIE) != 0, (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM1_CC_IRQn:
      reg = TIM1->DIER;
      snprintf(buffer, sizeof(buffer), "CC1IE=%d, CC2IE=%d, CC3IE=%d, CC4IE=%d",
               (reg & TIM_DIER_CC1IE) != 0, (reg & TIM_DIER_CC2IE) != 0,
               (reg & TIM_DIER_CC3IE) != 0, (reg & TIM_DIER_CC4IE) != 0);
      break;
    case TIM2_IRQn:
    case TIM3_IRQn:
    case TIM4_IRQn:
    case TIM5_IRQn:
    {
      TIM_TypeDef* tim = (irq_num == TIM2_IRQn)   ? TIM2
                         : (irq_num == TIM3_IRQn) ? TIM3
                         : (irq_num == TIM4_IRQn) ? TIM4
                                                  : TIM5;
      reg = tim->DIER;
      snprintf(buffer, sizeof(buffer), "UIE=%d, CC1IE=%d, CC2IE=%d, CC3IE=%d, CC4IE=%d",
               (reg & TIM_DIER_UIE) != 0, (reg & TIM_DIER_CC1IE) != 0, (reg & TIM_DIER_CC2IE) != 0,
               (reg & TIM_DIER_CC3IE) != 0, (reg & TIM_DIER_CC4IE) != 0);
      break;
    }
    case TIM6_DAC_IRQn:
      reg = TIM6->DIER;
      reg1 = DAC->CR;
      snprintf(buffer, sizeof(buffer), "TIM6_UIE=%d, DAC_DMAUDR1IE=%d, DAC_DMAUDR2IE=%d",
               (reg & TIM_DIER_UIE) != 0, (reg1 & DAC_CR_DMAUDRIE1) != 0,
               (reg1 & DAC_CR_DMAUDRIE2) != 0);
      break;
    case TIM7_IRQn:
      reg = TIM7->DIER;
      snprintf(buffer, sizeof(buffer), "UIE=%d", (reg & TIM_DIER_UIE) != 0);
      break;
    case TIM8_BRK_TIM12_IRQn:
      reg = TIM8->DIER;
      reg1 = TIM12->DIER;
      snprintf(buffer, sizeof(buffer), "TIM8_BIE=%d, TIM12_UIE=%d", (reg & TIM_DIER_BIE) != 0,
               (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM8_UP_TIM13_IRQn:
      reg = TIM8->DIER;
      reg1 = TIM13->DIER;
      snprintf(buffer, sizeof(buffer), "TIM8_UIE=%d, TIM13_UIE=%d", (reg & TIM_DIER_UIE) != 0,
               (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM8_TRG_COM_TIM14_IRQn:
      reg = TIM8->DIER;
      reg1 = TIM14->DIER;
      snprintf(buffer, sizeof(buffer), "TIM8_TIE=%d, TIM8_COMIE=%d, TIM14_UIE=%d",
               (reg & TIM_DIER_TIE) != 0, (reg & TIM_DIER_COMIE) != 0, (reg1 & TIM_DIER_UIE) != 0);
      break;
    case TIM8_CC_IRQn:
      reg = TIM8->DIER;
      snprintf(buffer, sizeof(buffer), "CC1IE=%d, CC2IE=%d, CC3IE=%d, CC4IE=%d",
               (reg & TIM_DIER_CC1IE) != 0, (reg & TIM_DIER_CC2IE) != 0,
               (reg & TIM_DIER_CC3IE) != 0, (reg & TIM_DIER_CC4IE) != 0);
      break;

    // I2C
    case I2C1_EV_IRQn:
    case I2C2_EV_IRQn:
    case I2C3_EV_IRQn:
    {
      I2C_TypeDef* i2c = (irq_num == I2C1_EV_IRQn) ? I2C1 : (irq_num == I2C2_EV_IRQn) ? I2C2 : I2C3;
      reg = i2c->CR2;
      reg1 = i2c->SR1;
      snprintf(buffer, sizeof(buffer), "ITBUFEN=%d, ITEVTEN=%d, ADDR=%d, STOPF=%d, RXNE=%d, TXE=%d",
               (reg & I2C_CR2_ITBUFEN) != 0, (reg & I2C_CR2_ITEVTEN) != 0,
               (reg1 & I2C_SR1_ADDR) != 0, (reg1 & I2C_SR1_STOPF) != 0, (reg1 & I2C_SR1_RXNE) != 0,
               (reg1 & I2C_SR1_TXE) != 0);
      break;
    }
    case I2C1_ER_IRQn:
    case I2C2_ER_IRQn:
    case I2C3_ER_IRQn:
    {
      I2C_TypeDef* i2c = (irq_num == I2C1_ER_IRQn) ? I2C1 : (irq_num == I2C2_ER_IRQn) ? I2C2 : I2C3;
      reg = i2c->SR1;
      snprintf(buffer, sizeof(buffer), "BERR=%d, ARLO=%d, AF=%d, OVR=%d", (reg & I2C_SR1_BERR) != 0,
               (reg & I2C_SR1_ARLO) != 0, (reg & I2C_SR1_AF) != 0, (reg & I2C_SR1_OVR) != 0);
      break;
    }

    // CAN
    case CAN1_TX_IRQn:
    case CAN2_TX_IRQn:
    {
      CAN_TypeDef* can = (irq_num == CAN1_TX_IRQn) ? CAN1 : CAN2;
      reg = can->IER;
      snprintf(buffer, sizeof(buffer), "TMEIE=%d", (reg & CAN_IER_TMEIE) != 0);
      break;
    }
    case CAN1_RX0_IRQn:
    case CAN2_RX0_IRQn:
    {
      CAN_TypeDef* can = (irq_num == CAN1_RX0_IRQn) ? CAN1 : CAN2;
      reg = can->IER;
      snprintf(buffer, sizeof(buffer), "FMPIE0=%d", (reg & CAN_IER_FMPIE0) != 0);
      break;
    }
    case CAN1_RX1_IRQn:
    case CAN2_RX1_IRQn:
    {
      CAN_TypeDef* can = (irq_num == CAN1_RX1_IRQn) ? CAN1 : CAN2;
      reg = can->IER;
      snprintf(buffer, sizeof(buffer), "FMPIE1=%d", (reg & CAN_IER_FMPIE1) != 0);
      break;
    }
    case CAN1_SCE_IRQn:
    case CAN2_SCE_IRQn:
    {
      CAN_TypeDef* can = (irq_num == CAN1_SCE_IRQn) ? CAN1 : CAN2;
      reg = can->IER;
      snprintf(buffer, sizeof(buffer), "EWGIE=%d, EPVIE=%d, BOFIE=%d, LECIE=%d, ERRIE=%d",
               (reg & CAN_IER_EWGIE) != 0, (reg & CAN_IER_EPVIE) != 0, (reg & CAN_IER_BOFIE) != 0,
               (reg & CAN_IER_LECIE) != 0, (reg & CAN_IER_ERRIE) != 0);
      break;
    }


    case SDIO_IRQn:
    {
        reg = SDIO->MASK;
        snprintf(buffer, sizeof(buffer),
                 "CMDRENDIE=%d, CMDSENTIE=%d, DATAENDIE=%d, RXOVERRIE=%d, TXUNDERRIE=%d",
                 (reg & SDIO_MASK_CMDRENDIE) != 0, (reg & SDIO_MASK_CMDSENTIE) != 0,
                 (reg & SDIO_MASK_DATAENDIE) != 0, (reg & SDIO_MASK_RXOVERRIE) != 0,
                 (reg & SDIO_MASK_TXUNDERRIE) != 0);
        break;
    }

    // RTC
    case TAMP_STAMP_IRQn:
    {
        reg = RTC->ISR;
        reg1 = RTC->TAFCR;
        snprintf(buffer, sizeof(buffer),
                 "TAMP1IE=%d, TAMP2IE=%d, TIMESTAMPIE=%d, TAMP1F=%d, TAMP2F=%d, TSF=%d",
                 (reg1 & RTC_TAFCR_TAMPIE) != 0, (reg1 & RTC_TAFCR_TAMP2E) != 0,
                 (RTC->CR & RTC_CR_TSIE) != 0, (reg & RTC_ISR_TAMP1F) != 0,
                 (reg & RTC_ISR_TAMP2F) != 0, (reg & RTC_ISR_TSF) != 0);
        break;
    }
    case RTC_WKUP_IRQn:
    {
        reg = RTC->CR;
        snprintf(buffer, sizeof(buffer), "WKUPIE=%d, WUTF=%d", (reg & RTC_CR_WUTIE) != 0,
                 (RTC->ISR & RTC_ISR_WUTF) != 0);
        break;
    }
    case RTC_Alarm_IRQn:
    {
        reg = RTC->CR;
        reg1 = RTC->ISR;
        snprintf(buffer, sizeof(buffer), "ALRAIE=%d, ALRBIE=%d, ALRAF=%d, ALRBF=%d",
                 (reg & RTC_CR_ALRAIE) != 0, (reg & RTC_CR_ALRBIE) != 0,
                 (reg1 & RTC_ISR_ALRAF) != 0, (reg1 & RTC_ISR_ALRBF) != 0);
        break;
    }

    // System
    case RCC_IRQn:
    {
        reg = RCC->CIR;
        snprintf(buffer, sizeof(buffer),
                 "CSSIE=%d, PLLRDYIE=%d, HSE_RDYIE=%d, HSI_RDYIE=%d, LSE_RDYIE=%d, LSI_RDYIE=%d",
                 (reg & RCC_CIR_CSSC) != 0, (reg & RCC_CIR_PLLRDYIE) != 0,
                 (reg & RCC_CIR_HSERDYIE) != 0, (reg & RCC_CIR_HSIRDYIE) != 0,
                 (reg & RCC_CIR_LSERDYIE) != 0, (reg & RCC_CIR_LSIRDYIE) != 0);
        break;
    }
    case FLASH_IRQn:
    {
        reg = FLASH->CR;
        snprintf(buffer, sizeof(buffer), "EOPIE=%d, ERRIE=%d", (reg & FLASH_CR_EOPIE) != 0,
                 (reg & FLASH_CR_ERRIE) != 0);
        break;
    }
    case PVD_IRQn:
    {
        reg = PWR->CR;
        snprintf(buffer, sizeof(buffer), "PVDE=%d", (reg & PWR_CR_PVDE) != 0);
        break;
    }
    case WWDG_IRQn:
    {
        reg = WWDG->CFR;
        snprintf(buffer, sizeof(buffer), "EWI=%d", (reg & WWDG_CFR_EWI) != 0);
        break;
    }

    // ADC
    case ADC_IRQn:
    {
        reg = ADC1->CR1;
        snprintf(buffer, sizeof(buffer), "EOCIE=%d, JEOCIE=%d, AWDIE=%d, OVRIE=%d",
                 (reg & ADC_CR1_EOCIE) != 0, (reg & ADC_CR1_JEOCIE) != 0,
                 (reg & ADC_CR1_AWDIE) != 0, (reg & ADC_CR1_OVRIE) != 0);
        break;
    }

    // USB OTG
    case OTG_FS_WKUP_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_FS wakeup interrupt");
        break;
    }
    case OTG_FS_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_FS global interrupt");
        break;
    }
    case OTG_HS_EP1_OUT_IRQn:
    case OTG_HS_EP1_IN_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_HS endpoint interrupt");
        break;
    }
    case OTG_HS_WKUP_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_HS wakeup interrupt");
        break;
    }
    case OTG_HS_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "USB_HS global interrupt");
        break;
    }

    // Ethernet
    case ETH_IRQn:
    {
        reg = ETH->DMAIER;
        snprintf(buffer, sizeof(buffer), "ETH_DMA interrupts: TIE=%d, RIE=%d, NIE=%d",
                 (reg & ETH_DMAIER_TIE) != 0, (reg & ETH_DMAIER_RIE) != 0,
                 (reg & ETH_DMAIER_NISE) != 0);
        break;
    }
    case ETH_WKUP_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "Ethernet wakeup interrupt");
        break;
    }



    // DCMI
    case DCMI_IRQn:
    {
        reg = DCMI->IER;
        snprintf(buffer, sizeof(buffer),
                 "FRAME_IE=%d, OVR_IE=%d, ERR_IE=%d, VSYNC_IE=%d, LINE_IE=%d",
                 (reg & DCMI_IER_FRAME_IE) != 0, (reg & DCMI_IER_OVR_IE) != 0,
                 (reg & DCMI_IER_ERR_IE) != 0, (reg & DCMI_IER_VSYNC_IE) != 0,
                 (reg & DCMI_IER_LINE_IE) != 0);
        break;
    }

    // FSMC
    case FSMC_IRQn:
    {
        snprintf(buffer, sizeof(buffer), "FSMC interrupt");
        break;
    }

    // FPU
    case FPU_IRQn:
    {
        snprintf(buffer, sizeof(buffer),
                 "FPU exceptions: DIV_BY_ZERO, OVERFLOW, UNDERFLOW, INEXACT");
        break;
    }

    default:
      snprintf(buffer, sizeof(buffer), "N/A");
      break;
    }

      return buffer;
  }

  /* 모든 인터럽트 출력 */
  void PrintAllInterrupts(void)
  {
    uint32_t iser_value;
    uint8_t priority;
    int irq_num;

    io_printf("STM32F407 Interrupt Vector Table:\r\n");
    io_printf(
        "===================================================================================\r\n");
    io_printf("| IRQ Num | Priority | Name                 | Source Details\r\n");
    io_printf(
        "===================================================================================\r\n");

    for (irq_num = 0; irq_num < NUM_IRQS; irq_num++)
    {
      // NVIC 활성화 확인
      if (irq_num < 32)
      {
        iser_value = NVIC->ISER[0];
      }
      else if (irq_num < 64)
      {
        iser_value = NVIC->ISER[1];
      }
      else
      {
        iser_value = NVIC->ISER[2];
      }

      int is_enabled = (iser_value & (1 << (irq_num % 32))) ? 1 : 0;
      priority = NVIC->IP[irq_num] >> 4;

      if (is_enabled)
      {
        const char* source_details = GetInterruptSourceDetails((IRQn_Type)irq_num);
        io_printf("| %7d | %8d | %-20s | %-80s \r\n", irq_num, priority, IRQ_Table[irq_num].name,
                  source_details);
      }
    }

    io_printf(
        "===================================================================================\r\n");
    io_printf("STM32F407 Interrupt Vector Table (82 interrupts total) - RM0090 Rev 21\r\n");
    io_printf("DMA1: General purpose DMA streams (UART, SPI2/3, I2C, TIM, DAC)\r\n");
    io_printf("DMA2: High-speed DMA streams (ADC, SDIO, SPI1/4/5, TIM1/8, USART1/6, DCMI)\r\n");
    io_printf("SDIO: DMA2_Stream3(RX)/DMA2_Stream6(TX) Channel 4\r\n");
    io_printf(
        "Direction: P2M=Peripheral to Memory, M2P=Memory to Peripheral, M2M=Memory to Memory\r\n");
    io_printf(
        "===================================================================================\r\n");
  }
#endif

#endif