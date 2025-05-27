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
      snprintf(buffer, sizeof(buffer), "IMR=%04x,%s", (EXTI->IMR & 0x03E0),
               GetEXTIGroupMapping((EXTI->IMR & 0x03E0), 5));
      break;
    case EXTI15_10_IRQn:
      snprintf(buffer, sizeof(buffer), "IMR=%04x,%s", (EXTI->IMR & 0xFC00),
               GetEXTIGroupMapping((EXTI->IMR & 0xFC00), 10));
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
      snprintf(
          buffer, sizeof(buffer), "TE=%d, RE=%d, IDLEIE=%d, TCIE=%d, RXNEIE=%d",
          (usart->CR1 & USART_CR1_TE) != 0, (usart->CR1 & USART_CR1_RE) != 0,
          (usart->CR1 & USART_CR1_IDLEIE) != 0,
          (usart->CR1 & USART_CR1_TCIE) != 0,
          (usart->CR1 & USART_CR1_RXNEIE) != 0);
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
      snprintf(buffer, sizeof(buffer), "TXEIE=%d, RXNEIE=%d, ERRIE=%d",
               (spi->CR2 & SPI_CR2_TXEIE) != 0,
               (spi->CR2 & SPI_CR2_RXNEIE) != 0,
               (spi->CR2 & SPI_CR2_ERRIE) != 0);
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

      snprintf(buffer, sizeof(buffer), "%s,TCIE=%d, HTIE=%d, TEIE=%d", temp,
               (stream->CR & DMA_SxCR_TCIE) != 0,
               (stream->CR & DMA_SxCR_HTIE) != 0,
               (stream->CR & DMA_SxCR_TEIE) != 0);
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
      snprintf(buffer, sizeof(buffer), "UIE=%d, CC1IE=%d, CC2IE=%d",
               (tim->DIER & TIM_DIER_UIE) != 0,
               (tim->DIER & TIM_DIER_CC1IE) != 0,
               (tim->DIER & TIM_DIER_CC2IE) != 0);
      break;
    }
    case I2C1_EV_IRQn:
    case I2C2_EV_IRQn:
    case I2C3_EV_IRQn:
    {
      I2C_TypeDef* i2c = (irq_num == I2C1_EV_IRQn)   ? I2C1
                         : (irq_num == I2C2_EV_IRQn) ? I2C2
                                                     : I2C3;
      snprintf(buffer, sizeof(buffer),
               "ITBUFEN=%d, ITEVTEN=%d, ADDR=%d, STOPF=%d, RXNE=%d, TXE=%d",
               (i2c->CR2 & I2C_CR2_ITBUFEN) !=
                   0,  // Buffer interrupt enable (TXE/RXNE)
               (i2c->CR2 & I2C_CR2_ITEVTEN) != 0,  // Event interrupt enable
               (i2c->SR1 & I2C_SR1_ADDR) != 0,     // Address matched
               (i2c->SR1 & I2C_SR1_STOPF) != 0,    // Stop condition detected
               (i2c->SR1 & I2C_SR1_RXNE) != 0,     // Receive buffer not empty
               (i2c->SR1 & I2C_SR1_TXE) != 0       // Transmit buffer empty
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
      snprintf(buffer, sizeof(buffer), "BERR=%d, ARLO=%d, AF=%d, OVR=%d",
               (i2c->SR1 & I2C_SR1_BERR) != 0,  // Bus error
               (i2c->SR1 & I2C_SR1_ARLO) != 0,  // Arbitration lost
               (i2c->SR1 & I2C_SR1_AF) != 0,    // Acknowledge failure
               (i2c->SR1 & I2C_SR1_OVR) != 0    // Overrun/Underrun
      );
      break;
    }

    case SDIO_IRQn:
    {
      snprintf(
          buffer, sizeof(buffer),
          "CMDRENDIE=%d, CMDSENTIE=%d, DATAENDIE=%d, RXOVERRIE=%d, "
          "TXUNDERRIE=%d, STBITERRIE=%d",
          (SDIO->MASK & SDIO_MASK_CMDRENDIE) != 0,  // Command response received
          (SDIO->MASK & SDIO_MASK_CMDSENTIE) != 0,  // Command sent
          (SDIO->MASK & SDIO_MASK_DATAENDIE) != 0,  // Data transfer end
          (SDIO->MASK & SDIO_MASK_RXOVERRIE) != 0,  // Receive FIFO overrun
          (SDIO->MASK & SDIO_MASK_TXUNDERRIE) != 0,  // Transmit FIFO underrun
          (SDIO->MASK & SDIO_MASK_STBITERRIE) != 0   // Start bit error
      );
      break;
    }
    case TAMP_STAMP_IRQn:
    {
      snprintf(
          buffer, sizeof(buffer),
          "TAMP1IE=%d, TAMP2IE=%d, TIMESTAMPIE=%d, TAMP1F=%d, TAMP2F=%d, "
          "TSF=%d",
          (RTC->TAFCR & RTC_TAFCR_TAMPIE) !=
              0,  // General Tamper interrupt enable
          (RTC->TAFCR & RTC_TAFCR_TAMP2E) != 0,  // Tamper 2 interrupt enable
          (RTC->CR & RTC_CR_TSIE) != 0,          // Timestamp interrupt enable
          (RTC->ISR & RTC_ISR_TAMP1F) != 0,      // Tamper 1 flag
          (RTC->ISR & RTC_ISR_TAMP2F) != 0,      // Tamper 2 flag
          (RTC->ISR & RTC_ISR_TSF) != 0          // Timestamp flag
      );
      break;
    }

    case RCC_IRQn:
    {
      snprintf(
          buffer, sizeof(buffer),
          "CSSIE=%d, PLLRDYIE=%d, HSE_RDYIE=%d, HSI_RDYIE=%d, LSE_RDYIE=%d, "
          "LSI_RDYIE=%d",
          (RCC->CIR & RCC_CIR_CSSC) !=
              0,  // Clock Security System interrupt enable
          (RCC->CIR & RCC_CIR_PLLRDYIE) != 0,  // PLL Ready interrupt enable
          (RCC->CIR & RCC_CIR_HSERDYIE) != 0,  // HSE Ready interrupt enable
          (RCC->CIR & RCC_CIR_HSIRDYIE) != 0,  // HSI Ready interrupt enable
          (RCC->CIR & RCC_CIR_LSERDYIE) != 0,  // LSE Ready interrupt enable
          (RCC->CIR & RCC_CIR_LSIRDYIE) != 0   // LSI Ready interrupt enable
      );
      break;
    }
    case RTC_WKUP_IRQn:
    {
      snprintf(buffer, sizeof(buffer), "WKUPIE=%d, WUTF=%d",
               (RTC->CR & RTC_CR_WUTIE) != 0,  // Wakeup Timer interrupt enable
               (RTC->ISR & RTC_ISR_WUTF) != 0  // Wakeup Timer flag
      );
      break;
    }
    case ADC_IRQn:
    {
      snprintf(
          buffer, sizeof(buffer), "EOCIE=%d, JEOCIE=%d, AWDIE=%d, OVRIE=%d",
          (ADC1->CR1 & ADC_CR1_EOCIE) !=
              0,  // End of Conversion interrupt enable
          (ADC1->CR1 & ADC_CR1_JEOCIE) !=
              0,  // Injected End of Conversion interrupt enable
          (ADC1->CR1 & ADC_CR1_AWDIE) != 0,  // Analog Watchdog interrupt enable
          (ADC1->CR1 & ADC_CR1_OVRIE) != 0   // Overrun interrupt enable
      );
      break;
    }
    case RTC_Alarm_IRQn:
    {
      snprintf(buffer, sizeof(buffer),
               "ALRAIE=%d, ALRBIE=%d, ALRAF=%d, ALRBF=%d",
               (RTC->CR & RTC_CR_ALRAIE) != 0,   // Alarm A interrupt enable
               (RTC->CR & RTC_CR_ALRBIE) != 0,   // Alarm B interrupt enable
               (RTC->ISR & RTC_ISR_ALRAF) != 0,  // Alarm A flag
               (RTC->ISR & RTC_ISR_ALRBF) != 0   // Alarm B flag
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
