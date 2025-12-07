/**
 * @file test_di.c
 * @brief 디지털 입력(DI) 테스트 (BSP_DI 사용)
 * @details BSP_DI_0 ~ BSP_DI_5 입력 포트의 상태를 모니터링하고
 *          상태 변경 시 포트 정보와 상태를 출력합니다.
 */

#include "test_di.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug_io.h"
#include "console_utile.h"
#include "console_define.h"
#include "cli_key_code.h"
#include "stm32f4xx_hal.h"

#include "bsp_di.h"

/* DI 포트 개수 */
#define DI_PORT_COUNT 6

/* DI 포트 정보 구조체 */
typedef struct {
  uint8_t port_num;
  const char *port_name;
} di_port_info_t;

/* DI 포트 정보 테이블 */
static const di_port_info_t s_di_ports[DI_PORT_COUNT] = {
  {BSP_DI_0, "DI_0"},
  {BSP_DI_1, "DI_1"},
  {BSP_DI_2, "DI_2"},
  {BSP_DI_3, "DI_3"},
  {BSP_DI_4, "DI_4"},
  {BSP_DI_5, "DI_5"}
};

/**
 * @brief 디지털 입력(DI) 테스트 메인 함수
 */
void test_di(void)
{
  int i;
  int32_t current_state;
  int32_t prev_state[DI_PORT_COUNT];

  debug_printf("\n");
  debug_printf("┌────────────────────────────────────────┐\n");
  debug_printf("│  디지털 입력(DI) 상태 모니터링 테스트  │\n");
  debug_printf("├────────────────────────────────────────┤\n");
  debug_printf("│  테스트 포트:                          │\n");
  debug_printf("│  - DI_0 (BSP_DI_0)                     │\n");
  debug_printf("│  - DI_1 (BSP_DI_1)                     │\n");
  debug_printf("│  - DI_2 (BSP_DI_2)                     │\n");
  debug_printf("│  - DI_3 (BSP_DI_3)                     │\n");
  debug_printf("│  - DI_4 (BSP_DI_4)                     │\n");
  debug_printf("│  - DI_5 (BSP_DI_5)                     │\n");
  debug_printf("├────────────────────────────────────────┤\n");
  debug_printf("│  동작:                                 │\n");
  debug_printf("│  - 100ms 주기로 상태 확인              │\n");
  debug_printf("│  - 상태 변경 시 포트명과 상태 출력    │\n");
  debug_printf("│  - CTRL+C로 종료                       │\n");
  debug_printf("└────────────────────────────────────────┘\n");
  debug_printf("\n");

  /* 이전 상태 초기화 (-1: 초기값) */
  for (i = 0; i < DI_PORT_COUNT; i++)
  {
    prev_state[i] = -1;
  }

  debug_printf("모니터링 시작...\n\n");

  /* 초기 상태 출력 */
  debug_printf("초기 상태:\n");
  for (i = 0; i < DI_PORT_COUNT; i++)
  {
    current_state = bsp_di_read(s_di_ports[i].port_num);
    if (current_state >= 0)
    {
      debug_printf("  %s: %s\n",
                   s_di_ports[i].port_name,
                   (current_state == 1) ? "HIGH" : "LOW");
      prev_state[i] = current_state;
    }
    else
    {
      debug_printf("  %s: 읽기 오류 (코드: %d)\n",
                   s_di_ports[i].port_name, current_state);
    }
  }

  debug_printf("\n상태 변경 모니터링 중... (CTRL+C로 종료)\n\n");

  /* 메인 루프: 상태 모니터링 */
  while (1)
  {
    /* 모든 DI 포트 상태 확인 */
    for (i = 0; i < DI_PORT_COUNT; i++)
    {
      current_state = bsp_di_read(s_di_ports[i].port_num);

      /* 상태 읽기 성공 */
      if (current_state >= 0)
      {
        /* 상태 변경 감지 */
        if (current_state != prev_state[i])
        {
          debug_printf("[%s] 상태 변경: %s → %s\n",
                       s_di_ports[i].port_name,
                       (prev_state[i] == 1) ? "HIGH" :
                       (prev_state[i] == 0) ? "LOW" : "UNKNOWN",
                       (current_state == 1) ? "HIGH" : "LOW");
          prev_state[i] = current_state;
        }
      }
      /* 상태 읽기 오류 */
      else
      {
        if (prev_state[i] >= 0)  /* 이전에 정상이었다면 오류 출력 */
        {
          debug_printf("[%s] 읽기 오류 발생 (코드: %d)\n",
                       s_di_ports[i].port_name, current_state);
          prev_state[i] = -1;
        }
      }
    }

    /* CTRL+C 체크 */
    if (get_key(100) == KEY_CODE_CTRL_C)
    {
      debug_printf("\n\n테스트 종료\n");
      break;
    }
  }

  /* 최종 상태 출력 */
  debug_printf("\n최종 상태:\n");
  for (i = 0; i < DI_PORT_COUNT; i++)
  {
    current_state = bsp_di_read(s_di_ports[i].port_num);
    if (current_state >= 0)
    {
      debug_printf("  %s: %s\n",
                   s_di_ports[i].port_name,
                   (current_state == 1) ? "HIGH" : "LOW");
    }
    else
    {
      debug_printf("  %s: 읽기 오류\n", s_di_ports[i].port_name);
    }
  }

  debug_printf("\n");
}
