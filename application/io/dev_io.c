#define __STDC_WANT_LIB_EXT1__ 1
#include "dev_io.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "drv_rs232.h"
#include "drv_rs485.h"
#include "drv_rs485.h"
#include "drv_rs232.h"
#include "pcb_define.h"
#include "stm32f4xx_hal.h"
#include "system_err.h"
#include "FreeRTOS.h"  // pvPortMalloc, vPortFree 사용 시 필요
#include "terminal.h"
#include "tlsf.h"
#include "user_heap.h"
#include "util_time.h"
#include "cli_input.h"
#include "task_telnet_server.h"
#include "terminal_bridge.h"

static int32_t debug_uart_num = -1;

USART_TypeDef *debug_uart_base = USART1;

void debug_uart_init(uint32_t baud_rate)
{
  uint32_t pclk;

  if (debug_uart_base == USART1)
  {
    pclk = HAL_RCC_GetPCLK2Freq();
  }
  else
  {
    pclk = HAL_RCC_GetPCLK1Freq();
  }

  // 1. UART3 및 GPIO 클럭 활성화
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;  // UART3 클럭 활성화
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;   // GPIOB 클럭 활성화

  RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;   // USART3 리셋 활성화
  RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;  // USART3 리셋 비활성화

  // 2. GPIO 핀 설정 (PB10: TX, PB11: RX)
  GPIOB->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);     // 초기화
  GPIOB->MODER |= (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);  // AF 모드 설정
  GPIOB->AFR[1] &= ~((0xF << (2 * 4)) | (0xF << (3 * 4)));        // AFR[1] 클리어 (핀 10, 11)
  GPIOB->AFR[1] |= (7 << (2 * 4)) | (7 << (3 * 4));               // AF7 (USART3)

  // 3. UART 설정
  debug_uart_base->CR1 &= ~USART_CR1_UE;  // UART 비활성화

  // BRR 레지스터 설정
  debug_uart_base->BRR = UART_BRR_SAMPLING16(pclk, baud_rate);

  // (2) 데이터 비트, 패리티, 정지 비트 설정
  debug_uart_base->CR1 &= ~USART_CR1_M;     // 8 데이터 비트
  debug_uart_base->CR2 &= ~USART_CR2_STOP;  // 1 정지 비트
  debug_uart_base->CR1 &= ~USART_CR1_PCE;   // 패리티 비활성화

  // (3) 송신(TX) 및 수신(RX) 활성화
  debug_uart_base->CR1 |= USART_CR1_TE;  // 송신 활성화

  // (4) UART 활성화
  debug_uart_base->CR1 |= USART_CR1_UE;  // UART 활성화

  // (5) 송신 준비 확인
  while (!(debug_uart_base->SR & USART_SR_TC));  // 송신 완료 플래그 확인
}

void set_debug_uart_handle(int32_t drv) { debug_uart_num = drv; }

int32_t get_debug_uart_handle(void) { return debug_uart_num; }

/**
 * @brief os구동 없을때 사용
 */
void debug_puts_nonos(char *str)
{
  while (*str)
  {
    while (!(debug_uart_base->SR & USART_SR_TXE));  // 송신 버퍼가 비어있는지 확인
    debug_uart_base->DR = (uint8_t)*str++;          // 데이터 레지스터에 문자 송신
  }
}

void io_send(uint8_t *p_in_data, uint16_t data_len)
{
  drv_uart_send(debug_uart_num, p_in_data, data_len);
  terminal_bridge_send_output((char *)p_in_data, data_len);
}

void io_put_ch(char ch)
{
  drv_uart_send(debug_uart_num, (uint8_t *)&ch, 1);
  terminal_bridge_send_output((char *)&ch, 1);
}

void io_puts(const char *str)
{
  int32_t len = strlen(str);

  drv_uart_send(debug_uart_num, (uint8_t *)str, len);
  terminal_bridge_send_output((char *)str, len);
}


#define PRINTF_HEAP_USE 1

