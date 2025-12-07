/**
 * @file test_sram.c
 * @brief SRAM 동적 메모리 할당 테스트 (user_malloc 사용)
 * @details test_fram.c 구조를 참고하여 user_malloc으로 메모리를 할당하고
 *          쓰기/읽기 테스트를 수행합니다.
 */

#include "test_sram.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug_io.h"
#include "user_heap.h"
#include "console_utile.h"
#include "console_define.h"
#include "cli_key_code.h"
#include "stm32f4xx_hal.h"

/* 메모리 청크 크기 */
#define SRAM_CHUNK_SIZE     256            /* 읽기/쓰기 단위 */
#define SRAM_MAX_ALLOC_SIZE (1024 * 1024)  /* 최대 할당 크기 1MB */

/* 메뉴 정의 */
#define MENU_SRAM_RW_TEST       1
#define MENU_SRAM_HEX_DUMP      2
#define MENU_SRAM_PATTERN_TEST  3
#define MENU_SRAM_EXIT          0

/* 함수 프로토타입 */
static void menu_sram_rw_test(void);
static void menu_sram_hex_dump(void);
static void menu_sram_pattern_test(void);
static void print_sram_hex_dump(uint8_t *buffer, uint32_t size, uint32_t offset);
static int32_t sram_write_and_verify(uint8_t *sram_ptr, uint32_t size, uint8_t pattern);

/**
 * @brief SRAM 읽기/쓰기 테스트 메뉴
 */
static void menu_sram_rw_test(void)
{
  char pattern_str[16];
  int size_kb;
  int status;
  int confirm;
  uint32_t size;
  uint8_t pattern;
  uint8_t *sram_ptr;

  debug_printf("\n");
  debug_printf("+---------------------------------------+\n");
  debug_printf("|      SRAM 읽기/쓰기 테스트            |\n");
  debug_printf("+---------------------------------------+\n");
  debug_printf("user_malloc을 사용한 동적 메모리 테스트\n");

  /* 크기 입력 (KB 단위) */
  status = view_input_decimal("할당 크기(KB)", &size_kb, 1, SRAM_MAX_ALLOC_SIZE / 1024);
  if (status == MENU_ABORT || status == MENU_BACK)
  {
    debug_printf("취소되었습니다.\n");
    return;
  }

  size = size_kb * 1024;

  /* 패턴 입력 */
  debug_printf("테스트 패턴 입력 (HEX, 예: AA 또는 55): ");
  status = debug_scanf_s("%s", pattern_str, sizeof(pattern_str));
  if (status == KEY_CODE_CTRL_C || status == KEY_CODE_CTRL_Q)
  {
    debug_printf("취소되었습니다.\n");
    return;
  }

  /* 16진수 문자열을 숫자로 변환 */
  pattern = (uint8_t)strtol(pattern_str, NULL, 16);

  debug_printf("\n========================================\n");
  debug_printf("테스트 설정:\n");
  debug_printf("========================================\n");
  debug_printf("할당 크기  : %u bytes (%u KB)\n", size, size_kb);
  debug_printf("패턴       : 0x%02X\n", pattern);

  /* 확인 */
  status = view_confirm_continue("테스트를 시작하시겠습니까?", &confirm);
  if (status != MENU_OK || confirm != 1)
  {
    debug_printf("취소되었습니다.\n");
    return;
  }

  /* 메모리 할당 */
  debug_printf("\n메모리 할당 중...\n");
  sram_ptr = (uint8_t *)user_malloc(size);
  if (sram_ptr == NULL)
  {
    debug_printf("메모리 할당 실패! (요청 크기: %u bytes)\n", size);
    return;
  }

  debug_printf("메모리 할당 성공! (주소: 0x%08X)\n", (uint32_t)sram_ptr);

  /* 쓰기 및 검증 */
  debug_printf("\nSRAM 쓰기/읽기 테스트 시작...\n\n");
  if (sram_write_and_verify(sram_ptr, size, pattern) == 0)
  {
    debug_printf("\n테스트 성공!\n");
  }
  else
  {
    debug_printf("\n테스트 실패!\n");
  }

  /* 메모리 해제 */
  user_free(sram_ptr);
  debug_printf("\n메모리 해제 완료\n");

  debug_printf("\n아무 키나 누르세요...");
  get_key(0xFFFFFFFF);
}

