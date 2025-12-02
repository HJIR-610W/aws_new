#include "drv_uart_def.h"

#include "bsp_stm32_uart.h"
#include "bsp_stm32_cdc.h"
#include "bsp_uart.h"
#include "TL16C554.h"
#include "cmsis_os2.h"
#include "dev_io.h"

// 드라이버 타입 정의
typedef enum {
  UART_DRIVER_STM32,
  UART_DRIVER_TL16C554,
  UART_DRIVER_CDC,
  UART_DRIVER_INVALID
} uart_driver_type_t;

// UART 핀맵 구조체
typedef struct {
  uart_driver_type_t driver_type;
  int driver_num;
} uart_pinmap_t;

// BSP UART 번호를 드라이버 번호로 매핑하는 테이블
static const uart_pinmap_t uart_pinmap[11] = {
  [BSP_UART_0_D_SUB_0]  = {UART_DRIVER_TL16C554,    TL16C554_UART_1_D_SUB},        // VHF
  [BSP_UART_1_TTL_ONLY]      = {UART_DRIVER_TL16C554, TL16C554_UART_2_TTL_TTL},   // 블루투스 모듈
  [BSP_UART_2_EXT_A]    = {UART_DRIVER_TL16C554, TL16C554_UART_3_RS232_A},   // 사용자0
  [BSP_UART_3_EXT_B]    = {UART_DRIVER_TL16C554, TL16C554_UART_4_RS232_B},   // 사용자1
  [BSP_UART_4_EXT_C]    = {UART_DRIVER_TL16C554, TL16C554_UART_7_RS232_C},   // 사용자2
  [BSP_UART_5_EXT_D]    = {UART_DRIVER_TL16C554, TL16C554_UART_8_RS232_D},   // 사용자3
  [BSP_UART_6_RS485_A_ONLY]  = {UART_DRIVER_TL16C554, TL16C554_UART_5_RS485_A},   // RS485 A
  [BSP_UART_7_RS485_B_ONLY]  = {UART_DRIVER_TL16C554, TL16C554_UART_6_RS485_B},   // RS485 B
  [BSP_UART_8_CDMA]     = {UART_DRIVER_STM32,    STM32_UART_0_CDMA},        // CDMA
  [BSP_UART_9_SDI_ONLY]      = {UART_DRIVER_STM32,    STM32_UART_1_SDI},         // SDI통신
  [BSP_UART_10_CDC]     = {UART_DRIVER_CDC,      STM32_CDC}                 // USB 디버깅
};

// 유효한 BSP UART 번호인지 확인
static inline bool is_valid_uart_num(int num) {
  return (num >= 0 && num <= 10);
}

// 핀맵 정보 가져오기
static inline const uart_pinmap_t* get_uart_pinmap(int num) {
  if (!is_valid_uart_num(num)) {
    return 0;
  }
  return &uart_pinmap[num];
}

int32_t bsp_uart_init(int32_t num, void *opt)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return -1;
  if (num == -1)
    return -1;
  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      return stm32_uart_init(pinmap->driver_num, opt);
    case UART_DRIVER_TL16C554:
      return tl16c554_init(pinmap->driver_num, opt);
    case UART_DRIVER_CDC:
      return stm32_cdc_init(pinmap->driver_num, opt);

    default:
      return -1;
  }
}

void bsp_uart_close(int num)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return;
  if (num == -1)
    return ;
  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      stm32_uart_close(pinmap->driver_num);
      break;
    case UART_DRIVER_TL16C554:
      tl16c554_close(pinmap->driver_num);
      break;
    case UART_DRIVER_CDC:

      break;
    default:
      break;
  }
}

void bsp_uart_flush_rx(int num)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return;
  if (num == -1)
    return ;
  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      stm32_uart_flush_rx(pinmap->driver_num);
      break;
    case UART_DRIVER_TL16C554:
      tl16c554_recv_flush(pinmap->driver_num);
      break;
    case UART_DRIVER_CDC:
 stm32_cdc_flush_rx();
      break;
    default:
      break;
  }
}

void bsp_uart_set(int num, eUART_SET_OPTION_t cmd, void *option)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return;
  if (num == -1)
    return ;
  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      stm32_uart_set(pinmap->driver_num, cmd, option);
      break;
    case UART_DRIVER_TL16C554:
      tl16c554_set(pinmap->driver_num, cmd, option);
      break;
    case UART_DRIVER_CDC:
      // CDC does not have set function
      break;
    default:
      break;
  }
}

int32_t bsp_uart_send(int num, const uint8_t *pData, uint16_t dataLen)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return -1;
  if (num == -1)
    return -1;
  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      return stm32_uart_send(pinmap->driver_num, pData, dataLen);
    case UART_DRIVER_TL16C554:
      return tl16c554_send(pinmap->driver_num, pData, dataLen);
    case UART_DRIVER_CDC:
      return stm32_cdc_send(pinmap->driver_num, pData, dataLen);
    default:
      return -1;
  }
}

