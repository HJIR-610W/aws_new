#include <stdio.h>
#include <string.h>

#include "cli_key_code.h"
#include "console_utile.h"
#include "debug_io.h"
#include "bsp_uart.h"
#include "cli_input.h"
#include "util_memory.h"
#include "pcb_define.h"
#include "drv_do.h"
#define UART_PORT_MAX 7

#include <stdbool.h>

#include "stm32f4xx_hal.h"

uint32_t count_trailing_zeros(uint32_t x)
{
    if (x == 0)
        return 32; // 정의되지 않은 동작에 대한 보호

    uint32_t pos = 0;
    while ((x & 1) == 0)
    {
        x >>= 1;
        pos++;
    }
    return pos;
}


bool is_gpio_interrupt_enabled(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  uint32_t pin_num = count_trailing_zeros(GPIO_Pin);  // ex: GPIO_PIN_10 -> 10
  uint32_t exti_port_val;
  uint32_t exticr_index = pin_num / 4;
  uint32_t exticr_shift = (pin_num % 4) * 4;

  // 1. SYSCFG에서 포트 확인
  exti_port_val = (SYSCFG->EXTICR[exticr_index] >> exticr_shift) & 0xF;

  uint32_t gpio_port_index = 0xFF;
  if (GPIOx == GPIOA)
    gpio_port_index = 0;
  else if (GPIOx == GPIOB)
    gpio_port_index = 1;
  else if (GPIOx == GPIOC)
    gpio_port_index = 2;
  else if (GPIOx == GPIOD)
    gpio_port_index = 3;
  else if (GPIOx == GPIOE)
    gpio_port_index = 4;
  else if (GPIOx == GPIOF)
    gpio_port_index = 5;
  else if (GPIOx == GPIOG)
    gpio_port_index = 6;
  else if (GPIOx == GPIOH)
    gpio_port_index = 7;
  else if (GPIOx == GPIOI)
    gpio_port_index = 8;

  if (exti_port_val != gpio_port_index)
  {
    dbg_printf("포트 매핑 불일치: SYSCFG = %u, 기대값 = %u\r\n", exti_port_val, gpio_port_index);
    return false;
  }

  if (!(EXTI->IMR & (1 << pin_num)))
  {
    dbg_printf("EXTI 인터럽트 마스크됨 (IMR[%d] = 0)\r\n", pin_num);
    return false;
  }

  IRQn_Type irq;
  if (pin_num <= 4)
    irq = (IRQn_Type)(EXTI0_IRQn + pin_num);
  else if (pin_num <= 9)
    irq = EXTI9_5_IRQn;
  else
    irq = EXTI15_10_IRQn;

  if (!(NVIC->ISER[irq / 32] & (1 << (irq % 32))))
  {
    dbg_printf("NVIC ISER[%d] 비활성화됨\r\n", irq);
    return false;
  }

  uint32_t priority = NVIC_GetPriority(irq);
  uint32_t preempt_priority = (priority >> (8 - __NVIC_PRIO_BITS)) & 0xF;
  uint32_t sub_priority = (priority >> 0) & ((1 << (8 - __NVIC_PRIO_BITS)) - 1);

  bool rising = (EXTI->RTSR & (1 << pin_num)) != 0;
  bool falling = (EXTI->FTSR & (1 << pin_num)) != 0;

  dbg_printf("GPIO 인터럽트 설정됨: 포트=GPIO%c, 핀=%d\r\n", 'A' + gpio_port_index, pin_num);
  dbg_printf("  IRQn = %d\r\n", irq);
  dbg_printf("  NVIC PreemptPriority = %lu\r\n", preempt_priority);
  dbg_printf("  NVIC SubPriority = %lu\r\n", sub_priority);
  dbg_printf("  Edge Trigger: %s%s\r\n", rising ? "RISING " : "", falling ? "FALLING" : "");

  return true;
}

static uint32_t GPIO_PortToIndex(GPIO_TypeDef *GPIOx)
{
    if (GPIOx == GPIOA) return 0;
    if (GPIOx == GPIOB) return 1;
    if (GPIOx == GPIOC) return 2;
    if (GPIOx == GPIOD) return 3;
    if (GPIOx == GPIOE) return 4;
    if (GPIOx == GPIOF) return 5;
    if (GPIOx == GPIOG) return 6;
    if (GPIOx == GPIOH) return 7;
    if (GPIOx == GPIOI) return 8;
    return 0xFFFFFFFF;  // 오류
}