/**
 * @brief SRAM HEX DUMP 메뉴
 */
static void menu_sram_hex_dump(void)
{
  int size_kb;
  int dump_size_bytes;
  int status;
  int key;
  uint32_t size;
  uint32_t dump_size;
  uint32_t offset;
  uint8_t *sram_ptr;

  debug_printf("\n");
  debug_printf("+---------------------------------------+\n");
  debug_printf("|        SRAM HEX DUMP                  |\n");
  debug_printf("+---------------------------------------+\n");

  /* 할당 크기 입력 (KB 단위) */
  status = view_input_decimal("할당 크기(KB)", &size_kb, 1, SRAM_MAX_ALLOC_SIZE / 1024);
  if (status == MENU_ABORT || status == MENU_BACK)
  {
    debug_printf("취소되었습니다.\n");
    return;
  }

  size = size_kb * 1024;

  /* 덤프 크기 입력 (바이트 단위) */
  status = view_input_decimal("덤프 크기(bytes)", &dump_size_bytes, 1, 2048);
  if (status == MENU_ABORT || status == MENU_BACK)
  {
    debug_printf("취소되었습니다.\n");
    return;
  }

  dump_size = dump_size_bytes;

  if (dump_size > size)
  {
    debug_printf("경고: 덤프 크기가 할당 크기보다 큽니다. 할당 크기로 제한합니다.\n");
    dump_size = size;
  }

  /* 메모리 할당 */
  debug_printf("\n메모리 할당 중...\n");
  sram_ptr = (uint8_t *)user_malloc(size);
  if (sram_ptr == NULL)
  {
    debug_printf("메모리 할당 실패!\n");
    return;
  }

  debug_printf("메모리 할당 성공! (주소: 0x%08X)\n", (uint32_t)sram_ptr);

  /* 테스트 데이터 초기화 (주소 기반 패턴) */
  debug_printf("테스트 데이터 초기화 중 (주소 기반 패턴)...\n");
  for (offset = 0; offset < size; offset++)
  {
    sram_ptr[offset] = (uint8_t)(offset % 256);
  }

  debug_printf("\n========================================\n");
  debug_printf("SRAM HEX DUMP\n");
  debug_printf("주소: 0x%08X, 크기: %u bytes\n", (uint32_t)sram_ptr, dump_size);
  debug_printf("========================================\n\n");

  /* HEX DUMP 출력 */
  offset = 0;
  while (offset < dump_size)
  {
    uint32_t chunk_size = ((dump_size - offset) > SRAM_CHUNK_SIZE) ?
                          SRAM_CHUNK_SIZE : (dump_size - offset);

    /* HEX DUMP 출력 */
    print_sram_hex_dump(&sram_ptr[offset], chunk_size, offset);

    offset += chunk_size;

    /* 256바이트마다 일시정지 */
    if ((offset % 256 == 0) && (offset < dump_size))
    {
      debug_printf("\n계속하려면 아무 키나 누르세요 (Ctrl+C: 중단)...");
      key = get_key(0xFFFFFFFF);
      if (key == KEY_CODE_CTRL_C)
      {
        debug_printf("\n중단되었습니다.\n");
        break;
      }
      debug_printf("\n\n");
    }
  }

  user_free(sram_ptr);
  debug_printf("\n메모리 해제 완료\n");

  debug_printf("\nHEX DUMP 완료!\n");
  debug_printf("아무 키나 누르세요...");
  get_key(0xFFFFFFFF);
}

/**
 * @brief SRAM 다중 패턴 테스트 메뉴
 */
