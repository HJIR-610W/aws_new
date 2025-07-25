
#include "test_sdi12.h"

#include "cli_key_code.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "dev_io.h"
#include "drv_di.h"
#include "bsp_do.h"
#include "bsp_uart.h"
#include "pcb_define.h"
#include "drv_power.h"

extern UART_HandleTypeDef huart6;

#define SDI_TXD_HIGH() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET)
#define SDI_TXD_LOW() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET)

#define SDI_DIR_TX_OFF() bsp_do_low(BSP_DO_DIR_SDI)
#define SDI_DIR_TX_ON() bsp_do_high(BSP_DO_DIR_SDI)

#define SDI_SEND(data, len) drv_uart_send(g_sdi_uart, data, len)
#define SDI_RECV(buff, buffSize, timeout) drv_uart_recv(g_sdi_uart, buff, buffSize, timeout)

#define HART_POWER_ON() drv_power_on(DRV_POWER_HART_24V)
#define HART_POWER_OFF() drv_power_off(DRV_POWER_HART_24V)

#define SDI_RX_INT_DISABLE() __HAL_UART_DISABLE_IT(&huart6, UART_IT_RXNE)
#define SDI_RX_INT_ENABLE() __HAL_UART_ENABLE_IT(&huart6, UART_IT_RXNE)

#define SDI_UART_DISABLE() __HAL_UART_DISABLE(&huart6)
#define SDI_UART_ENABLE() __HAL_UART_ENABLE(&huart6)

int32_t g_sdi_uart;


const osThreadAttr_t sdiTask_attributes = {
    .name = "sdiTask",
    .stack_size = TASK_STACK(TASK_SDI_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_SDI_DEF),
};

uint8_t set_even_parity(uint8_t data)
{
  uint8_t parity = 0;

  // 0~6비트까지 XOR 연산하여 패리티 계산 (짝수 패리티)
  for (int i = 0; i < 7; i++)
  {
    parity ^= (data >> i) & 1;
  }

  // 패리티 비트를 7번째 비트에 설정
  data &= 0x7F;           // 상위 비트(7번째 비트) 초기화
  data |= (parity << 7);  // 패리티 비트를 7번째 비트에 설정

  return data;
}



void sdi_uart_tx_reset(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void sdi_uart_tx_set(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void send_uart6_break()
{
  USART6->CR1 |= USART_CR1_SBK;         // Break 신호 시작
  while (USART6->CR1 & USART_CR1_SBK);  // Break 신호가 끝날 때까지 대기
}

volatile uint32_t g_int_num = UART_IT_RXNE;
void sdi_send(uint8_t *cmd, uint16_t dataLen)
{
  // TX 라인은 송신 모드에서는 idle 일때 High 이고 수신 모드로 전환되면 low가 출력됨

  // SDI는 7bit data 1bit parity 라서 데이터에 parity를 넣어줘야 한다.
  for (int i = 0; i < dataLen; i++)
  {
    cmd[i] = set_even_parity(cmd[i]);
  }
#if 0 
  SDI_UART_DISABLE();  //uart를 비활성 화 시킨다.
  sdi_uart_tx_reset(); //uart tx gpio로 변경
  SDI_RX_INT_DISABLE();    //rx인터럽브 비활성

  SDI_DIR_TX_ON();   // 드라이버 ic를 송신  모드로 설정 
  SDI_TXD_HIGH();    // SDI 라인을 High로 만든다.
  osDelay(12);
  SDI_TXD_LOW();     // SDI 라인을 Low로 만든다.
  osDelay(9);
  sdi_uart_tx_set(); // uart tx gpio를 uart 사용으로 재설정
  SDI_UART_ENABLE();  //uart를 활성화 시킨다.
  SDI_SEND(cmd,dataLen); //데이터 전송
  osDelay(1);
  SDI_DIR_TX_OFF(); //드라이버 ic를 수신 모드로 설정
  SDI_RX_INT_ENABLE();  //uart 수신 인터럽트 허용
#else
  (void)g_int_num;

  SDI_DIR_TX_ON();     // 드라이버 ic를 송신  모드로 설정
  send_uart6_break();  // 8.333ms
  send_uart6_break();  // 8.333ms SDI12  규격에 센서를 깨우기 위해 TX break 신호 12ms low
  osDelay(10);         // marking 즉 1이 8.3ms 동안 유지되어야함
  SDI_SEND(cmd, dataLen);  // 데이터 전송
  osDelay(1);
  SDI_DIR_TX_OFF();  // 드라이버 ic를 수신 모드로 설정 15ms안에 응답해야한다고함


#endif
}

void sdiTask(void *arg)
{
  uint8_t buff[50];
  uint8_t cmd[] = {"0XR3!"};  // 레코더  ?I
  // uint8_t cmd[]={0x00,0xff,0x00};
  int32_t len;

  SDI_DIR_TX_OFF();

  io_printf("0XR3 이런 문자열이 출력되면 정상\r\n");
  io_printf("CTRL+Q 종료료\r\n");

  while (1)
  {
    sdi_send(cmd, sizeof(cmd) - 1);
    len = SDI_RECV(buff, sizeof(buff), 3000);
    if (len)
    {
      for(int i = 0; i< len; i++)
      {
        buff[i]&=0x7F;//even 페리티 제거 
      }
      LOG_MEM(buff,len,0,16);
    }
    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      break;
    }
  }
}

void test_sdi12(void)
{
  uart_config_t uart_config = {.dataLen = UART_DATA_LEN_8, .stop_bit = 0};


  uart_config.baud = 1200;
  uart_config.parityIdx = PARITY_NONE;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;

  g_sdi_uart = BSP_UART_9_SDI_ONLY;

      bsp_uart_init(BSP_UART_9_SDI_ONLY, &uart_config);

  sdiTask(0);
}