/**
 * @file test_mcu_port.c
 * @brief STM32F407IG MCU GPIO 상태 및 구성 모니터링
 * @details 모든 GPIO 포트(PA~PI)의 입력 상태, 방향, Pull-up/down, 모드 등을 출력합니다.
 */

#include "test_mcu_port.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug_io.h"
#include "user_heap.h"
#include "console_utile.h"
#include "console_define.h"
#include "cli_key_code.h"
#include "stm32f4xx_hal.h"

/* STM32F407IG GPIO 포트 정의 (GPIOA ~ GPIOI) */
#define GPIO_PORT_COUNT 9

/* VT100 터미널 제어 명령어 */
#define VT100_CLEAR_SCREEN    "\033[2J"      /* 화면 지우기 */
#define VT100_CURSOR_HOME     "\033[H"       /* 커서를 홈(0,0)으로 이동 */
#define VT100_HIDE_CURSOR     "\033[?25l"    /* 커서 숨기기 */
#define VT100_SHOW_CURSOR     "\033[?25h"    /* 커서 보이기 */
#define VT100_COLOR_RED       "\033[31m"     /* 빨간색 텍스트 */
#define VT100_COLOR_WHITE     "\033[37m"     /* 흰색 텍스트 */
#define VT100_COLOR_RESET     "\033[0m"      /* 색상 초기화 */

/* GPIO 포트 정보 구조체 */
typedef struct {
  GPIO_TypeDef *port;
  const char *port_name;
} gpio_port_info_t;

/* GPIO 핀 구성 정보 구조체 */
typedef struct {
  uint8_t pin_num;
  uint8_t state;        /* 0: LOW, 1: HIGH */
  uint32_t mode;        /* GPIO_MODE_INPUT, OUTPUT_PP, AF_PP, ANALOG */
  uint32_t pull;        /* GPIO_NOPULL, PULLUP, PULLDOWN */
  uint32_t speed;       /* GPIO_SPEED_FREQ_LOW ~ VERY_HIGH */
  uint32_t alternate;   /* AF0 ~ AF15 (주변장치 기능) */
} gpio_pin_config_t;

/* GPIO 포트 테이블 */
static const gpio_port_info_t s_gpio_ports[GPIO_PORT_COUNT] = {
  {GPIOA, "PA"},
  {GPIOB, "PB"},
  {GPIOC, "PC"},
  {GPIOD, "PD"},
  {GPIOE, "PE"},
  {GPIOF, "PF"},
  {GPIOG, "PG"},
  {GPIOH, "PH"},
  {GPIOI, "PI"}
};

/**
 * @brief GPIO 모드 문자열 변환
 */
static const char* get_mode_string(uint32_t mode)
{
  switch (mode)
  {
    case GPIO_MODE_INPUT:                   return "INPUT  ";
    case GPIO_MODE_OUTPUT_PP:               return "OUT_PP ";
    case GPIO_MODE_OUTPUT_OD:               return "OUT_OD ";
    case GPIO_MODE_AF_PP:                   return "AF_PP  ";
    case GPIO_MODE_AF_OD:                   return "AF_OD  ";
    case GPIO_MODE_ANALOG:                  return "ANALOG ";
    case GPIO_MODE_IT_RISING:               return "IT_RISE";
    case GPIO_MODE_IT_FALLING:              return "IT_FALL";
    case GPIO_MODE_IT_RISING_FALLING:       return "IT_BOTH";
    case GPIO_MODE_EVT_RISING:              return "EV_RISE";
    case GPIO_MODE_EVT_FALLING:             return "EV_FALL";
    case GPIO_MODE_EVT_RISING_FALLING:      return "EV_BOTH";
    default:                                return "UNKNOWN";
  }
}

/**
 * @brief GPIO Pull 상태 문자열 변환
 */
static const char* get_pull_string(uint32_t pull)
{
  switch (pull)
  {
    case GPIO_NOPULL:   return "NOPULL";
    case GPIO_PULLUP:   return "PULLUP";
    case GPIO_PULLDOWN: return "PULLDN";
    default:            return "UNKNWN";
  }
}

/**
 * @brief Alternate Function 상세 설명 가져오기
 * @param af_num AF 번호 (0~15)
 * @param buffer 설명 문자열 저장 버퍼
 * @param buffer_size 버퍼 크기
 */