int32_t io_printf(const char *pFmt, ...)
{
  char buff[2];
  char *ptr = NULL;
  char *temp = NULL;
  va_list ap;
  int32_t len=0;

  (void)len;
  (void)temp;
#if PRINTF_HEAP_USE == 0
  char printf_buff[256];
#endif

  // 먼저 필요한 길이 측정
  va_start(ap, pFmt);
  len = vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
  va_end(ap);

#if PRINTF_HEAP_USE
  // 동적 메모리 할당 모드
  if (len > (sizeof(buff) - 1))
  {
    temp = user_malloc(len + 1);  // null 포함
    if (temp)
    {
      va_start(ap, pFmt);
      vsnprintf_s((char *)temp, len + 1, (char *)pFmt, ap);
      va_end(ap);
      ptr = temp;
    }
    else
    {
      return 1;  // 메모리 할당 에러
    }
  }
  else
  {
    // 매우 짧은 메시지 (1바이트)는 buff에 다시 포맷팅
    va_start(ap, pFmt);
    vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
    va_end(ap);
    ptr = buff;
  }
#else
  // 고정 버퍼 모드
  va_start(ap, pFmt);
  vsnprintf_s((char *)printf_buff, sizeof(printf_buff), (char *)pFmt, ap);
  va_end(ap);
  ptr = printf_buff;
#endif

  if (debug_uart_num != -1 && ptr)
  {
    io_send((uint8_t *)ptr, strlen(ptr));
  }

#if PRINTF_HEAP_USE
  if (temp)
  {
    user_free(temp);
  }
#endif

  return 0;
}

int32_t io_vprintf(const char *pFmt, va_list ap)
{
  char buff[2];
  char *ptr = NULL;
  char *temp = NULL;
  int32_t len=0;
  int32_t total_len=0;
  va_list ap_copy;

  (void)len;
  (void)temp;
  (void)total_len;
#if PRINTF_HEAP_USE == 0
  char printf_buff[256];
#endif

  // 먼저 필요한 길이 측정
  va_copy(ap_copy, ap);
  len = vsnprintf_s(buff, sizeof(buff), pFmt, ap_copy);
  va_end(ap_copy);

#if PRINTF_HEAP_USE
  // 동적 메모리 할당 모드: 측정된 길이만큼 할당
  if (len > (sizeof(buff) - 1))
  {
    total_len = len + 1;  // null 문자 포함
    temp = pvPortMalloc(total_len);
    if (temp)
    {
      va_copy(ap_copy, ap);
      vsnprintf_s(temp, total_len, pFmt, ap_copy);
      va_end(ap_copy);
      ptr = temp;
    }
    else
    {
      return 1;  // 메모리 할당 실패
    }
  }
  else
  {
    // 매우 짧은 메시지 (1바이트)는 buff 사용
    va_copy(ap_copy, ap);
    vsnprintf_s(buff, sizeof(buff), pFmt, ap_copy);
    va_end(ap_copy);
    ptr = buff;
  }
#else
  // 고정 버퍼 모드: 256바이트 버퍼 사용
  vsnprintf_s(printf_buff, sizeof(printf_buff), pFmt, ap);
  ptr = printf_buff;
#endif

  if (debug_uart_num != -1 && ptr)
  {
    drv_uart_send(debug_uart_num, (uint8_t *)ptr, strlen(ptr));
  }

#if PRINTF_HEAP_USE
  if (temp)
  {
    vPortFree(temp);
  }
#endif

  return 0;
}









int32_t io_recv(char *p_out_buffer, uint16_t out_size, uint32_t timeout)
{
  int32_t cnt;

  cnt = drv_uart_recv(debug_uart_num, (uint8_t *)p_out_buffer, out_size, timeout);

  return cnt;
}

void LOG_MEM(uint8_t *src, uint32_t size, uint32_t startAddr, uint32_t col)
{
  int32_t i, j, row;
  uint8_t ch;
  char temp[100];

  int32_t len;

  if ((size % col) == 0)
    row = (size / col);
  else
    row = (size / col) + 1;

  io_printf("\n\r\n\r                ");
  len = 0;
  temp[0] = 0;

  for (j = 0; j < col; j++)
  {
    len = strnlen_s(temp, sizeof(temp));
    snprintf_s(&temp[len], sizeof(temp) - len, "%02X ", j);
  }
  io_printf(temp);

  io_printf("  ");
  for (j = 0; j < col; j++)
  {
    io_printf("%X", j % 16);
  }

  for (i = 0; i < row; i++)
  {
    snprintf_s(temp, sizeof(temp), "\n\r%04d  %08X  ", (int32_t)(i * col), (startAddr + i * col));
    io_printf(temp);

    temp[0] = 0;

    for (j = 0; j < col; j++)
    {
      if (((i * col) + j) < size)
      {
        len = strnlen_s(temp, sizeof(temp));
        snprintf_s(&temp[len], sizeof(temp) - len, "%02X ", src[i * col + j]);
      }
      else
      {
        len = strnlen_s(temp, sizeof(temp));
        snprintf_s(&temp[len], sizeof(temp) - len, "   ");
      }
    }
    io_printf(temp);
    io_printf("  ");

    temp[0] = 0;

    for (j = 0; j < col; j++)
    {
      if (((i * col) + j) < size)
      {
        ch = src[i * col + j];
        if ((ch >= 0x20) && (ch < 0x7F) && ch != '%')
        {
          len = strnlen_s(temp, sizeof(temp));
          snprintf_s(&temp[len], sizeof(temp) - len, "%c", ch);
        }
        else
        {
          len = strnlen_s(temp, sizeof(temp));
          snprintf_s(&temp[len], sizeof(temp) - len, ".");
        }
      }
    }
    io_printf(temp);
  }
  io_printf("\n\r");
}

