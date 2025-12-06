
#include <stdbool.h>

#include "bsp_rs485.h"
#include "bsp_uart.h"
#include "bsp_do.h"
#include "os_user_def.h"
#include "pcb_define.h"
#include "debug_io.h"
#include "bsp_delay.h"
typedef struct rs
{
  int32_t uart_number;
  int dir_do_num;
  bool opened;
  void *sem;
} rs485_instance_t;

rs485_instance_t rs485_instance[BSP_RS485_MAX] = {

    [BSP_RS485_C] = {.uart_number = BSP_UART_6_RS485_A_ONLY, .dir_do_num = BSP_DO_DIR_RS485_A},
    [BSP_RS485_D] = {.uart_number = BSP_UART_7_RS485_B_ONLY, .dir_do_num = BSP_DO_DIR_RS485_B},
    [BSP_RS485_RS232_A] = {.uart_number = BSP_UART_2_EXT_A, .dir_do_num = BSP_DO_DIR_RS485_C},
    [BSP_RS485_RS232_B] = {.uart_number = BSP_UART_3_EXT_B, .dir_do_num = BSP_DO_DIR_RS485_D},
};

int32_t bsp_rs485_init(int32_t num,void *opt)
{

    if (rs485_instance[num].opened)
      return 1;
    bsp_do_init();
    bsp_do_low(rs485_instance[num].dir_do_num); // 수신 모드
    bsp_uart_init(rs485_instance[num].uart_number, opt);
    OS_CREATE_BINARY_SEM(rs485_instance[num].sem);
    rs485_instance[num].opened = true;



    return (rs485_instance[num].opened?1:0);
}


void safe_us_delay(uint32_t us_delay)
{
  uint32_t start;
  uint32_t delay;
  start = mcu_get_clk();
  osDelay(1);
  delay = cal_elapsed_us(start);

  if (delay < us_delay)
  {
    osDelay(1);
  }
}
/**
 * @brief RS485 데이터 전송
 * @param drv
 * @param pData
 * @param dataLen
 * @retval -1 전송 오류, 0>= 전송된 데이터 수
 */
int32_t bsp_rs485_send(int num,const uint8_t *pData, size_t dataLen)
{

  int32_t cnt = 0;

 
      
      
  OS_PEND_SEM(rs485_instance[num].sem, osWaitForever);

  // TODO:이 드라이버를 호출하는 task보다 우선높은곳이 있으면 osDelay 1이상 지연됨됨
  bsp_do_high(rs485_instance [num].dir_do_num); // 출력으로 설정
  safe_us_delay(100);
  //osDelay(1);//osDelay는 1틱 기준이기때문에 
  cnt = bsp_uart_send(rs485_instance[num].uart_number, pData, dataLen);
 // osDelay(1);
  safe_us_delay(100);
  bsp_do_low(rs485_instance[num].dir_do_num); // 입력으로 설정


  OS_POST_SEM(rs485_instance[num].sem);

  return cnt;
}

/**
 * @brief RS485 수신
 * @param timeOutms 타임아웃시간동안만 데이터 수신, 데이터가 계속 들어와도 무시하고 딱 정해진
 * 시간만수신
 * @retval -1에러,0 수신없음, 1이상 수신된 데이터 길이
 */
int32_t bsp_rs485_recv(int num, uint8_t *pBuff, size_t rLen, uint32_t timeOutms)
{
   int32_t len = 0;

  len = bsp_uart_recv(rs485_instance[num].uart_number, pBuff, rLen, timeOutms);

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
int32_t bsp_rs485_recv_opt(int num, uint8_t *buffer, size_t buffer_size, uint32_t timeout1_ms, uint32_t timeout2_ms)
{
  int32_t len = 0;

  len = bsp_uart_recv_opt(rs485_instance[num].uart_number, buffer, buffer_size, timeout1_ms, timeout2_ms);

  return len;
}

void bsp_rs485_set(int num, eUART_SET_OPTION_t cmd, void *option)
{
   bsp_uart_set(rs485_instance[num].uart_number, cmd, option);

}

void bsp_rs485_get(int num, eUART_GET_OPTION_t cmd, void *value)
{
  bsp_uart_get(rs485_instance[num].uart_number, cmd, value);
}



void bsp_rs485_flush_rx(int num)
{
  bsp_uart_flush_rx(rs485_instance[num].uart_number);
}