static void get_af_description(uint32_t af_num, char *buffer, size_t buffer_size)
{
  switch (af_num)
  {
    case 0:
      snprintf(buffer, buffer_size, "RTC_50Hz, MCO, TAMPER, SWJ(SWD/JTAG), TRACE");
      break;
    case 1:
      snprintf(buffer, buffer_size, "TIM1, TIM2");
      break;
    case 2:
      snprintf(buffer, buffer_size, "TIM3, TIM4, TIM5");
      break;
    case 3:
      snprintf(buffer, buffer_size, "TIM8, TIM9, TIM10, TIM11");
      break;
    case 4:
      snprintf(buffer, buffer_size, "I2C1, I2C2, I2C3");
      break;
    case 5:
      snprintf(buffer, buffer_size, "SPI1, SPI2/I2S2, I2S3ext");
      break;
    case 6:
      snprintf(buffer, buffer_size, "SPI3/I2S3, I2S2ext");
      break;
    case 7:
      snprintf(buffer, buffer_size, "USART1, USART2, USART3, I2S3ext");
      break;
    case 8:
      snprintf(buffer, buffer_size, "UART4, UART5, USART6");
      break;
    case 9:
      snprintf(buffer, buffer_size, "CAN1, CAN2, TIM12, TIM13, TIM14");
      break;
    case 10:
      snprintf(buffer, buffer_size, "OTG_FS, OTG_HS (USB)");
      break;
    case 11:
      snprintf(buffer, buffer_size, "ETH (Ethernet)");
      break;
    case 12:
      snprintf(buffer, buffer_size, "FSMC, OTG_HS_FS, SDIO");
      break;
    case 13:
      snprintf(buffer, buffer_size, "DCMI (Camera Interface)");
      break;
    case 14:
      snprintf(buffer, buffer_size, "Reserved");
      break;
    case 15:
      snprintf(buffer, buffer_size, "EVENTOUT");
      break;
    default:
      snprintf(buffer, buffer_size, "Unknown");
      break;
  }
}

/**
 * @brief GPIO 속도 문자열 변환
 */
static const char* get_speed_string(uint32_t speed)
{
  switch (speed)
  {
    case GPIO_SPEED_FREQ_LOW:       return "LOW ";
    case GPIO_SPEED_FREQ_MEDIUM:    return "MED ";
    case GPIO_SPEED_FREQ_HIGH:      return "HIGH";
    case GPIO_SPEED_FREQ_VERY_HIGH: return "VHGH";
    default:                        return "UNKN";
  }
}

/**
 * @brief GPIO 핀 구성 정보 읽기
 * @param port GPIO 포트
 * @param pin_num 핀 번호 (0~15)
 * @param config 구성 정보 저장 구조체
 */
static void read_gpio_pin_config(GPIO_TypeDef *port, uint8_t pin_num, gpio_pin_config_t *config)
{
  uint32_t position = pin_num * 2;
  uint32_t pin_mask = (1 << pin_num);

  config->pin_num = pin_num;

  /* 현재 상태 읽기 */
  GPIO_PinState state = HAL_GPIO_ReadPin(port, pin_mask);
  config->state = (state == GPIO_PIN_SET) ? 1 : 0;

  /* 모드 읽기 (MODER 레지스터) */
  config->mode = (port->MODER >> position) & 0x03;

  /* Pull-up/Pull-down 읽기 (PUPDR 레지스터) */
  config->pull = (port->PUPDR >> position) & 0x03;

  /* 속도 읽기 (OSPEEDR 레지스터) */
  config->speed = (port->OSPEEDR >> position) & 0x03;

  /* Alternate Function 읽기 (AFR 레지스터) */
  if (pin_num < 8)
  {
    config->alternate = (port->AFR[0] >> (pin_num * 4)) & 0x0F;
  }
  else
  {
    config->alternate = (port->AFR[1] >> ((pin_num - 8) * 4)) & 0x0F;
  }
}

/**
 * @brief GPIO 포트 상태 테이블 출력 (간략 버전)
 */
static void print_gpio_simple_table_header(void)
{
  int pin;

  debug_printf("\n");
  debug_printf("STM32F407IG GPIO 입력 상태 (간략)\n");
  debug_printf("================================================================================\n");
  debug_printf("포트 | ");

  for (pin = 0; pin < 16; pin++)
  {
    debug_printf("%2d ", pin);
  }

  debug_printf("|\n");
  debug_printf("-----|");

  for (pin = 0; pin < 16; pin++)
  {
    debug_printf("---");
  }

  debug_printf("|\n");
}