void dev_io_get(dev_io_t *dev, uint8_t cmd, void *opt)
{
  switch (cmd)
  {
    case DEV_IO_GET_CMD_CFG:
      switch (dev->io)
      {
        case eRS485_IO:

          break;
        case eRS232_IO:

          break;
      }
      break;
  }
}

void dev_io_write(dev_io_t *dev, uint8_t *data, uint32_t dataLen, uint32_t opt)
{
  switch (dev->io)
  {
    case eRS485_IO:
      drv_rs485_send(dev->num, data, dataLen);
      break;
    case eRS232_IO:
      drv_uart_send(dev->num, data, dataLen);
      break;
  }
}

void dev_io_flush(dev_io_t *dev)
{
  switch (dev->io)
  {
    case eRS485_IO:
      drv_rs485_flush_rx(dev->num);
      break;
    case eRS232_IO:
      drv_uart_flush_rx(dev->num);
      break;
  }
}

uint16_t dev_io_read(dev_io_t *dev, uint8_t *out, uint32_t dataLen, uint8_t cmd, void *opt)
{
  devIoTimeOutopt_t *pdevopt = opt;
  uint32_t data_timeout;
  data_timeout = pdevopt->waitTimeOutMs / 2;

  switch (dev->io)
  {
    case eRS485_IO:
       return drv_rs485_recv_opt(dev->num, out, dataLen, pdevopt->waitTimeOutMs,data_timeout);
      break;
    case eRS232_IO:

      return drv_uart_recv_opt(dev->num, out, dataLen, pdevopt->waitTimeOutMs, data_timeout);
      break;
  }
  return 0;
}



/**
 * @brief Task에서 디버깅용으로 출력 하고 싶을때
 *         콘솔 메뉴에서 task id를 설정해주면 id가 일치하는 task는 
 *         printf 
 */

static void *g_task_id;
static bool foreced_print = false;
void set_task_id(void *task_id)
{
  g_task_id = task_id;
}

void set_forced_print(bool set) { foreced_print = set; }


void task_printf(const char *pFmt, ...)
{
  void *task_id;

  task_id = osThreadGetId();

  if (task_id == NULL && foreced_print==false)
  {
    return;
  }

  if (g_task_id == NULL && foreced_print==false)
  {
    return;
  }

  if (task_id == g_task_id || (foreced_print))
  {
    va_list args;
    va_start(args, pFmt);
    io_vprintf(pFmt, args); 
    va_end(args);
  }
}


void task_hex_dump(const char *title, const uint8_t *data, uint32_t length)
{
  if (title || foreced_print)
    task_printf("%s (len=%d):\r\n", title, (int)length);

  for (uint32_t i = 0; i < length; i++)
  {
    if (i % 16 == 0)
      task_printf("%04X: ", (unsigned int)i);  // 주소/인덱스 출력

    task_printf("%02X ", data[i]);

    if ((i + 1) % 16 == 0 || i + 1 == length)
      task_printf("\r\n");
  }
}

void io_printf_color(int color, const char *pFmt, ...)
{
  io_printf("%c[%dm", 27, color);

  va_list args;
  va_start(args, pFmt);
  io_vprintf(pFmt, args);
  va_end(args);

  io_printf("%c[%dm", 27, 37);
}

int io_scanf_s(const char *fmt, ...)
{
  va_list args;
  int ret;

  va_start(args, fmt);
  ret = cli_vscanf_s(fmt, args); 
  va_end(args);

  return ret;
}



int32_t io_inject(uint8_t *p_data,uint32_t data_len)
{
  return drv_uart_inject(debug_uart_num, p_data, data_len);
}