void trigger_gpio_interrupt(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    uint32_t pin_num = 0;
    uint32_t port_index = GPIO_PortToIndex(GPIOx);
    if (port_index == 0xFFFFFFFF)
        return;

    pin_num = count_trailing_zeros(GPIO_Pin);  // GPIO_PIN_10 → 10 (IAR이면 대체 함수 사용)

#if 1
    // 1. SYSCFG EXTICR 설정
    uint32_t exticr_index = pin_num / 4;
    uint32_t exticr_shift = (pin_num % 4) * 4;

    SYSCFG->EXTICR[exticr_index] &= ~(0xF << exticr_shift);
    SYSCFG->EXTICR[exticr_index] |=  (port_index << exticr_shift);

    // 2. EXTI IMR 활성화
    EXTI->IMR |= (1 << pin_num);

    // 3. NVIC IRQ 설정
    IRQn_Type irq;
    if (pin_num <= 4)
        irq = (IRQn_Type)(EXTI0_IRQn + pin_num);
    else if (pin_num <= 9)
        irq = EXTI9_5_IRQn;
    else
        irq = EXTI15_10_IRQn;

    HAL_NVIC_SetPriority(irq, 0, 0);
    HAL_NVIC_EnableIRQ(irq);
#endif
    // 4. 소프트웨어 인터럽트 발생
//    EXTI->SWIER |= (1 << pin_num);
}


uint8_t g_uart_ll=0;

void test_uart(void)
{
  uart_config_t uart_config;
  int len;
  int32_t selected_port;
  int port_index;
  char buff[100];
  char rx_buff[50];
  int baud = 57600;
  const char *rs232_port_name[UART_PORT_MAX] = {"VHF", "TTL", "A", "B", "C", "D", "CDMA"};
  const int32_t rs232_drv_num[UART_PORT_MAX] = {
    BSP_UART_0_D_SUB_0,
    BSP_UART_1_TTL_ONLY,
    BSP_UART_2_EXT_A,
    BSP_UART_3_EXT_B,
    BSP_UART_4_EXT_C,
    BSP_UART_5_EXT_D,
    BSP_UART_8_CDMA
  };

  dbg_printf("RS232 포트별 테스트\r\n");
  dbg_printf("주의: RS232 A,B는 하드웨어 점퍼 설정 필요\r\n");
  dbg_printf("기능: 포트이름 전송 후 1초간 수신 데이터 에코 및 HEX 출력\r\n\r\n");

  if(input_decimal_prompt("테스트할 포트를 선택하세요 (0:VHF, 1:TTL, 2:A, 3:B, 4:C, 5:D, 6:CDMA)", &port_index, 0, 6) != MENU_OK)
  {
    port_index = 0;
    dbg_printf("기본 포트 VHF로 설정합니다.\r\n");
  }

  if(input_decimal_prompt("통신 속도를 입력해주세요", &baud, 1200, 115200) != MENU_OK)
  {
    baud = 57600;
    dbg_printf("기본 속도 %d로 설정합니다.\r\n", baud);
  }

  selected_port = rs232_drv_num[port_index];
  
  uart_config.baud = baud;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  bsp_uart_init(selected_port, &uart_config);
  
  dbg_printf("RS232 포트 %s, 속도 %d로 테스트 시작 (CTRL+Q 종료)\r\n", 
            rs232_port_name[port_index], baud);

  while (1)
  {
    snprintf(buff, sizeof(buff), "RS232 %s\r\n", rs232_port_name[port_index]);
    len = bsp_uart_send(selected_port, (uint8_t*)buff, strlen(buff));
    
    if(len < 0)
    {
      dbg_printf("RS232 %s 전송 실패\r\n", rs232_port_name[port_index]);
    }
    else
    {
      dbg_printf("전송: %s", buff);
    }

    len = bsp_uart_recv(selected_port, (uint8_t*)rx_buff, sizeof(rx_buff), 1000);
    
    if(len > 0)
    {
      bsp_uart_send(selected_port, (uint8_t*)rx_buff, len);
      
      dbg_printf("수신 및 에코 (%d bytes): ", len);
      for(int i = 0; i < len; i++)
      {
        dbg_printf("%c", rx_buff[i]);
      }
      dbg_printf("\r\n");
    }

    if (get_key(10) == KEY_CODE_CTRL_C)
    {
      dbg_printf("테스트 종료\r\n");
      return;
    }

    if (g_uart_ll)
    {
      g_uart_ll = 0;
      len = bsp_uart_recv_ll(selected_port, (uint8_t*)rx_buff, sizeof(rx_buff), 1000);
    }
  }
}