/**
 * @brief GPIO 포트 상태 테이블 출력 (한 행)
 */
static void print_gpio_simple_port_row(const char *port_name, GPIO_TypeDef *port)
{
  int pin;
  uint8_t state;

  debug_printf(" %s  | ", port_name);

  for (pin = 0; pin < 16; pin++)
  {
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(port, (1 << pin));
    state = (pin_state == GPIO_PIN_SET) ? 1 : 0;
    debug_printf(" %d ", state);
  }

  debug_printf("|\n");
}

/**
 * @brief GPIO 포트 상태 테이블 출력 (푸터)
 */
static void print_gpio_table_footer(void)
{
  int pin;

  debug_printf("-----|");

  for (pin = 0; pin < 16; pin++)
  {
    debug_printf("---");
  }

  debug_printf("|\n");
  debug_printf("================================================================================\n");
}

/**
 * @brief 모든 GPIO 포트 상태 출력 (간략 버전)
 */
static void display_all_gpio_states_simple(void)
{
  int port;

  /* 테이블 헤더 출력 */
  print_gpio_simple_table_header();

  /* 각 포트별 상태 출력 */
  for (port = 0; port < GPIO_PORT_COUNT; port++)
  {
    print_gpio_simple_port_row(s_gpio_ports[port].port_name, s_gpio_ports[port].port);
  }

  /* 테이블 푸터 출력 */
  print_gpio_table_footer();
}

/**
 * @brief 특정 GPIO 포트 상세 구성 정보 출력
 * @param port_index 포트 인덱스 (0: GPIOA, 1: GPIOB, ...)
 */
static void display_port_detailed_config(int port_index)
{
  gpio_pin_config_t config;
  int pin;
  uint32_t mode_type;
  char af_desc[80];
  char mode_detail[128];

  if (port_index < 0 || port_index >= GPIO_PORT_COUNT)
  {
    debug_printf("오류: 유효하지 않은 포트 인덱스입니다. (범위: 0 ~ %d)\n", GPIO_PORT_COUNT - 1);
    return;
  }

  debug_printf("\n");
  debug_printf("================================================================================\n");
  debug_printf("포트 %s 상세 구성 정보 (STM32F407IG)\n", s_gpio_ports[port_index].port_name);
  debug_printf("================================================================================\n");
  debug_printf("핀  | 상태 | 모드     | PULL   | 속도 | AF  | 모드 설명\n");
  debug_printf("----|------|----------|--------|------|-----|--------------------------------\n");

  for (pin = 0; pin < 16; pin++)
  {
    read_gpio_pin_config(s_gpio_ports[port_index].port, pin, &config);

    /* 모드 타입 (MODER 레지스터 값) */
    mode_type = config.mode;

    /* 모드 설명 생성 */
    switch (mode_type)
    {
      case 0x00:  /* INPUT */
        snprintf(mode_detail, sizeof(mode_detail), "입력");
        break;

      case 0x01:  /* OUTPUT */
        snprintf(mode_detail, sizeof(mode_detail), "출력");
        break;

      case 0x02:  /* ALTERNATE FUNCTION */
        snprintf(mode_detail, sizeof(mode_detail), "AF%d - 주변장치", config.alternate);
        break;

      case 0x03:  /* ANALOG */
        snprintf(mode_detail, sizeof(mode_detail), "아날로그");
        break;

      default:
        snprintf(mode_detail, sizeof(mode_detail), "알 수 없음");
        break;
    }

    debug_printf("%s%-2d |  %d   | %-8s | %-6s | %-4s | %-3d | %s\n",
                 s_gpio_ports[port_index].port_name,
                 pin,
                 config.state,
                 get_mode_string(mode_type),
                 get_pull_string(config.pull),
                 get_speed_string(config.speed),
                 config.alternate,
                 mode_detail);
  }

  debug_printf("================================================================================\n");

  /* AF 사용 핀에 대한 상세 설명 추가 */
  debug_printf("\n[AF(Alternate Function) 상세 정보]\n");
  debug_printf("--------------------------------------------------------------------------------\n");

  for (pin = 0; pin < 16; pin++)
  {
    read_gpio_pin_config(s_gpio_ports[port_index].port, pin, &config);

    /* AF 모드인 핀만 상세 정보 출력 */
    if (config.mode == 0x02)  /* ALTERNATE FUNCTION */
    {
      get_af_description(config.alternate, af_desc, sizeof(af_desc));
      debug_printf("%s%-2d : AF%-2d - %s\n",
                   s_gpio_ports[port_index].port_name,
                   pin,
                   config.alternate,
                   af_desc);
    }
  }

  debug_printf("--------------------------------------------------------------------------------\n");
  debug_printf("\n[STM32F407IG AF 기능 요약]\n");
  debug_printf("AF0  : RTC_50Hz, MCO, TAMPER, SWJ(SWD/JTAG), TRACE\n");
  debug_printf("AF1  : TIM1, TIM2 (타이머)\n");
  debug_printf("AF2  : TIM3, TIM4, TIM5 (타이머)\n");
  debug_printf("AF3  : TIM8, TIM9, TIM10, TIM11 (타이머)\n");
  debug_printf("AF4  : I2C1, I2C2, I2C3 (I2C 통신)\n");
  debug_printf("AF5  : SPI1, SPI2/I2S2, I2S3ext (SPI/I2S 통신)\n");
  debug_printf("AF6  : SPI3/I2S3, I2S2ext (SPI/I2S 통신)\n");
  debug_printf("AF7  : USART1, USART2, USART3, I2S3ext (UART 통신)\n");
  debug_printf("AF8  : UART4, UART5, USART6 (UART 통신)\n");
  debug_printf("AF9  : CAN1, CAN2, TIM12, TIM13, TIM14 (CAN 통신/타이머)\n");
  debug_printf("AF10 : OTG_FS, OTG_HS (USB)\n");
  debug_printf("AF11 : ETH (Ethernet 통신)\n");
  debug_printf("AF12 : FSMC, OTG_HS_FS, SDIO (메모리 인터페이스/USB/SD카드)\n");
  debug_printf("AF13 : DCMI (카메라 인터페이스)\n");
  debug_printf("AF14 : Reserved (예약됨)\n");
  debug_printf("AF15 : EVENTOUT (이벤트 출력)\n");
  debug_printf("================================================================================\n");
}

