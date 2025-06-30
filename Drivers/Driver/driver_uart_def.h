

#ifndef DRIVER_UART_DEF_H
#define DRIVER_UART_DEF_H

#include <stdint.h>

#include "driver_interface.h"
#define PARITY_NONE 0
#define PARITY_ODD 1
#define PARITY_EVEN 2

#define UART_DATA_LEN_8 0
#define UART_DATA_LEN_9 1
typedef struct uart_baud_config_s
{
  int baud;
  uint8_t parityIdx;
  uint8_t stop_bit;
  uint8_t dataLen;
} uart_config_t;

typedef enum uart_set_cmd_s
{
  eUART_SET_CONFIG,
  eUART_SET_TIMEOUT
} eUART_SET_CMD_t;

typedef struct uart_optTimeOut_s
{
  uint32_t frameTimeOutMs;
  uint32_t dataTimeOutMs;
} uart_optTimeOut_t;

typedef enum uart_recv_opt_s
{
  /*
   최소 1바이트 수신 후 특정 시간동안 데이터 수신 없는경우 리턴
   그사이 대기시간이 초과되면 리턴
   예)프레임 대기시간 100ms,데이터 대기시간 2ms
   100ms동안 데이터 수신 없으면 리턴
   100ms프레임 시간동안 데이터가 수신되고 연속해서 데이터 대기시간 동안 데이터
   수신 없으면 프레임 시간이 남았다고 하더라고 리턴
   - modbus 같은 경우 유용
  */
  eUART_OPT_DATA_TIMEOUT_1,

} eUART_RECV_OPT_t;

typedef enum
{
  UART_SET_BAUDRATE,   // 보드레이트 설정
  UART_SET_MODE,       // 모드 설정 (일반, DMA, 저전력)
  UART_SET_CALLBACK,   // 콜백 함수 등록
  UART_SET_PARITY,     // 패리티 설정
  UART_SET_STOP_BITS,  // 정지 비트 설정
  UART_SET_DATA_BITS,  // 데이터 비트 설정
  UART_SET_FLOW_CTRL,  // 흐름 제어 설정 (CTS/RTS)
} uart_set_option_t;

typedef enum
{
  UART_GET_CONFIG  // 설정값 읽기
} uart_get_option_t;
typedef struct
{
  void (*close)(driver_t *handle);
  int32_t (*send)(driver_t *handle, const uint8_t *data, uint16_t length);
  int32_t (*recv)(driver_t *handle, uint8_t *buffer, uint16_t length, uint32_t timeout);
  int32_t (*recv_ll)(driver_t *handle, uint8_t *buffer, uint16_t length, uint32_t timeout);
  void (*flush_rx)(driver_t *handle);
  int32_t (*available)(driver_t *handle);

  int32_t (*recv_opt)(driver_t *drv, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms,
                      uint32_t timeout2_ms);

  // **하나의 set() 함수로 모든 설정 관리**
  void (*set)(driver_t *handle, uart_set_option_t option, void *value);
  void (*get)(driver_t *handle, uart_get_option_t option, void *value);
  int (*inject)(driver_t *handle, const uint8_t *data, uint16_t length);
} uart_api_t;

#endif
