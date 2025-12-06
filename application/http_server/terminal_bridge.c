#include "websocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmsis_os2.h"
#include "task_logging.h"
#include "user_heap.h"
#include "debug_io.h"
#include "task_core_debug.h"
#include "io_interface.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"

io_if_t g_telnet_io;

int telnet_io_recv(io_if_t *io,uint8_t *buffer,size_t len,uint32_t timeout_ms);
void telnet_io_inject(io_if_t *io,uint8_t *data,size_t len);
void telnet_io_init(void);
int telnet_io_send(io_if_t *io,const uint8_t *data,size_t len );
void telnet_io_flush(io_if_t *io);
void telnet_io_inject(io_if_t *io,uint8_t *data,size_t len);

// 터미널 브리지 전역 변수
static void (*g_terminal_output_callback)(const uint8_t* data, size_t len) = NULL;
static bool g_bridge_initialized = false;


void terminal_bridge_init(void)
{

    if (!g_bridge_initialized) 
    {
      //  telnet_io_init();
        g_bridge_initialized = true;
        task_printf("Terminal Bridge: 초기화 완료 - 콘솔 시스템 연결 필요\r\n");
    }

}

//tcp에서 데이선 수신되면 
void terminal_bridge_send_command(const char* command, size_t len)
{
    if (!g_bridge_initialized || !command || len == 0)
    {
        return;
    }
    
   // telnet_io_inject(NULL,(uint8_t *)command,len);

    debug_inject((uint8_t *)command,len);
  
}

void terminal_bridge_set_output_callback(void (*callback)(const uint8_t* data, size_t len))
{
    
    g_terminal_output_callback = callback;
    task_printf("Terminal Bridge: 출력 콜백 설정 완료\r\n");
}

void terminal_bridge_cleanup(void)
{
    g_terminal_output_callback = NULL;
    g_bridge_initialized = false;
    debug_set_io_default();

}


bool terminal_bridge_is_initialized(void)
{
    return g_bridge_initialized;
}

void terminal_bridge_send_prompt(void)
{
    const uint8_t* prompt = "$ ";
    
    if (g_terminal_output_callback )
    {
        g_terminal_output_callback(prompt, strlen((char *)prompt));
    }
}




static StreamBufferHandle_t stream_buffer;

void telnet_io_init(void)
{
    if(stream_buffer==NULL)
    stream_buffer =  xStreamBufferCreate(200, 1);


}

void telnet_send(const uint8_t *data,size_t len )
{
     if (g_terminal_output_callback && data && len > 0)
    {
        g_terminal_output_callback(data, len);
    }
}


int telnet_io_send(io_if_t *io,const uint8_t *data,size_t len )
{
    if (g_terminal_output_callback && data && len > 0)
    {
        g_terminal_output_callback(data, len);
    }
    return 0;
}

int telnet_io_recv(io_if_t *io,uint8_t *buffer,size_t buffSize,uint32_t timeout_ms)
{
 uint32_t start_tick;
  uint32_t elapsed_tick;
  uint32_t remaining_timeout;
  size_t bytes_available;
  size_t bytes_read;
  size_t cnt = 0;



  // timeOutMs가 0인 경우: 논블로킹 모드
  if (timeout_ms == 0)
  {
    bytes_available = xStreamBufferBytesAvailable(stream_buffer);
    //읽을 데이터가 있으면 읽고 버퍼보다 더 많으면 버퍼만큼만 읽는다
    if (bytes_available > 0)
    {
      size_t bytes_to_read = (bytes_available > buffSize) ? buffSize : bytes_available;
      bytes_read = xStreamBufferReceive(stream_buffer,
                                        buffer,
                                        bytes_to_read,
                                        0); // 대기시간 0
      cnt = bytes_read;
    }
    // 데이터가 없으면 cnt는 0으로 리턴

    return cnt;
  }

  start_tick = osKernelGetTickCount();

  // timeOutMs가 0xFFFFFFFF인 경우: 무한 대기 모드
  // 무한대기하면서 버퍼가 완전히 차면 리턴
  if (timeout_ms == 0xFFFFFFFF)
  {
    while (cnt < buffSize)
    {
      bytes_available = xStreamBufferBytesAvailable(stream_buffer);

      size_t bytes_to_read = buffSize - cnt;//남은 버퍼 사이즈
      if (bytes_available > bytes_to_read)//읽을것이 버퍼 사이즈 보다 크면 버퍼만큼만 읽는다
      {
        bytes_available = bytes_to_read;
      }

      if (bytes_available == 0)//읽을 데이터가 없으면 한바이트 수신될때가지 무한대기
      {
        // 데이터가 없으면 최소 1바이트 수신까지 무한 대기
        bytes_read = xStreamBufferReceive(stream_buffer,
                                          &buffer[cnt],
                                          1,
                                          osWaitForever);
      }
      else
      {
        // 사용 가능한 데이터를 읽음
        bytes_read = xStreamBufferReceive(stream_buffer,
                                          &buffer[cnt],
                                          bytes_available,
                                          osWaitForever);
      }

      if (bytes_read > 0)
      {
        cnt += bytes_read;
      }
    }
  }
  // timeOutMs가 양수인 경우: 지정된 타임아웃 적용
  else
  {
    uint32_t timeout_tick = pdMS_TO_TICKS(timeout_ms);

    while (cnt < buffSize)
    {
      elapsed_tick = osKernelGetTickCount() - start_tick;

      if (elapsed_tick >= timeout_tick)
      {
        break; // Timeout 발생
      }

      remaining_timeout = timeout_tick - elapsed_tick;

      bytes_available = xStreamBufferBytesAvailable(stream_buffer);

      size_t bytes_to_read = buffSize - cnt;
      if (bytes_available > bytes_to_read)
      {
        bytes_available = bytes_to_read;
      }

      if (bytes_available == 0)
      {
        // 데이터가 없으면 최소 1바이트 수신 대기
        bytes_read = xStreamBufferReceive(stream_buffer,
                                          &buffer[cnt],
                                          1,
                                          remaining_timeout);
      }
      else
      {
        // 데이터를 읽음
        bytes_read = xStreamBufferReceive(stream_buffer,
                                          &buffer[cnt],
                                          bytes_available,
                                          remaining_timeout);
      }

      if (bytes_read > 0)
      {
        cnt += bytes_read;
      }
      else
      {
        // xStreamBufferReceive가 0을 리턴하면 타임아웃 발생
        break;
      }
    }
  }

  return cnt;
}

void telnet_io_flush(io_if_t *io)
{
  uint8_t data;

  while (xStreamBufferBytesAvailable(stream_buffer) > 0)
  {
    xStreamBufferReceive(stream_buffer, &data, 1, 0);
  }

}

void telnet_io_inject(io_if_t *io,uint8_t *data,size_t len)
{

   xStreamBufferSend(stream_buffer, data, len,pdMS_TO_TICKS(100));



}


