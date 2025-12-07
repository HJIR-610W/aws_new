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
#include "shell.h"

#define TELNET_MIRROR_USE 1

#define DEBUG_UART_NUM BSP_UART_10_CDC


io_if_t g_debug_uart_io;
/*
//디버깅 포트는 초기화 안되더라도  NULL이 안되게 한다. 실제 최종 send,recv 함수 안에서 에러 처리 
telent 포트랑 미러링 때문에 
injection 처리가 안된다.

rs232이는 커넥터 연결 감지 없기때문에 항상 정상처리해야하며
usb 인경우 초기화 실패 있지만 드라이버 자체에서 에러 처리 해야한다.

여기서 debug 포트를 io 함수로 사용하는 이유는 
debug포트가 꼭 uart가 아닐수도 있다.
rs485
tcp
spi 등 입출력만 된다면 아무거나 가능하다.

*/
io_if_t *g_current_debug_io;

io_ops_t g_uart_io_ops={.recv = drv_uart_io_recv,
                        .send = drv_uart_io_send,
                        .flush = drv_uart_io_flush,
                        .ioctl = NULL,
                        .inject = drv_uart_io_inject};


io_ops_t g_telnet_io_ops={.recv = telnet_io_recv,
                          .send = telnet_io_send,
                          .flush = telnet_io_flush,
                          .ioctl = NULL,
                          .inject =NULL};

void debug_set_io(io_if_t *debug_io)
{
  g_current_debug_io = debug_io;
}

void debug_set_io_default(void)
{
  g_current_debug_io = &g_debug_uart_io;
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


    debug_send( (uint8_t *)ptr, strlen(ptr));


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

/**
 * 
 */
void debug_inject(uint8_t *data,size_t len)
{
//  io_if_t io;

 // io.dev_num = DEBUG_UART_NUM;
 // g_uart_io_ops.inject(&io,data,len);

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

  debug_send(ptr,strlen((char*)ptr));

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
#if TELNET_MIRROR_USE ==1
  telnet_send(data,len);
#endif
}

void debug_put_ch(uint8_t ch)
{
  io_send(g_current_debug_io, &ch, 1);
  #if TELNET_MIRROR_USE ==1
  telnet_send(&ch,1);
  #endif
}

void debug_puts(const uint8_t *string)
{
  size_t len = strlen((char *)string);

  io_send(g_current_debug_io, string, len);
  #if TELNET_MIRROR_USE ==1
  telnet_send(string, len);
  #endif
}

int32_t debug_get_ch(uint8_t *buffer)
{
  return io_recv(g_current_debug_io,buffer,1, 0xFFFFFFFF);
}

int32_t debug_get_ch_nonblocking(uint8_t *buffer)
{
  return   io_recv(g_current_debug_io,buffer,1, 0);
}



int32_t debug_get_key(uint32_t timeout_ms)
{
  uint8_t ch;
  uint32_t start_time = osKernelGetTickCount();
  uint32_t elapsed = 0;


  while (1)
  {
    uint32_t remain = timeout_ms - elapsed;
    if (remain == 0)
    {
      return (int32_t)KEY_CODE_NONE;
    }

    if (debug_recv(&ch, 1, remain) == 1)
    {
      break;
    }

    elapsed = osKernelGetTickCount() - start_time;
    if (elapsed >= timeout_ms)
    {
      return (int32_t)KEY_CODE_NONE;
    }
  }


  if (ch == 0x1B)
  {
    uint8_t seq[2];
    int seq_idx = 0;
    elapsed = osKernelGetTickCount() - start_time;

    while (seq_idx < 2)
    {
      uint32_t remain = timeout_ms - elapsed;
      if (remain == 0)
      {
        return (int32_t)KEY_CODE_ESC;
      }

      if (debug_recv(&seq[seq_idx], 1, remain) == 1)
      {
        seq_idx++;
      }

      elapsed = osKernelGetTickCount() - start_time;
      if (elapsed >= timeout_ms)
      {
        return (int32_t)KEY_CODE_ESC;
      }
    }

    if (seq[0] == '[')
    {
      switch (seq[1])
      {
        case 'A':
          return KEY_CODE_UP;
        case 'B':
          return KEY_CODE_DOWN;
        case 'C':
          return KEY_CODE_RIGHT;
        case 'D':
          return KEY_CODE_LEFT;
        case 'H':
          return KEY_CODE_HOME;
        case 'F':
          return KEY_CODE_END;
        default:
          return KEY_CODE_UNKNOWN;
      }
    }
    else if (seq[0] == 'O')
    {
      switch (seq[1])
      {
        case 'H':
          return KEY_CODE_HOME;
        case 'F':
          return KEY_CODE_END;
        default:
          return KEY_CODE_UNKNOWN;
      }
    }

    return KEY_CODE_UNKNOWN;
  }


  if (ch >= 0x01 && ch <= 0x1A)
  {
    return (int32_t)ch;
  }


  return (int32_t)ch;
}







int32_t debug_init(void)
{
  int32_t result;
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  uart_config.baud = 115200;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  
  drv_uart_init(DEBUG_UART_NUM, &uart_config,"debug");

  io_init(&g_debug_uart_io,IO_COM_TYPE_RS232,&g_uart_io_ops,DEBUG_UART_NUM);
  g_current_debug_io = &g_debug_uart_io;
    
  return 1;

}

void debug_deinit(void)
{
  drv_uart_deinit(DEBUG_UART_NUM);
  g_current_debug_io = NULL;
}