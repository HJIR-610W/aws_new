#define __STDC_WANT_LIB_EXT1__ 1
#include "debug_io.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

 

#include "drv_rs232.h"
#include "drv_rs485.h"
#include "pcb_define.h"

#include "system_err.h"
#include "FreeRTOS.h"  // pvPortMalloc, vPortFree 사용 시 필요
#include "user_heap.h"
#include "util_time.h"
#include "task_telnet_server.h"
#include "terminal_bridge.h"
#include "io_interface.h"
 

io_if_t g_debug_uart_io;
io_if_t g_debug_uart_io;
io_if_t *g_current_debug_io;



void set_debug_io(io_if_t *debug_io)
{
  g_current_debug_io = debug_io;
}


io_if_t *get_debug_io(void) 
{ 
  return g_current_debug_io; 
}


int32_t debug_vprintf(const char *fmt, va_list ap)
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
  len = vsnprintf_s(buff, sizeof(buff), fmt, ap_copy);
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
      vsnprintf_s(temp, total_len, fmt, ap_copy);
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
    vsnprintf_s(buff, sizeof(buff), fmt, ap_copy);
    va_end(ap_copy);
    ptr = buff;
  }
#else
  // 고정 버퍼 모드: 256바이트 버퍼 사용
  vsnprintf_s(printf_buff, sizeof(printf_buff), fmt, ap);
  ptr = printf_buff;
#endif


    io_send(g_current_debug_io, (uint8_t *)ptr, strlen(ptr));


#if PRINTF_HEAP_USE
  if (temp)
  {
    vPortFree(temp);
  }
#endif

  return 0;
}





void debug_printf_color(int color, const char *fmt, ...)
{
  debug_printf("%c[%dm", 27, color);

  va_list args;
  va_start(args, fmt);
  debug_vprintf(fmt, args);
  va_end(args);

  debug_printf("%c[%dm", 27, 37);
}

int debug_scanf_s(const char *fmt, ...)
{
  va_list args;
  int ret;

  va_start(args, fmt);
  ret  = shell_vscanf_s(fmt,args);
  va_end(args);

  return ret;
}

void debug_inject(uint8_t *data,size_t len)
{
  io_inject(g_current_debug_io,data,len);
}

void debug_dump(uint8_t *src, size_t size, uint32_t startAddr, uint32_t col)
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

int32_t debug_recv(uint8_t *out_buffer, size_t out_size, uint32_t timeout_ms)
{
  int32_t len;

  len = io_recv(g_current_debug_io,out_buffer,out_size, timeout_ms);

  return len;
}



#define PRINTF_HEAP_USE 1

int32_t debug_printf(const char *fmt, ...)
{
  uint8_t buff[2];
  const uint8_t *ptr = NULL;
  uint8_t *temp = NULL;
  va_list ap;
  int32_t len=0;

  (void)len;
  (void)temp;
#if PRINTF_HEAP_USE == 0
  char printf_buff[256];
#endif

  // 먼저 필요한 길이 측정
  va_start(ap, fmt);
  len = vsnprintf_s((char *)buff, sizeof(buff), (char *)fmt, ap);
  va_end(ap);

#if PRINTF_HEAP_USE
  // 동적 메모리 할당 모드
  if (len > (sizeof(buff) - 1))
  {
    temp = user_malloc(len + 1);  // null 포함
    if (temp)
    {
      va_start(ap, fmt);
      vsnprintf_s((char *)temp, len + 1, (char *)fmt, ap);
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
    va_start(ap, fmt);
    vsnprintf_s((char *)buff, sizeof(buff), (char *)fmt, ap);
    va_end(ap);
    ptr = buff;
  }
#else
  // 고정 버퍼 모드
  va_start(ap, fmt);
  vsnprintf_s((char *)printf_buff, sizeof(printf_buff), (char *)fmt, ap);
  va_end(ap);
  ptr = printf_buff;
#endif

  io_send(g_current_debug_io,ptr,strlen((char*)ptr));

#if PRINTF_HEAP_USE
  if (temp)
  {
    user_free(temp);
  }
#endif

  return 0;
}


void debug_send(const uint8_t *data, size_t len)
{
  io_send(g_current_debug_io, data, len);
  terminal_bridge_send_output((char *)data, len);
}

void debug_put_ch(uint8_t ch)
{
  io_send(g_current_debug_io, &ch, 1);
  terminal_bridge_send_output((char *)&ch, 1);
}

void debug_puts(const uint8_t *string)
{
  size_t len = strlen((char *)string);

  io_send(g_current_debug_io, string, len);
  terminal_bridge_send_output((char *)string, len);
}

int32_t debug_get_ch(uint8_t *buffer)
{
  return   io_recv(g_current_debug_io,buffer,1, 0xFFFFFFFF);
}

int32_t debug_get_ch_nonblocking(uint8_t *buffer)
{
  return   io_recv(g_current_debug_io,buffer,1, 0);
}


#define DEBUG_UART_NUM BSP_UART_10_CDC


io_ops_t g_io_ops={.recv = drv_uart_io_recv,
                   .send = drv_uart_io_send,
                   .flush = drv_uart_io_flush,
                   .ioctl = NULL};
void debug_init(void)
{
  int32_t result;
  
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};
  uart_config.baud = 115200;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  

  result = drv_uart_init(DEBUG_UART_NUM, &uart_config,"debug");
  if(result > 0)
  {
    io_init(&g_debug_uart_io,IO_COM_TYPE_RS232,&g_io_ops,DEBUG_UART_NUM);
    g_current_debug_io = &g_debug_uart_io;
  }
}

void debug_deinit(void)
{

}