#include <stdio.h>
#include <string.h>

#include "cli_key_code.h"
#include "console_utile.h"
#include "dev_io.h"
#include "driver_uart.h"
#include "cli_input.h"
#include "utile.h"
#include "pcb_define.h"

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
    debug_printf("포트 매핑 불일치: SYSCFG = %u, 기대값 = %u\r\n", exti_port_val, gpio_port_index);
    return false;
  }

  if (!(EXTI->IMR & (1 << pin_num)))
  {
    debug_printf("EXTI 인터럽트 마스크됨 (IMR[%d] = 0)\r\n", pin_num);
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
    debug_printf("NVIC ISER[%d] 비활성화됨\r\n", irq);
    return false;
  }

  uint32_t priority = NVIC_GetPriority(irq);
  uint32_t preempt_priority = (priority >> (8 - __NVIC_PRIO_BITS)) & 0xF;
  uint32_t sub_priority = (priority >> 0) & ((1 << (8 - __NVIC_PRIO_BITS)) - 1);

  bool rising = (EXTI->RTSR & (1 << pin_num)) != 0;
  bool falling = (EXTI->FTSR & (1 << pin_num)) != 0;

  debug_printf("GPIO 인터럽트 설정됨: 포트=GPIO%c, 핀=%d\r\n", 'A' + gpio_port_index, pin_num);
  debug_printf("  IRQn = %d\r\n", irq);
  debug_printf("  NVIC PreemptPriority = %lu\r\n", preempt_priority);
  debug_printf("  NVIC SubPriority = %lu\r\n", sub_priority);
  debug_printf("  Edge Trigger: %s%s\r\n", rising ? "RISING " : "", falling ? "FALLING" : "");

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

#if 0
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
    EXTI->SWIER |= (1 << pin_num);
}




void test_uart(void)
{
  uart_config_t uart_config;
  driver_t *port[UART_PORT_MAX];
  char buff[30];
  char rx_buff[10];
   char *rs232_port_name[UART_PORT_MAX] = {"VHF", "TTL", "A", "B", "C", "D","CDMA"};
  int baud;
  int len;
  int rs232_number=-1;

  debug_printf("RS232 CDMA,TTL,A,B,C,D,CDMA 테스트\r\n");
  debug_printf("주의:RS232 A,B는 하드웨어점퍼 설정 필요\r\n");

    debug_printf("포트 이름을 입력해주세요\r\n");
  if (cli_scanf_s("%7s", buff) == CLI_KEYCODE_CTRL_C)
  {
    return ;
  }

  debug_printf("기능:1초마다 각 포트이름 전송되며 1초 대기,입력 에코처리함\r\n");
  debug_printf("통신 속도를 입력해주세요\r\n");

  if (get_int_input("통신 속도를 입력해주세요", &baud, 1200, 115200) != MENU_OK)
  {
    baud = 57600;
    debug_printf("기본 속도로 설정합니다.%d\r\n", baud);
  }
  debug_printf("이제 테스트 진행하세요 CTRL+Q 종료\r\n");


  uart_config.baud = baud;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  port[0] = driver_uart_open(UART_0_D_SUB_0, &uart_config);
  port[1] = driver_uart_open(UART_1_TTL, &uart_config);
  port[2] = driver_uart_open(UART_2_EXT_A, &uart_config);
  port[3] = driver_uart_open(UART_3_EXT_B, &uart_config);
  port[4] = driver_uart_open(UART_4_EXT_C, &uart_config);
  port[5] = driver_uart_open(UART_5_EXT_D, &uart_config);
  port[6] = driver_uart_open(UART_8_CDMA, &uart_config);

  for (int n = 0; n < _countof(rs232_port_name);n++)
  {
    if(strcmp(buff, rs232_port_name[n])==0)
    {
      rs232_number = n;
      break;
    }
  }
  if (rs232_number ==-1)
  {
    debug_printf("포트 이름을 확인해주세요\r\n");
    return ;
  }



  is_gpio_interrupt_enabled(IN_EX_UART_INT_7_GPIO_Port, IN_EX_UART_INT_7_Pin);
  is_gpio_interrupt_enabled(IN_EX_UART_INT_8_GPIO_Port, IN_EX_UART_INT_8_Pin);

  trigger_gpio_interrupt(IN_EX_UART_INT_7_GPIO_Port,IN_EX_UART_INT_7_Pin);
  
  
  while (1)
  {
    snprintf(buff, sizeof(buff), "RS232 %s\r\n", rs232_port_name[rs232_number]);
    len = driver_uart_send(port[rs232_number], buff, strlen(buff));

    if (len < 0)
    {
      snprintf(buff, sizeof(buff), "RS232 %s error\r\n", rs232_port_name[rs232_number]);
      debug_printf(buff);
    }

    len = driver_uart_recv(port[rs232_number], rx_buff, sizeof(rx_buff), 1000);
    if (len)
    {
      driver_uart_send(port[rs232_number], rx_buff, len);
    }
    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      return;
    }


  }
}