/**
 * @brief 모든 GPIO 포트 상세 구성 정보 출력 (요약)
 */
static void display_all_ports_summary(void)
{
  gpio_pin_config_t config;
  int port;
  int pin;
  int input_count;
  int output_count;
  int af_count;
  int analog_count;
  int pullup_count;
  int pulldown_count;

  debug_printf("\n");
  debug_printf("================================================================================\n");
  debug_printf("모든 GPIO 포트 구성 요약\n");
  debug_printf("================================================================================\n");
  debug_printf("포트 | 입력 | 출력 | AF | 아날로그 | PULLUP | PULLDOWN\n");
  debug_printf("-----|------|------|-------|----------|--------|----------\n");

  for (port = 0; port < GPIO_PORT_COUNT; port++)
  {
    input_count = 0;
    output_count = 0;
    af_count = 0;
    analog_count = 0;
    pullup_count = 0;
    pulldown_count = 0;

    for (pin = 0; pin < 16; pin++)
    {
      read_gpio_pin_config(s_gpio_ports[port].port, pin, &config);

      /* 모드 카운트 */
      switch (config.mode)
      {
        case 0x00: input_count++; break;
        case 0x01: output_count++; break;
        case 0x02: af_count++; break;
        case 0x03: analog_count++; break;
      }

      /* Pull 카운트 */
      switch (config.pull)
      {
        case GPIO_PULLUP: pullup_count++; break;
        case GPIO_PULLDOWN: pulldown_count++; break;
      }
    }

    debug_printf(" %s  |  %2d  |  %2d  |  %2d   |    %2d    |   %2d   |    %2d\n",
                 s_gpio_ports[port].port_name,
                 input_count,
                 output_count,
                 af_count,
                 analog_count,
                 pullup_count,
                 pulldown_count);
  }

  debug_printf("================================================================================\n");
}

/**
 * @brief GPIO 상태 변화 모니터링 (간략, VT100)
 * @param interval_ms 모니터링 주기 (ms)
 */
