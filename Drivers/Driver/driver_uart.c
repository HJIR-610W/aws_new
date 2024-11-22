


#include <stdint.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "driver_stm32_uart.h"

#include "driver_uart.h"
#include "TL16C554.h"


#define BUFFER_SIZE 256 // 링 버퍼의 크기

// 링 버퍼 구조체
typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    size_t head;
    size_t tail;
} RingBuffer;

static RingBuffer uartRingBuffer;
static osSemaphoreId uartDataAvailable; // 데이터가 링 버퍼에 있음을 나타내는 세마포어
osSemaphoreDef(uartDataAvailable); // 세마포어 정의

// 링 버퍼 초기화 함수
void ring_buffer_init(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
}

// 링 버퍼가 비어 있는지 확인
bool ring_buffer_is_empty(RingBuffer *rb) {
    return rb->head == rb->tail;
}

// 링 버퍼에 데이터 삽입
bool ring_buffer_put(RingBuffer *rb, uint8_t data) {
    size_t next_head = (rb->head + 1) % BUFFER_SIZE;
    if (next_head == rb->tail) {
        // 버퍼가 가득 참
        return false;
    }
    rb->buffer[rb->head] = data;
    rb->head = next_head;
    return true;
}

// 링 버퍼에서 데이터 가져오기
bool ring_buffer_get(RingBuffer *rb, uint8_t *data) {
    if (ring_buffer_is_empty(rb)) {
        return false;
    }
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % BUFFER_SIZE;
    return true;
}

// 시리얼 수신 함수 (플랫폼에 맞게 구현 필요)
bool serial_receive_nonblocking(uint8_t *data, size_t length) {
    // 하드웨어 의존적인 비차단 수신 함수 구현
    // ex) UART에 대한 수신 데이터 체크 후 데이터 복사
    return true; // 데이터 수신 시 true 반환
}


typedef struct adc_api_s
{
    void (*send)(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen);
    int32_t (*recv)(driver_t *tls16c554,uint8_t *pBuff);
    int32_t (*recv_byte)(driver_t *tls16c554,uint8_t *pData);
    void (*set)(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option);
    void (*init)(driver_t *tls16c554);
    uint16_t (*recv_bytes)(driver_t *tls16c554,uint8_t *pData,uint16_t rLen,uint32_t timeoutMs);
}rs232_api_t;



driver_t g_rs232[UART_MAX];
static rs232_api_t g_rs232_api ={.send = tls16c554_send,
                                 .recv = tls16c554_recv,
                                 .recv_byte = tls16c554_recv_byte,
                                 .set = tls16c554_set,
                                 .init =tls16c554_init,
                                 .recv_bytes=tls16c554_uart_recvs};
              
static rs232_api_t g_stm32_uart_api ={.send = stm32_uart_send,
                                      .recv = tls16c554_recv,
                                      .recv_byte = tls16c554_recv_byte,
                                      .set = stm32_uart_set,
                                      .recv_bytes = stm32_uart_recv};


