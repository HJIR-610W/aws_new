

#include "driver_485.h"

#include "driver_do.h"
#include "driver_uart.h"
#include "os_user_def.h"
#include "pcb_define.h"
typedef struct rs485_cfg_s
{
  driver_t *uart_io;
  driver_t *do_io;
} rs485_cfg_t;

rs485_cfg_t g_rs485_cfg[RS485_MAX];
driver_t g_rs485[RS485_MAX];

driver_t *driver_rs485_open(uint32_t num, void *opt)
{
  if (g_rs485[num].opened == true)
  {
    return &g_rs485[num];
  }

  g_rs485[num].cfg = &g_rs485_cfg[num];

  switch (num)
  {
    case RS485_A:
      g_rs485_cfg[num].uart_io = driver_uart_open(UART_6_RS485_A, opt);
      g_rs485_cfg[num].do_io = driver_do_open(DO_DIR_RS485_A, 0);
      g_rs485[num].name = "RS485_A";
      driver_do_low(g_rs485_cfg[num].do_io);  // 수신 모드
      break;
    case RS485_B:
      g_rs485[num].name = "RS485_B";
      g_rs485_cfg[num].uart_io = driver_uart_open(UART_7_RS485_B, opt);
      g_rs485_cfg[num].do_io = driver_do_open(DO_DIR_RS485_B, 0);
      driver_do_low(g_rs485_cfg[num].do_io);  // 수신 모드
      break;
    case RS485_C:
      g_rs485[num].name = "RS485_C";
      g_rs485_cfg[num].uart_io = driver_uart_open(UART_2_EXT_A, opt);
      g_rs485_cfg[num].do_io = driver_do_open(DO_DIR_RS485_C, 0);
      driver_do_low(g_rs485_cfg[num].do_io);  // 수신 모드
      break;
    case RS485_D:
      g_rs485[num].name = "RS485_D";
      g_rs485_cfg[num].uart_io = driver_uart_open(UART_3_EXT_B, opt);
      g_rs485_cfg[num].do_io = driver_do_open(DO_DIR_RS485_D, 0);
      driver_do_low(g_rs485_cfg[num].do_io);  // 수신 모드
      break;
  }

  g_rs485[num].opened = true;

  OS_CREATE_BINARY_SEM(g_rs485[num].sem); 


  return &g_rs485[num];
}

/**
 * @brief RS485 데이터 전송
 * @param drv
 * @param pData
 * @param dataLen
 * @retval -1 전송 오류, 0>= 전송된 데이터 수
 */
int32_t driver_rs485_send(driver_t *drv, uint8_t *pData, uint16_t dataLen)
{
  rs485_cfg_t *cfg = drv->cfg;
  int32_t cnt = 0;


  OS_PEND_SEM(drv->sem, osWaitForever);

  // TODO:이 드라이버를 호출하는 task보다 우선높은곳이 있으면 osDelay 1이상 지연됨됨
  driver_do_high(cfg->do_io);  // 출력으로 설정
  osDelay(1);
  cnt = driver_uart_send(cfg->uart_io, pData, dataLen);
  osDelay(1);
  driver_do_low(cfg->do_io);  // 입력으로 설정


  OS_POST_SEM(drv->sem);


  return cnt;
}

/**
 * @brief RS485 수신
 * @param timeOutms 타임아웃시간동안만 데이터 수신, 데이터가 계속 들어와도 무시하고 딱 정해진
 * 시간만수신
 * @retval -1에러,0 수신없음, 1이상 수신된 데이터 길이
 */
int32_t driver_rs485_recv(driver_t *drv, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutms)
{
  rs485_cfg_t *cfg = drv->cfg;
  int32_t len = 0;

  len = driver_uart_recv(cfg->uart_io, pBuff, rLen, timeOutms);

  return len;
}

/**
 * @brief 2가지 타임아웃 적용하여 수신
 * timeout1은 전체 수신 대기 시간
 * timeout2는 연속 바이트간 대기 시간
 * 예)어떠한 패킷을 수신하는데 데이터간 수신 시간은 거의 연속적이다
 * 예를 들어 끝을 결정하기 어려운 패킷이 있다면 효과적
 * timeout1 2초
 * timeout2 2ms
 * 명령어 보내고 2초간 기다리고 일단 그 안에 1바이트라도 수신되면 패킷이
 * 수신되기 시작 의미 그런데 2ms 안에 그다음 데이터가 수신안되면 종료로 판단
 * @details 
 * #test
 */
int32_t driver_rs485_recv_opt(driver_t *drv, uint8_t *buffer, uint16_t buffer_size,
                              uint32_t timeout1_ms, uint32_t timeout2_ms)
{
  rs485_cfg_t *cfg = drv->cfg;
  int32_t len = 0;

  len = driver_uart_recv_opt(cfg->uart_io, buffer, buffer_size, timeout1_ms, timeout2_ms);

  return len;
}

void driver_rs485_set(driver_t *drv, uart_set_option_t cmd, void *option)
{
  rs485_cfg_t *cfg = drv->cfg;

  switch (cmd)
  {
    case UART_SET_BAUDRATE:
      driver_uart_set(cfg->uart_io, cmd, option);
      break;
  }
}

void driver_rs485_get(driver_t *drv, uart_get_option_t cmd, void *value)
{
  rs485_cfg_t *cfg = drv->cfg;

  switch (cmd)
  {
    case UART_GET_CONFIG:
      driver_uart_get(cfg->uart_io, cmd, value);
      break;
  }
}

void driver_rs485_close(driver_t *drv) {}

void driver_rs485_flush_rx(driver_t *drv)
{
  rs485_cfg_t *cfg = drv->cfg;

  driver_uart_flush_rx(cfg->uart_io);
}