#define __STDC_WANT_LIB_EXT1__ 1
#include "dev_io.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "Lib\tlsf\tlsf.h"
#include "app_rs232.h"
#include "app_rs485.h"
#include "driver_485.h"
#include "driver_uart.h"
#include "pcb_define.h"
#include "stm32f4xx_hal.h"
#include "system_err.h"
#include "terminal.h"
#include "user_heap.h"

static driver_t *debug_uart = NULL;
;
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

void set_debug_uart_handle(driver_t *drv) { debug_uart = drv; }

driver_t *get_debug_uart_handle(void) { return debug_uart; }

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

static char g_printf_buff[512];

#define PRINTF_HEAP_USE 0


int32_t debug_printf(const char *pFmt, ...)
{
  char buff[2];
  char *ptr = NULL;
  char *temp = NULL;
  va_list ap;
  int32_t len;

  // 먼저 format 후 len의 길이를 확인 후 메모리를 할당후 최종 처리
  va_start(ap, pFmt);
  len = vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
  va_end(ap);

#if PRINTF_HEAP_USE
  if (len > (sizeof(buff) - 1))  //
  {
    temp = aws_malloc(len + 1);  // null포함
    if (temp)
    {
      va_start(ap, pFmt);
      len = vsnprintf_s((char *)temp, len + 1, (char *)pFmt, ap);
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
    ptr = buff;  // 1바이트만 전송하게 되면 버퍼로 처리
  }
#else
  va_start(ap, pFmt);
  len = vsnprintf_s((char *)g_printf_buff, sizeof(g_printf_buff), (char *)pFmt, ap);
  va_end(ap);

  ptr = g_printf_buff;
#endif
  if (debug_uart && ptr)  // os구동중인지 확인
  {
    driver_uart_send(debug_uart, (uint8_t *)ptr, strlen(ptr));
  }
  else if (ptr)  // os 없으면
  {
    debug_puts_nonos(ptr);
  }

#ifdef PRINTF_HEAP_USE
  if (temp)
  {
    aws_free(temp);
  }
#endif
  return 0;
}


int32_t error_printf(const char *pFmt, ...)
{
  char buff[2];
  char *ptr = NULL;
  char *temp = NULL;
  va_list ap;
  int32_t len;

  // 먼저 format 후 len의 길이를 확인 후 메모리를 할당후 최종 처리
  va_start(ap, pFmt);
  len = vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
  va_end(ap);

  if (len > (sizeof(buff) - 1))  //
  {
#if PRINTF_HEAP_USE
    temp = aws_malloc(len + 1);  // null포함
    if (temp)
    {
      va_start(ap, pFmt);
      len = vsnprintf_s((char *)temp, len + 1, (char *)pFmt, ap);
      va_end(ap);
      ptr = temp;
    }
    else
    {
      return 1;  // 메모리 할당 에러
    }
#else
    va_start(ap, pFmt);
    len = vsnprintf_s((char *)g_printf_buff, sizeof(g_printf_buff), (char *)pFmt, ap);
    va_end(ap);

    ptr = g_printf_buff;
#endif
  }
  else
  {
    ptr = buff;  // 1바이트만 전송하게 되면 버퍼로 처리
  }

  if (debug_uart && ptr)  // os구동중인지 확인
  {
    driver_uart_send(debug_uart, "\x1B[31m", 5);
    driver_uart_send(debug_uart, (uint8_t *)ptr, strlen(ptr));
    driver_uart_send(debug_uart, "\x1B[37m", 5);
  }
  else if (ptr)  // os 없으면
  {
    debug_puts_nonos((char *)"\x1B[31m");
    debug_puts_nonos(ptr);
    debug_puts_nonos((char *)"\x1B[37m");
  }
#if PRINTF_HEAP_USE
  if (temp)
  {
    aws_free(temp);
  }
#endif
  return 0;
}

void debug_send(uint8_t *pData, uint16_t dataLen) { driver_uart_send(debug_uart, pData, dataLen); }

void debug_putch(char ch) { driver_uart_send(debug_uart, (uint8_t *)&ch, 1); }
void debug_puts(const char *str)
{
  while (*str)
  {
    driver_uart_send(debug_uart, (uint8_t *)str, 1);
    str++;
  }
}

int32_t debug_recv(char *out, uint16_t outSize, uint32_t timeout)
{
  int32_t cnt;

  cnt = driver_uart_recv(debug_uart, (uint8_t *)out, outSize, timeout);

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

  debug_printf("\n\r\n\r                ");
  len = 0;
  temp[0] = 0;

  for (j = 0; j < col; j++)
  {
    len = strnlen_s(temp, sizeof(temp));
    snprintf_s(&temp[len], sizeof(temp) - len, "%02X ", j);
  }
  debug_printf(temp);

  debug_printf("  ");
  for (j = 0; j < col; j++)
  {
    debug_printf("%X", j % 16);
  }

  for (i = 0; i < row; i++)
  {
    snprintf_s(temp, sizeof(temp), "\n\r%04d  %08X  ", (int32_t)(i * col), (startAddr + i * col));
    debug_printf(temp);

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
    debug_printf(temp);
    debug_printf("  ");

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
    debug_printf(temp);
  }
  debug_printf("\n\r");
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
      driver_rs485_send(dev->driver, data, dataLen);
      break;
    case eRS232_IO:
      driver_uart_send(dev->driver, data, dataLen);
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
       return driver_rs485_recv_opt(dev->driver, out, dataLen, pdevopt->waitTimeOutMs,data_timeout);
      break;
    case eRS232_IO:

      return driver_uart_recv_opt(dev->driver, out, dataLen, pdevopt->waitTimeOutMs, data_timeout);
      break;
  }
  return 0;
}



