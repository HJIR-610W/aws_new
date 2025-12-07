/**
 * @file test_uart.c
 * @brief UART 다중 포트 동시 송수신 테스트
 * @details 8개 UART 포트에 대해 각각 독립적인 태스크를 생성하여
 *          동시 송수신 동작을 테스트합니다.
 */

#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"

#include "test_uart.h"
#include "bsp_uart.h"
#include "drv_uart_def.h"
#include "debug_io.h"
#include "cli_key_code.h"
#include "console_utile.h"

/* 테스트할 UART 포트 정의 */
#define TEST_UART_PORT_COUNT 8

/* UART 포트 설정 구조체 */
typedef struct {
  uint8_t port_num;
  const char *port_name;
} uart_port_info_t;

/* UART 태스크 파라미터 구조체 */
typedef struct {
  uint8_t port_num;
  const char *port_name;
  uart_config_t config;
} uart_task_param_t;

/* 테스트 포트 정보 테이블 */
static const uart_port_info_t s_uart_ports[TEST_UART_PORT_COUNT] = {
  {BSP_UART_0_D_SUB_0,      "VHF"},
  {BSP_UART_2_EXT_A,        "USER0"},
  {BSP_UART_3_EXT_B,        "USER1"},
  {BSP_UART_4_EXT_C,        "USER2"},
  {BSP_UART_5_EXT_D,        "USER3"},
  {BSP_UART_6_RS485_A_ONLY, "RS485_A"},
  {BSP_UART_7_RS485_B_ONLY, "RS485_B"},
  {BSP_UART_8_CDMA,         "CDMA"}
};

/* UART 태스크 핸들 배열 */
static osThreadId_t s_uart_task_handles[TEST_UART_PORT_COUNT] = {NULL};

/* UART 태스크 파라미터 배열 (전역 변수로 유지) */
static uart_task_param_t s_uart_task_params[TEST_UART_PORT_COUNT];

/* 테스트 종료 플래그 */
static volatile uint8_t s_test_stop_flag = 0;

/**
 * @brief UART 테스트 태스크
 * @param argument uart_task_param_t 포인터
 */
static void uart_test_task(void *argument)
{
  char tx_buff[64];
  char rx_buff[128];
  int len;
  int rx_index = 0;
  uart_task_param_t *param = (uart_task_param_t *)argument;

  /* UART 초기화 */
  if (bsp_uart_init(param->port_num, &param->config) < 0)
  {
    debug_printf("[%s] UART 초기화 실패\r\n", param->port_name);
    osThreadExit();
  }

  debug_printf("[%s] UART 테스트 시작 (Baud: %d)\r\n",
               param->port_name, param->config.baud);

  /* 메인 루프 */
  while (!s_test_stop_flag)
  {
    /* 1초마다 포트 이름 전송 */
    snprintf(tx_buff, sizeof(tx_buff), "[%s] Test\r\n", param->port_name);
    bsp_uart_send(param->port_num, (uint8_t *)tx_buff, strlen(tx_buff));

    debug_printf("[%s] TX: %s", param->port_name, tx_buff);

    /* 수신 데이터 처리 (0x0A 까지 수신) */
    while (!s_test_stop_flag && rx_index < (int)(sizeof(rx_buff) - 1))
    {
      len = bsp_uart_recv(param->port_num, (uint8_t *)&rx_buff[rx_index], 1, 100);

      if (len > 0)
      {
        if (rx_buff[rx_index] == 0x0A) /* LF 수신 */
        {
          rx_index++;
          rx_buff[rx_index] = '\0';

          /* 수신 데이터 에코 */
          bsp_uart_send(param->port_num, (uint8_t *)rx_buff, rx_index);

          debug_printf("[%s] RX & ECHO (%d bytes): ", param->port_name, rx_index);
          for (int i = 0; i < rx_index; i++)
          {
            debug_printf("%02X ", (uint8_t)rx_buff[i]);
          }
          debug_printf("\r\n");

          rx_index = 0; /* 버퍼 리셋 */
          break; /* LF 수신 후 1초 대기로 복귀 */
        }
        else
        {
          rx_index++;
        }
      }
      else
      {
        break; /* 타임아웃 시 1초 대기로 복귀 */
      }
    }

    /* 버퍼 오버플로우 방지 */
    if (rx_index >= (int)(sizeof(rx_buff) - 1))
    {
      debug_printf("[%s] RX 버퍼 오버플로우, 리셋\r\n", param->port_name);
      rx_index = 0;
    }

    /* 1초 대기 */
    osDelay(1000);
  }

  /* UART 종료 */
  bsp_uart_deinit(param->port_num);
  debug_printf("[%s] UART 테스트 종료\r\n", param->port_name);

  /* 태스크 종료 */
  osThreadExit();
}

/**
 * @brief 모든 UART 테스트 태스크 생성
 * @param config UART 설정 구조체
 * @return 0: 성공, -1: 실패
 */