static void menu_sram_pattern_test(void)
{
  int size_kb;
  int status;
  int confirm;
  uint32_t i;
  uint32_t size;
  uint8_t *sram_ptr;
  uint8_t patterns[] = {0x00, 0xFF, 0xAA, 0x55, 0xCC, 0x33};
  int pattern_count = sizeof(patterns) / sizeof(patterns[0]);

  debug_printf("\n");
  debug_printf("+---------------------------------------+\n");
  debug_printf("|     SRAM 다중 패턴 테스트             |\n");
  debug_printf("+---------------------------------------+\n");

  /* 크기 입력 (KB 단위) */
  status = view_input_decimal("할당 크기(KB)", &size_kb, 1, SRAM_MAX_ALLOC_SIZE / 1024);
  if (status == MENU_ABORT || status == MENU_BACK)
  {
    debug_printf("취소되었습니다.\n");
    return;
  }

  size = size_kb * 1024;

  debug_printf("\n========================================\n");
  debug_printf("테스트 설정:\n");
  debug_printf("========================================\n");
  debug_printf("할당 크기  : %u bytes (%u KB)\n", size, size_kb);
  debug_printf("테스트 패턴: 0x00, 0xFF, 0xAA, 0x55, 0xCC, 0x33\n");

  /* 확인 */
  status = view_confirm_continue("테스트를 시작하시겠습니까?", &confirm);
  if (status != MENU_OK || confirm != 1)
  {
    debug_printf("취소되었습니다.\n");
    return;
  }

  /* 메모리 할당 */
  debug_printf("\n메모리 할당 중...\n");
  sram_ptr = (uint8_t *)user_malloc(size);
  if (sram_ptr == NULL)
  {
    debug_printf("메모리 할당 실패!\n");
    return;
  }

  debug_printf("메모리 할당 성공! (주소: 0x%08X)\n", (uint32_t)sram_ptr);

  /* 각 패턴별로 테스트 */
  debug_printf("\n다중 패턴 테스트 시작...\n\n");
  for (i = 0; i < pattern_count; i++)
  {
    debug_printf("========================================\n");
    debug_printf("패턴 %u/%u: 0x%02X\n", i + 1, pattern_count, patterns[i]);
    debug_printf("========================================\n");

    if (sram_write_and_verify(sram_ptr, size, patterns[i]) != 0)
    {
      debug_printf("\n패턴 0x%02X 테스트 실패!\n", patterns[i]);
      user_free(sram_ptr);
      debug_printf("\n아무 키나 누르세요...");
      get_key(0xFFFFFFFF);
      return;
    }

    debug_printf("패턴 0x%02X 테스트 성공!\n\n", patterns[i]);
  }

  /* 메모리 해제 */
  user_free(sram_ptr);
  debug_printf("\n메모리 해제 완료\n");

  debug_printf("\n========================================\n");
  debug_printf("모든 패턴 테스트 성공!\n");
  debug_printf("========================================\n");

  debug_printf("\n아무 키나 누르세요...");
  get_key(0xFFFFFFFF);
}

/**
 * @brief SRAM HEX DUMP 출력
 * @param buffer 버퍼 포인터
 * @param size 버퍼 크기
 * @param offset 오프셋 주소
 */
static void print_sram_hex_dump(uint8_t *buffer, uint32_t size, uint32_t offset)
{
  char ascii_buf[17];
  int j;
  uint32_t i;

  for (i = 0; i < size; i += 16)
  {
    /* 오프셋 출력 */
    debug_printf("%08X: ", offset + i);

    /* HEX 출력 */
    for (j = 0; j < 16; j++)
    {
      if (i + j < size)
      {
        debug_printf("%02X ", buffer[i + j]);
        /* ASCII 버퍼에 저장 */
        if (buffer[i + j] >= 32 && buffer[i + j] <= 126)
        {
          ascii_buf[j] = buffer[i + j];
        }
        else
        {
          ascii_buf[j] = '.';
        }
      }
      else
      {
        debug_printf("   ");
        ascii_buf[j] = ' ';
      }

      /* 8바이트마다 공백 추가 */
      if (j == 7)
      {
        debug_printf(" ");
      }
    }

    /* ASCII 출력 */
    ascii_buf[16] = '\0';
    debug_printf(" |%s|\n", ascii_buf);
  }
}

/**
 * @brief SRAM 쓰기 및 검증
 * @param sram_ptr SRAM 시작 주소
 * @param size 크기
 * @param pattern 패턴 값
 * @return 0: 성공, -1: 실패
 */
