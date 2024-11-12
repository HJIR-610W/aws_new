


#include <stdint.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"


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

#if 0 

// COBS 디코딩 함수
size_t cobs_decode(const uint8_t *input, size_t length, uint8_t *output) {
    if (length == 0) return 0;

    size_t read_index = 0, write_index = 0;
    while (read_index < length) {
        uint8_t code = input[read_index];
        if (read_index + code > length && code != 1) {
            // 유효하지 않은 COBS 패킷
            return 0;
        }

        read_index++;
        for (uint8_t i = 1; i < code; i++) {
            output[write_index++] = input[read_index++];
        }

        if (code != 0xFF && read_index < length) {
            output[write_index++] = 0;
        }
    }
    return write_index;
}

// 비차단 시리얼 데이터 수신 및 COBS 디코딩
bool recv_cobs(uint8_t *buff, uint16_t buffSize, uint32_t timeOutMs) {
    uint8_t tempBuffer[256]; // 임시 수신 버퍼
    size_t receivedLength = 0;
    uint32_t startTime = osKernelSysTick();
uint32_t timeoutTicks = timeOutMs * (configTICK_RATE_HZ  / 1000);


    while ((osKernelSysTick() - startTime) < timeoutTicks) {
        if (serial_receive_nonblocking(tempBuffer + receivedLength, 1)) {
            receivedLength++;
            // 0x00은 COBS 프레임의 종료 바이트
            if (tempBuffer[receivedLength - 1] == 0x00) {
                size_t decodedLength = cobs_decode(tempBuffer, receivedLength, buff);
                return decodedLength > 0; // 성공 시 true 반환
            }
        }
        osDelay(1); // 1ms 대기 후 다시 시도 (비차단)
    }

    return false; // 타임아웃 발생 시 false 반환
}

#endif







typedef struct adc_api_s
{
    void (*send)(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen);
    void (*recv)(driver_t *tls16c554,uint8_t *pBuff,uint16_t rLen);
    int32_t (*recv_byte)(driver_t *tls16c554,uint8_t *pData);
    void (*set)(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option);
    void (*init)(driver_t *tls16c554);
}rs232_api_t;



driver_t g_rs232[8];
static rs232_api_t g_rs232_api ={.send = tls16c554_send,
                                 .recv = tls16c554_recv,
                                 .recv_byte = tls16c554_recv_byte,
                                 .set = tls16c554_set,
                                 .init =tls16c554_init};
              
driver_t *driver_uart_open(int  num)
{
 
  if(g_rs232[num].opened == true)
  {
   return &g_rs232[num];
  }
  
  switch(num)
  {
    case UART_EX_232_1:
    case  UART_EX_TTL_2:  
    case UART_EX_232_3:
    case  UART_EX_232_4:
    case  UART_EX_232_7:
    case  UART_EX_232_8:
        g_rs232[num].handle = tls16c554_open(num - UART_EX_232_1);;
        return &g_rs232[num];
    break;    
  }
 
  return 0;
}

void driver_send_uart(driver_t *uart,uint8_t *pData,uint16_t dataLen)
{
  rs232_api_t *api = (rs232_api_t *)uart->api;
  
  api->send(uart->handle,pData,dataLen);
}

void driver_recv_uart(driver_t *uart,uint8_t *pBuff,uint16_t buffSize)
{
  rs232_api_t *api = (rs232_api_t *)uart->api;
  
  api->recv(uart->handle,pBuff,buffSize);
}

int driver_recv_uart_byte(driver_t *uart,uint8_t *pData)
{
  rs232_api_t *api = (rs232_api_t *)uart->api;
  
  return api->recv_byte(uart->handle,pData);
}

void driver_set_uart(driver_t *uart,eUART_SET_CMD_t cmd,void *para)
{
      rs232_api_t *api = (rs232_api_t *)uart->api;
  
  api->set(uart->handle,cmd,para);  
}

void driver_get_uart(driver_t *uart,eUART_SET_CMD_t cmd,void *config)
{
  rs232_api_t *api = (rs232_api_t *)uart->api;
  
//  api->init(uart->handle);
}