static int create_all_uart_tasks(const uart_config_t *config)
{
  char task_name[32];
  int i;
  osThreadAttr_t task_attr;

  for (i = 0; i < TEST_UART_PORT_COUNT; i++)
  {
    /* 태스크 파라미터 설정 */
    s_uart_task_params[i].port_num = s_uart_ports[i].port_num;
    s_uart_task_params[i].port_name = s_uart_ports[i].port_name;
    memcpy(&s_uart_task_params[i].config, config, sizeof(uart_config_t));

    /* 태스크 속성 설정 */
    snprintf(task_name, sizeof(task_name), "uart_%s", s_uart_ports[i].port_name);

    memset(&task_attr,0,sizeof(osThreadAttr_t));
    task_attr.name = task_name;
    task_attr.stack_size = 1024; /* 최소 스택 크기 */
    task_attr.priority = osPriorityNormal;

    /* 태스크 생성 */
    s_uart_task_handles[i] = osThreadNew(uart_test_task,
                                          &s_uart_task_params[i],
                                          &task_attr);

    if (s_uart_task_handles[i] == NULL)
    {
      debug_printf("UART 태스크 생성 실패: %s\r\n", s_uart_ports[i].port_name);
      return -1;
    }
  }

  return 0;
}

/**
 * @brief 모든 UART 테스트 태스크 종료
 */
static void terminate_all_uart_tasks(void)
{
  int i;
  int max_wait_count = 30; /* 최대 3초 대기 (30 * 100ms) */
  int wait_count = 0;

  /* 종료 플래그 설정 */
  s_test_stop_flag = 1;

  /* 모든 태스크가 osThreadExit()으로 자연스럽게 종료될 때까지 대기 */
  while (wait_count < max_wait_count)
  {
    int all_terminated = 1;

    for (i = 0; i < TEST_UART_PORT_COUNT; i++)
    {
      if (s_uart_task_handles[i] != NULL)
      {
        osThreadState_t state = osThreadGetState(s_uart_task_handles[i]);
        if (state != osThreadTerminated && state != osThreadInactive)
        {
          all_terminated = 0;
          break;
        }
      }
    }

    if (all_terminated)
    {
      debug_printf("모든 태스크 정상 종료 완료\r\n");
      break;
    }

    osDelay(100);
    wait_count++;
  }

  /* 타임아웃 시 강제 종료 */
  if (wait_count >= max_wait_count)
  {
    debug_printf("태스크 종료 타임아웃, 강제 종료 시도\r\n");
    for (i = 0; i < TEST_UART_PORT_COUNT; i++)
    {
      if (s_uart_task_handles[i] != NULL)
      {
        osThreadTerminate(s_uart_task_handles[i]);
      }
    }
  }

  /* 태스크 핸들 초기화 */
  for (i = 0; i < TEST_UART_PORT_COUNT; i++)
  {
    s_uart_task_handles[i] = NULL;
  }
}

/**
 * @brief UART 멀티포트 동시 테스트 메인 함수
 */
void test_uart(void)
{
  uart_config_t uart_config;
  int baud = 9600;

  debug_printf("\r\n");
  debug_printf("┌────────────────────────────────────────┐\r\n");
  debug_printf("│  UART 다중 포트 동시 송수신 테스트     │\r\n");
  debug_printf("├────────────────────────────────────────┤\r\n");
  debug_printf("│  테스트 포트:                          │\r\n");
  debug_printf("│  - VHF (BSP_UART_0)                    │\r\n");
  debug_printf("│  - USER0 (BSP_UART_2)                  │\r\n");
  debug_printf("│  - USER1 (BSP_UART_3)                  │\r\n");
  debug_printf("│  - USER2 (BSP_UART_4)                  │\r\n");
  debug_printf("│  - USER3 (BSP_UART_5)                  │\r\n");
  debug_printf("│  - RS485_A (BSP_UART_6)                │\r\n");
  debug_printf("│  - RS485_B (BSP_UART_7)                │\r\n");
  debug_printf("│  - CDMA (BSP_UART_8)                   │\r\n");
  debug_printf("├────────────────────────────────────────┤\r\n");
  debug_printf("│  동작:                                 │\r\n");
  debug_printf("│  1. 각 포트마다 독립 태스크 생성       │\r\n");
  debug_printf("│  2. 1초마다 포트명 송신                │\r\n");
  debug_printf("│  3. 0x0A(LF) 수신 시 데이터 에코       │\r\n");
  debug_printf("│  4. CTRL+C로 종료                      │\r\n");
  debug_printf("└────────────────────────────────────────┘\r\n");
  debug_printf("\r\n");

  /* 통신 속도 입력 */
  if (view_input_decimal("통신 속도를 입력하세요", &baud, 1200, 115200) != MENU_OK)
  {
    baud = 9600;
    debug_printf("기본 속도 %d로 설정합니다.\r\n", baud);
  }

  /* UART 설정 */
  uart_config.baud = baud;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;

  /* 종료 플래그 초기화 */
  s_test_stop_flag = 0;

  /* 모든 UART 태스크 생성 */
  if (create_all_uart_tasks(&uart_config) < 0)
  {
    debug_printf("UART 태스크 생성 실패\r\n");
    return;
  }

  debug_printf("\r\n모든 UART 태스크 시작됨 (CTRL+C로 종료)\r\n\r\n");

  /* CTRL+C 입력 대기 */
  while (1)
  {
    if (get_key(100) == KEY_CODE_CTRL_C)
    {
      debug_printf("\r\n\r\n테스트 종료 요청...\r\n");
      break;
    }
  }

  /* 모든 태스크 종료 */
  terminate_all_uart_tasks();

  debug_printf("UART 멀티포트 테스트 완료\r\n\r\n");
}

