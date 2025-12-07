/**
 * @file test_do.c
 * @brief 디지털 출력(DO) 테스트 (BSP_DO 사용)
 * @details BSP_DO_EXT_0 ~ BSP_DO_EXT_5 출력 포트를 사용자 입력으로 제어합니다.
 */

#include "test_do.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug_io.h"
#include "console_utile.h"
#include "console_define.h"
#include "cli_key_code.h"
#include "stm32f4xx_hal.h"

#include "bsp_do.h"

/* DO 포트 개수 */
#define DO_PORT_COUNT 6

/* DO 포트 정보 구조체 */
typedef struct {
  uint8_t port_num;
  const char *port_name;
} do_port_info_t;

/* DO 포트 정보 테이블 */
static const do_port_info_t s_do_ports[DO_PORT_COUNT] = {
  {BSP_DO_EXT_0, "DO_0"},
  {BSP_DO_EXT_1, "DO_1"},
  {BSP_DO_EXT_2, "DO_2"},
  {BSP_DO_EXT_3, "DO_3"},
  {BSP_DO_EXT_4, "DO_4"},
  {BSP_DO_EXT_5, "DO_5"}
};

/**
 * @brief 포트 번호 유효성 검사
 * @param port_num 포트 번호 (0~5)
 * @return 0: 유효, -1: 무효
 */
static int validate_port_number(int port_num)
{
  if (port_num < 0 || port_num >= DO_PORT_COUNT)
  {
    debug_printf("오류: 유효하지 않은 포트 번호입니다. (범위: 0 ~ %d)\n", DO_PORT_COUNT - 1);
    return -1;
  }
  return 0;
}

/**
 * @brief 상태 문자열 파싱
 * @param state_str 상태 문자열 (low, high, 0, 1)
 * @return 1: HIGH, 0: LOW, -1: 무효
 */
static int parse_state_string(const char *state_str)
{
  if (strcasecmp(state_str, "high") == 0 || strcmp(state_str, "1") == 0)
  {
    return 1;  /* HIGH */
  }
  else if (strcasecmp(state_str, "low") == 0 || strcmp(state_str, "0") == 0)
  {
    return 0;  /* LOW */
  }
  else
  {
    debug_printf("오류: 유효하지 않은 상태입니다. (허용: low, high, 0, 1)\n");
    return -1;
  }
}

/**
 * @brief 현재 설정 상태 표시 (시뮬레이션)
 * @details BSP_DO는 읽기 기능이 없으므로 내부 상태 배열로 추적
 */
static void display_current_states(const uint8_t *states)
{
  int i;

  debug_printf("\n현재 DO 상태:\n");
  debug_printf("+------+----------+--------+\n");
  debug_printf("| 포트 | 이름     | 상태   |\n");
  debug_printf("+------+----------+--------+\n");

  for (i = 0; i < DO_PORT_COUNT; i++)
  {
    debug_printf("|  %d   | %-8s | %-6s |\n",
                 i,
                 s_do_ports[i].port_name,
                 (states[i] == 1) ? "HIGH" : "LOW");
  }

  debug_printf("+------+----------+--------+\n\n");
}

/**
 * @brief 모든 DO 포트 초기화 (LOW로 설정)
 */
static void initialize_all_do_ports(uint8_t *states)
{
  int i;

  debug_printf("모든 DO 포트를 LOW로 초기화 중...\n");

  for (i = 0; i < DO_PORT_COUNT; i++)
  {
    bsp_do_low(s_do_ports[i].port_num);
    states[i] = 0;  /* LOW */
  }

  debug_printf("초기화 완료!\n\n");
}

/**
 * @brief 디지털 출력(DO) 테스트 메인 함수
 */