void quad_gpio_init(void)
{

}
driver_t *driver_uart_open(int  num)
{
  uart_baud_config_t cfg_baud;
 
  if(g_rs232[num].opened == true)
  {
   return &g_rs232[num];
  }
  
  switch(num)
  {
    case UART_STM32_1:  //디버깅용
    g_rs232[num].name ="UART_STM32_1";
    g_rs232[num].handle = stm32_uart_open(num);;
    g_rs232[num].api = &g_stm32_uart_api;
      break;
  case UART_STM32_3:  //D-SUB
    g_rs232[num].name ="UART_STM32_3";
    g_rs232[num].handle = stm32_uart_open(num);;
    g_rs232[num].api = &g_stm32_uart_api;
      break;
    case UART_STM32_6:
      g_rs232[num].name ="UART_STM32_6";
      g_rs232[num].handle = stm32_uart_open(num);;
      g_rs232[num].api = &g_stm32_uart_api;
      break;
    case UART_EX_232_1:
      g_rs232[num].name ="UART_EX_232_1";
      g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
      cfg_baud.baud = 115200;
      tls16c554_set(g_rs232[num].handle,eUART_SET_CONFIG,&cfg_baud);
      g_rs232[num].api = &g_rs232_api;
      break;
    case UART_EX_TTL_2:  
        g_rs232[num].name ="UART_EX_TTL_2";
        g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
       cfg_baud.baud = 115200;
      tls16c554_set(g_rs232[num].handle,eUART_SET_CONFIG,&cfg_baud);
        g_rs232[num].api = &g_rs232_api;
          break;
    case UART_EX_232_A_3:
        g_rs232[num].name ="UART_EX_232_A_3";
        g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
        cfg_baud.baud = 1200;
        tls16c554_set(g_rs232[num].handle,eUART_SET_CONFIG,&cfg_baud);
        g_rs232[num].api = &g_rs232_api;
          break;
    case UART_EX_232_B_4:
        g_rs232[num].name ="UART_EX_232_B_4";
        g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
        cfg_baud.baud = 1200;
        tls16c554_set(g_rs232[num].handle,eUART_SET_CONFIG,&cfg_baud);
        g_rs232[num].api = &g_rs232_api;
          break;
    case UART_EX_485_1:
        g_rs232[num].name ="UART_EX_485_1";
        g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
       cfg_baud.baud = 9600;
      tls16c554_set(g_rs232[num].handle,eUART_SET_CONFIG,&cfg_baud);
        g_rs232[num].api = &g_rs232_api;
          break;
    case UART_EX_485_2:
      g_rs232[num].name ="g_rs232";
      g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
      cfg_baud.baud = 115200;
      tls16c554_set(g_rs232[num].handle,eUART_SET_CONFIG,&cfg_baud);
      g_rs232[num].api = &g_rs232_api;
          break;
    case UART_EX_232_C_7:
      g_rs232[num].name ="UART_EX_232_C_7";
      g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
      cfg_baud.baud = 1200;
      tls16c554_set(g_rs232[num].handle,eUART_SET_CONFIG,&cfg_baud);
      g_rs232[num].api = &g_rs232_api;
          break;
    case UART_EX_232_D_8:
      g_rs232[num].name ="UART_EX_232_D_8";
      g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
      cfg_baud.baud = 1200;
      tls16c554_set(g_rs232[num].handle,eUART_SET_CONFIG,&cfg_baud);
      g_rs232[num].api = &g_rs232_api;
   break;    
  }
         return &g_rs232[num];

}

void driver_uart_send(driver_t *drv,uint8_t *pData,uint16_t dataLen)
{
  rs232_api_t *api = (rs232_api_t *)drv->api;

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
  if(dataLen)
  {
    api->send(drv->handle,pData,dataLen);
  }
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

}

int32_t driver_uart_recv(driver_t *drv,uint8_t *pBuff)
{
  rs232_api_t *api = (rs232_api_t *)drv->api;
  
    if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
  api->recv(drv->handle,pBuff);
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

  return 0;
}

int driver_recv_uart_byte(driver_t *drv,uint8_t *pData)
{
  int32_t cnt;

  rs232_api_t *api = (rs232_api_t *)drv->api;
      if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
 cnt =  api->recv_byte(drv->handle,pData);

   if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

  return cnt;
}

void driver_uart_set(driver_t *uart,eUART_SET_CMD_t cmd,void *para)
{
  rs232_api_t *api = (rs232_api_t *)uart->api;
  

  api->set(uart->handle,cmd,para);  
}

void driver_uart_get(driver_t *uart,eUART_SET_CMD_t cmd,void *config)
{

}

uint16_t driver_uart_recvs(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutMs)
{
  rs232_api_t *api = (rs232_api_t *)drv->api;
  uint16_t cnt;

      if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }

  cnt = api->recv_bytes(drv->handle,pBuff,rLen,timeOutMs);
   if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
  return cnt;
}