static void monitor_gpio_changes_simple(uint32_t interval_ms)
{
  uint8_t current_states[GPIO_PORT_COUNT][16];
  uint8_t prev_states[GPIO_PORT_COUNT][16];
  uint8_t changed_flags[GPIO_PORT_COUNT][16];
  int port;
  int pin;
  uint8_t state;

  /* 초기 상태 읽기 */
  for (port = 0; port < GPIO_PORT_COUNT; port++)
  {
    for (pin = 0; pin < 16; pin++)
    {
      GPIO_PinState pin_state = HAL_GPIO_ReadPin(s_gpio_ports[port].port, (1 << pin));
      prev_states[port][pin] = (pin_state == GPIO_PIN_SET) ? 1 : 0;
      changed_flags[port][pin] = 0;
    }
  }

  /* 화면 초기화 및 커서 숨기기 */
  debug_printf(VT100_CLEAR_SCREEN);
  debug_printf(VT100_CURSOR_HOME);
  debug_printf(VT100_HIDE_CURSOR);

  /* 모니터링 루프 */
  while (1)
  {
    /* 커서를 홈으로 이동하여 같은 위치에 다시 그리기 */
    debug_printf(VT100_CURSOR_HOME);

    /* 헤더 출력 */
    debug_printf("\n");
    debug_printf("STM32F407IG GPIO 상태 모니터링 (간략) - 주기: %ums\n", interval_ms);
    debug_printf("================================================================================\n");
    debug_printf("포트 | ");
    for (pin = 0; pin < 16; pin++)
    {
      debug_printf("%2d ", pin);
    }
    debug_printf("|\n");
    debug_printf("-----|");
    for (pin = 0; pin < 16; pin++)
    {
      debug_printf("---");
    }
    debug_printf("|\n");

    /* 모든 포트 상태 읽기 및 출력 */
    for (port = 0; port < GPIO_PORT_COUNT; port++)
    {
      debug_printf(" %s  | ", s_gpio_ports[port].port_name);

      for (pin = 0; pin < 16; pin++)
      {
        GPIO_PinState pin_state = HAL_GPIO_ReadPin(s_gpio_ports[port].port, (1 << pin));
        current_states[port][pin] = (pin_state == GPIO_PIN_SET) ? 1 : 0;
        state = current_states[port][pin];

        /* 변화 감지 */
        if (current_states[port][pin] != prev_states[port][pin])
        {
          changed_flags[port][pin] = 1;
          prev_states[port][pin] = current_states[port][pin];
        }

        /* 변경된 핀은 빨간색으로 표시 */
        if (changed_flags[port][pin])
        {
          debug_printf(VT100_COLOR_RED " %d " VT100_COLOR_RESET, state);
        }
        else
        {
          debug_printf(" %d ", state);
        }
      }

      debug_printf("|\n");
    }

    /* 푸터 출력 */
    debug_printf("-----|");
    for (pin = 0; pin < 16; pin++)
    {
      debug_printf("---");
    }
    debug_printf("|\n");
    debug_printf("================================================================================\n");
    debug_printf(VT100_COLOR_RED "빨간색" VT100_COLOR_RESET ": 변경된 핀 | CTRL+C: 종료\n");
    debug_printf("                                                                                \n");

    /* CTRL+C 체크 */
    if (get_key(interval_ms) == KEY_CODE_CTRL_C)
    {
      debug_printf(VT100_SHOW_CURSOR);
      debug_printf("\n\n모니터링 종료\n\n");
      break;
    }

    /* 변경 플래그 초기화 (다음 사이클에서는 흰색으로 표시) */
    for (port = 0; port < GPIO_PORT_COUNT; port++)
    {
      for (pin = 0; pin < 16; pin++)
      {
        changed_flags[port][pin] = 0;
      }
    }
  }
}

/**
 * @brief GPIO 상태 변화 모니터링 (상세 로그)
 * @param interval_ms 모니터링 주기 (ms)
 */