/**
 * @brief 대안 제안: 폴링 기반 단일 태스크 UART 테스트
 * @details 태스크 생성 없이 단일 루프에서 모든 포트를 순차적으로 처리
 */
void test_uart_polling(void)
{
  uart_config_t uart_config;
  char tx_buff[64];
  char rx_buff[8][128];
  int rx_index[8] = {0};
  int len;
  int baud = 9600;
  int i;
  uint32_t tick_1sec = 0;

  debug_printf("\r\n");
  debug_printf("┌────────────────────────────────────────┐\r\n");
  debug_printf("│  UART 폴링 기반 테스트 (단일 태스크)   │\r\n");
  debug_printf("└────────────────────────────────────────┘\r\n");
  debug_printf("\r\n");

  /* 통신 속도 입력 */
  if (view_input_decimal("통신 속도를 입력하세요", &baud, 1200, 115200) != MENU_OK)
  {
    baud = 9600;
    debug_printf("기본 속도 %d로 설정합니다.\r\n", baud);
  }

  /* UART 설정 */
  uart_config.baud = baud;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;
  uart_config.dataLen = UART_DATA_LEN_8;

  /* 모든 UART 초기화 */
  for (i = 0; i < TEST_UART_PORT_COUNT; i++)
  {
    if (bsp_uart_init(s_uart_ports[i].port_num, &uart_config) < 0)
    {
      debug_printf("[%s] UART 초기화 실패\r\n", s_uart_ports[i].port_name);
    }
    else
    {
      debug_printf("[%s] UART 초기화 완료\r\n", s_uart_ports[i].port_name);
    }
  }

  debug_printf("\r\n테스트 시작 (CTRL+C로 종료)\r\n\r\n");

  tick_1sec = osKernelGetTickCount();

  /* 메인 루프 */
  while (1)
  {
    /* 1초마다 모든 포트에 데이터 전송 */
    if ((osKernelGetTickCount() - tick_1sec) >= 1000)
    {
      tick_1sec = osKernelGetTickCount();

      for (i = 0; i < TEST_UART_PORT_COUNT; i++)
      {
        snprintf(tx_buff, sizeof(tx_buff), "[%s] Test\r\n", s_uart_ports[i].port_name);
        bsp_uart_send(s_uart_ports[i].port_num, (uint8_t *)tx_buff, strlen(tx_buff));
        debug_printf("[%s] TX\r\n", s_uart_ports[i].port_name);
      }
    }

    /* 모든 포트 수신 데이터 폴링 */
    for (i = 0; i < TEST_UART_PORT_COUNT; i++)
    {
      len = bsp_uart_recv(s_uart_ports[i].port_num,
                          (uint8_t *)&rx_buff[i][rx_index[i]],
                          1,
                          1); /* 1ms 타임아웃 */

      if (len > 0)
      {
        if (rx_buff[i][rx_index[i]] == 0x0A) /* LF 수신 */
        {
          rx_index[i]++;
          rx_buff[i][rx_index[i]] = '\0';

          /* 수신 데이터 에코 */
          bsp_uart_send(s_uart_ports[i].port_num, (uint8_t *)rx_buff[i], rx_index[i]);

          debug_printf("[%s] RX & ECHO (%d bytes)\r\n",
                       s_uart_ports[i].port_name, rx_index[i]);

          rx_index[i] = 0; /* 버퍼 리셋 */
        }
        else
        {
          rx_index[i]++;

          /* 버퍼 오버플로우 방지 */
          if (rx_index[i] >= (int)(sizeof(rx_buff[i]) - 1))
          {
            debug_printf("[%s] RX 버퍼 오버플로우\r\n", s_uart_ports[i].port_name);
            rx_index[i] = 0;
          }
        }
      }
    }

    /* CTRL+C 체크 */
    if (get_key(1) == KEY_CODE_CTRL_C)
    {
      debug_printf("\r\n\r\n테스트 종료 요청...\r\n");
      break;
    }
  }

  /* 모든 UART 종료 */
  for (i = 0; i < TEST_UART_PORT_COUNT; i++)
  {
    bsp_uart_deinit(s_uart_ports[i].port_num);
  }

  debug_printf("UART 폴링 테스트 완료\r\n\r\n");
}