int32_t bsp_uart_recv(int num, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs)
{
  int32_t len;
  
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return -1;
  if(num==-1)
  return -1;


  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      len =  stm32_uart_recv(pinmap->driver_num, pBuff, buffSize, timeOutMs);
      break;
    case UART_DRIVER_TL16C554:
      len = tl16c554_recv(pinmap->driver_num, pBuff, buffSize, timeOutMs);
      break;
    case UART_DRIVER_CDC:
      len =  stm32_cdc_recv(pinmap->driver_num, pBuff, buffSize, timeOutMs);
      break;
    default:
      len = -1;
  }
  

  return len;
}

int32_t bsp_uart_recv_opt(int num, uint8_t *buffer, uint16_t buffer_size, uint32_t timeout1_ms, uint32_t timeout2_ms)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  int32_t len;

  if (!pinmap) return -1;
  if (num == -1)
    return -1;
  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      len =  stm32_uart_recv_opt(pinmap->driver_num, buffer, buffer_size, timeout1_ms, timeout2_ms);
      break;
    case UART_DRIVER_TL16C554:
      len = tl16c554_recv_opt(pinmap->driver_num, buffer, buffer_size, timeout1_ms, timeout2_ms);
      break;
    case UART_DRIVER_CDC:
      len =  stm32_cdc_recv_opt(pinmap->driver_num, buffer, buffer_size, timeout1_ms, timeout2_ms);
      break;
    default:
      return -1;
  }

  return len;
}

void bsp_uart_get(int num, eUART_GET_OPTION_t cmd, void *option)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return;
  if (num == -1)
    return ;
  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      stm32_uart_get(pinmap->driver_num, cmd, option);
      break;
    case UART_DRIVER_TL16C554:
      tl16c554_get(pinmap->driver_num, cmd, option);
      break;
    case UART_DRIVER_CDC:
      // CDC does not have get function
      break;
    default:
      break;
  }
}

int32_t bsp_uart_inject(int num, const uint8_t *pData, uint16_t dataLen)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return -1;
  if (num == -1)
    return -1;
  switch (pinmap->driver_type) {
    case UART_DRIVER_STM32:
      return stm32_uart_inject(pinmap->driver_num, pData, dataLen);
    case UART_DRIVER_TL16C554:
      return tl16c554_recv_inject(pinmap->driver_num, pData, dataLen);
    case UART_DRIVER_CDC:
      return stm32_cdc_inject(pinmap->driver_num, pData, dataLen);
    default:
      return -1;
  }
}

int32_t bsp_uart_recv_crlf(int num, char *pBuff, uint16_t bSize, uint32_t tout_ms)
{
    const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return -1;
  if (num == -1)
    return -1;
  switch (pinmap->driver_type)
  {
    case UART_DRIVER_STM32:
      return stm32_uart_recv_crlf(pinmap->driver_num, pBuff, bSize, tout_ms);
    case UART_DRIVER_TL16C554:
      return tl16c554_recv_crlf(pinmap->driver_num, pBuff, bSize, tout_ms);
    case UART_DRIVER_CDC:
      return stm32_cdc_recv_crlf(pinmap->driver_num, pBuff, bSize, tout_ms);
    default:
      return -1;
  }
}

int32_t bsp_uart_get_char(int num, uint8_t *pBuff, uint16_t rLen)
{
  if (num == -1)
    return -1;

  return bsp_uart_recv(num,pBuff,1,osWaitForever);
  
}

int32_t bsp_uart_get_charNonBlocking(int num, uint8_t *pBuff)
{
  if (num == -1)
    return -1;
  return -1;
}

int32_t bsp_uart_recv_ll(int num, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutMs)
{
  const uart_pinmap_t* pinmap = get_uart_pinmap(num);
  if (!pinmap) return -1;
  if (num == -1)
    return -1;
  switch (pinmap->driver_type)
  {
    case UART_DRIVER_STM32:

      return -1;
    case UART_DRIVER_TL16C554:
      return tl16c554_recv_ll(pinmap->driver_num, pBuff, rLen, timeOutMs);
    case UART_DRIVER_CDC:

      return -1;
    default:
      return -1;
  }
}


void bsp_uart_set_config(int num,uart_config_t *config)
{
  const uart_pinmap_t *pinmap = get_uart_pinmap(num);
  if (!pinmap)
    return ;
  if (num == -1)
    return ;
  switch (pinmap->driver_type)
  {
  case UART_DRIVER_STM32:
    stm32_uart_set_config(pinmap->driver_num, config);
    break;
  case UART_DRIVER_TL16C554:
    tl16c554_set_config(pinmap->driver_num, config);
    break;
  case UART_DRIVER_CDC:
    break;
  default:
    break;
  }
}