static void monitor_gpio_changes(uint32_t interval_ms)
{
  uint8_t current_states[GPIO_PORT_COUNT][16];
  uint8_t prev_states[GPIO_PORT_COUNT][16];
  int port;
  int pin;

  /* 초기 상태 읽기 */
  for (port = 0; port < GPIO_PORT_COUNT; port++)
  {
    for (pin = 0; pin < 16; pin++)
    {
      GPIO_PinState state = HAL_GPIO_ReadPin(s_gpio_ports[port].port, (1 << pin));
      prev_states[port][pin] = (state == GPIO_PIN_SET) ? 1 : 0;
    }
  }

  debug_printf("\nGPIO 상태 변화 모니터링 시작 (%ums 주기)\n", interval_ms);
  debug_printf("CTRL+C로 종료\n\n");

  /* 모니터링 루프 */
  while (1)
  {
    /* 모든 포트 상태 읽기 */
    for (port = 0; port < GPIO_PORT_COUNT; port++)
    {
      for (pin = 0; pin < 16; pin++)
      {
        GPIO_PinState state = HAL_GPIO_ReadPin(s_gpio_ports[port].port, (1 << pin));
        current_states[port][pin] = (state == GPIO_PIN_SET) ? 1 : 0;

        /* 변화 감지 */
        if (current_states[port][pin] != prev_states[port][pin])
        {
          debug_printf("[%s%d] %d → %d\n",
                       s_gpio_ports[port].port_name,
                       pin,
                       prev_states[port][pin],
                       current_states[port][pin]);

          prev_states[port][pin] = current_states[port][pin];
        }
      }
    }

    /* CTRL+C 체크 */
    if (get_key(interval_ms) == KEY_CODE_CTRL_C)
    {
      debug_printf("\n모니터링 종료\n\n");
      break;
    }
  }
}

/**
 * @brief MCU GPIO 포트 테스트 메인 함수
 */
void test_mcu_port(void)
{
  int choice;
  int port_index;
  int interval_ms;
  int ret;

  while (1)
  {
    debug_printf("\n");
    debug_printf("┌────────────────────────────────────────┐\n");
    debug_printf("│  STM32F407IG GPIO 구성 및 상태 모니터링│\n");
    debug_printf("├────────────────────────────────────────┤\n");
    debug_printf("│  1. 모든 GPIO 포트 상태 (간략)         │\n");
    debug_printf("│  2. 특정 포트 상세 구성 정보           │\n");
    debug_printf("│  3. 모든 포트 구성 요약                │\n");
    debug_printf("│  4. GPIO 상태 변화 모니터링            │\n");
    debug_printf("│  5. GPIO 상태 변화 모니터링 (간략)     │\n");
    debug_printf("│  0. 이전 메뉴                          │\n");
    debug_printf("├────────────────────────────────────────┤\n");
    debug_printf("│  포트: PA(0), PB(1), PC(2), PD(3)      │\n");
    debug_printf("│        PE(4), PF(5), PG(6), PH(7)      │\n");
    debug_printf("│        PI(8)                           │\n");
    debug_printf("└────────────────────────────────────────┘\n");

    ret = view_input_decimal("선택", &choice, 0, 5);

    if (ret == MENU_ABORT || ret == MENU_BACK || choice == 0)
    {
      debug_printf("\n이전 메뉴로 돌아갑니다.\n");
      return;
    }

    if (ret != MENU_OK)
    {
      continue;
    }

    switch (choice)
    {
      case 1:
        /* 모든 GPIO 포트 상태 표시 (간략) */
        display_all_gpio_states_simple();
        debug_printf("\n아무 키나 누르세요...");
        get_key(0xFFFFFFFF);
        break;

      case 2:
        /* 특정 포트 상세 구성 정보 */
        debug_printf("\n포트 선택:\n");
        debug_printf("0:PA, 1:PB, 2:PC, 3:PD, 4:PE, 5:PF, 6:PG, 7:PH, 8:PI\n");

        ret = view_input_decimal("포트 인덱스", &port_index, 0, GPIO_PORT_COUNT - 1);
        if (ret == MENU_OK)
        {
          display_port_detailed_config(port_index);
          debug_printf("\n아무 키나 누르세요...");
          get_key(0xFFFFFFFF);
        }
        break;

      case 3:
        /* 모든 포트 구성 요약 */
        display_all_ports_summary();
        debug_printf("\n아무 키나 누르세요...");
        get_key(0xFFFFFFFF);
        break;

      case 4:
        /* GPIO 상태 변화 모니터링 */
        ret = view_input_decimal("모니터링 주기(ms)", &interval_ms, 10, 10000);
        if (ret == MENU_OK)
        {
          monitor_gpio_changes(interval_ms);
        }
        break;
      case 5:
         ret = view_input_decimal("모니터링 주기(ms)", &interval_ms, 10, 10000);
        if (ret == MENU_OK)
        {
      monitor_gpio_changes_simple(interval_ms);
        }
        

      default:
        debug_printf("잘못된 선택입니다.\n");
        break;
    }
  }
}