static int32_t sram_write_and_verify(uint8_t *sram_ptr, uint32_t size, uint8_t pattern)
{
  uint32_t i;
  uint32_t startClk;
  uint32_t endClk;
  uint32_t elapsed;
  uint32_t error_count;
  uint32_t progress;
  uint32_t prev_progress;

  /* ==================== SRAM 쓰기 ==================== */
  debug_printf("\n[1/2] SRAM 쓰기 중...\n");
  debug_printf("주소: 0x%08X, 크기: %u bytes, 패턴: 0x%02X\n",
               (uint32_t)sram_ptr, size, pattern);

  startClk = HAL_GetTick();

  /* 패턴 쓰기 */
  for (i = 0; i < size; i++)
  {
    sram_ptr[i] = pattern;

    /* 진행률 표시 (10% 단위) */
    progress = (i * 100) / size;
    prev_progress = ((i > 0 ? i - 1 : 0) * 100) / size;
    if ((progress / 10) != (prev_progress / 10))
    {
      debug_printf("  진행률: %u%%\n", progress);
    }
  }

  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  debug_printf("쓰기 완료: %u bytes, %u ms (%.2f KB/s)\n",
               size, elapsed,
               (size / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  /* ==================== 데이터 검증 ==================== */
  debug_printf("\n[2/2] 데이터 검증 중...\n");

  startClk = HAL_GetTick();
  error_count = 0;

  /* 데이터 비교 */
  for (i = 0; i < size; i++)
  {
    if (sram_ptr[i] != pattern)
    {
      if (error_count < 10)  /* 처음 10개만 출력 */
      {
        debug_printf("데이터 불일치! 주소: 0x%08X, 예상: 0x%02X, 실제: 0x%02X\n",
                     (uint32_t)&sram_ptr[i], pattern, sram_ptr[i]);
      }
      error_count++;
    }

    /* 진행률 표시 (10% 단위) */
    progress = (i * 100) / size;
    prev_progress = ((i > 0 ? i - 1 : 0) * 100) / size;
    if ((progress / 10) != (prev_progress / 10))
    {
      debug_printf("  진행률: %u%%\n", progress);
    }
  }

  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  debug_printf("검증 완료: %u bytes, %u ms (%.2f KB/s)\n",
               size, elapsed,
               (size / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  if (error_count > 0)
  {
    debug_printf("\n검증 실패! 총 %u개 오류 발견\n", error_count);
    return -1;
  }

  debug_printf("검증 성공! 모든 데이터가 일치합니다.\n");
  return 0;
}

/**
 * @brief SRAM 테스트 메인 메뉴
 */
void test_sram(void)
{
  int choice;
  int status;

  while (1)
  {
    debug_printf("\n");
    debug_printf("+=======================================+\n");
    debug_printf("|         SRAM 테스트 메뉴              |\n");
    debug_printf("+=======================================+\n");
    debug_printf("| 1. SRAM 읽기/쓰기 테스트              |\n");
    debug_printf("| 2. SRAM HEX DUMP                      |\n");
    debug_printf("| 3. SRAM 다중 패턴 테스트              |\n");
    debug_printf("| 0. 이전 메뉴                          |\n");
    debug_printf("|                                       |\n");
    debug_printf("|    CTRL+C: 이전, CTRL+Q: 종료         |\n");
    debug_printf("+=======================================+\n");
    debug_printf("SRAM: user_malloc 동적 메모리 할당\n");

    status = view_input_decimal("선택", &choice, 0, 3);

    if (status == MENU_ABORT)
    {
      debug_printf("\n프로그램을 종료합니다.\n");
      return;
    }
    else if (status == MENU_BACK || choice == MENU_SRAM_EXIT)
    {
      debug_printf("\n이전 메뉴로 돌아갑니다.\n");
      return;
    }
    else if (status != MENU_OK)
    {
      continue;
    }

    switch (choice)
    {
      case MENU_SRAM_RW_TEST:
        menu_sram_rw_test();
        break;

      case MENU_SRAM_HEX_DUMP:
        menu_sram_hex_dump();
        break;

      case MENU_SRAM_PATTERN_TEST:
        menu_sram_pattern_test();
        break;

      default:
        debug_printf("잘못된 선택입니다.\n");
        break;
    }
  }
}