void test_do(void)
{
  char input_buffer[32];
  char state_str[16];
  int port_num;
  int state;
  int ret;
  uint8_t do_states[DO_PORT_COUNT] = {0};  /* 각 포트의 현재 상태 (0: LOW, 1: HIGH) */

  debug_printf("\n");
  debug_printf("┌────────────────────────────────────────┐\n");
  debug_printf("│  디지털 출력(DO) 제어 테스트           │\n");
  debug_printf("├────────────────────────────────────────┤\n");
  debug_printf("│  테스트 포트:                          │\n");
  debug_printf("│  - DO_0 (BSP_DO_EXT_0)                 │\n");
  debug_printf("│  - DO_1 (BSP_DO_EXT_1)                 │\n");
  debug_printf("│  - DO_2 (BSP_DO_EXT_2)                 │\n");
  debug_printf("│  - DO_3 (BSP_DO_EXT_3)                 │\n");
  debug_printf("│  - DO_4 (BSP_DO_EXT_4)                 │\n");
  debug_printf("│  - DO_5 (BSP_DO_EXT_5)                 │\n");
  debug_printf("├────────────────────────────────────────┤\n");
  debug_printf("│  입력 형식:                            │\n");
  debug_printf("│  - <포트번호>,<상태>                   │\n");
  debug_printf("│  - 예: 0,high  또는  3,low             │\n");
  debug_printf("│  - 상태: low, high, 0, 1               │\n");
  debug_printf("├────────────────────────────────────────┤\n");
  debug_printf("│  특수 명령:                            │\n");
  debug_printf("│  - all,low   : 모든 포트 LOW           │\n");
  debug_printf("│  - all,high  : 모든 포트 HIGH          │\n");
  debug_printf("│  - status    : 현재 상태 표시          │\n");
  debug_printf("│  - CTRL+C    : 종료                    │\n");
  debug_printf("└────────────────────────────────────────┘\n");
  debug_printf("\n");

  /* 모든 DO 포트 초기화 */
  initialize_all_do_ports(do_states);

  /* 초기 상태 표시 */
  display_current_states(do_states);

  /* 메인 루프: 사용자 입력 처리 */
  while (1)
  {
    debug_printf("명령 입력 (포트,상태) > ");

    /* 사용자 입력 받기 */
    ret = debug_scanf_s("%31s", input_buffer, sizeof(input_buffer));

    /* CTRL+C 체크 */
    if (ret == KEY_CODE_CTRL_C)
    {
      debug_printf("\n\n테스트 종료\n");
      break;
    }

    /* 입력 오류 체크 */
    if (ret < 1)
    {
      debug_printf("입력 오류. 다시 시도하세요.\n\n");
      continue;
    }

    /* "status" 명령어 처리 */
    if (strcasecmp(input_buffer, "status") == 0)
    {
      display_current_states(do_states);
      continue;
    }

    /* 콤마로 분리된 입력 파싱 */
    if (sscanf(input_buffer, "%d,%15s", &port_num, state_str) == 2)
    {
      /* 포트 번호 유효성 검사 */
      if (validate_port_number(port_num) < 0)
      {
        debug_printf("\n");
        continue;
      }

      /* 상태 문자열 파싱 */
      state = parse_state_string(state_str);
      if (state < 0)
      {
        debug_printf("\n");
        continue;
      }

      /* DO 포트 제어 */
      if (state == 1)
      {
        bsp_do_high(s_do_ports[port_num].port_num);
        do_states[port_num] = 1;
        debug_printf("[%s] 출력: HIGH\n\n", s_do_ports[port_num].port_name);
      }
      else
      {
        bsp_do_low(s_do_ports[port_num].port_num);
        do_states[port_num] = 0;
        debug_printf("[%s] 출력: LOW\n\n", s_do_ports[port_num].port_name);
      }
    }
    /* "all,<상태>" 명령어 처리 */
    else if (sscanf(input_buffer, "all,%15s", state_str) == 1)
    {
      int i;

      /* 상태 문자열 파싱 */
      state = parse_state_string(state_str);
      if (state < 0)
      {
        debug_printf("\n");
        continue;
      }

      /* 모든 DO 포트 제어 */
      debug_printf("모든 포트를 %s로 설정 중...\n", (state == 1) ? "HIGH" : "LOW");

      for (i = 0; i < DO_PORT_COUNT; i++)
      {
        if (state == 1)
        {
          bsp_do_high(s_do_ports[i].port_num);
          do_states[i] = 1;
        }
        else
        {
          bsp_do_low(s_do_ports[i].port_num);
          do_states[i] = 0;
        }
      }

      debug_printf("완료!\n\n");
      display_current_states(do_states);
    }
    else
    {
      debug_printf("잘못된 입력 형식입니다.\n");
      debug_printf("형식: <포트번호>,<상태> (예: 0,high 또는 all,low)\n\n");
    }
  }

  /* 종료 시 모든 포트 LOW로 설정 */
  debug_printf("\n종료 중: 모든 DO 포트를 LOW로 설정...\n");
  initialize_all_do_ports(do_states);

  debug_printf("테스트 종료 완료\n\n");
}
