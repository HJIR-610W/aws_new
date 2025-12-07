
#include "test_flash.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug_io.h"
#include "drv_flash.h"
#include "user_heap.h"
#include "console_utile.h"
#include "console_define.h"
#include "cli_key_code.h"
#include "stm32f4xx_hal.h"

/* Flash 용량 정의 */
#define FLASH_CAPACITY      (8 * 1024 * 1024)  /* 8MB */
#define FLASH_CHUNK_SIZE    512                 /* 최적 읽기/쓰기 크기 */

/* 메뉴 정의 */
#define MENU_FLASH_RW_TEST      1
#define MENU_FLASH_HEX_DUMP     2
#define MENU_FLASH_ERASE_CHECK  3
#define MENU_FLASH_EXIT         0

/* 함수 프로토타입 */
static void menu_flash_rw_test(void);
static void menu_flash_hex_dump(void);
static void menu_flash_erase_check(void);
static void print_flash_hex_dump(uint8_t *buffer, uint32_t size, uint32_t base_addr);
static int32_t flash_write_and_verify(uint32_t start_addr, uint32_t size, uint8_t pattern);

/**
 * @brief Flash 읽기/쓰기 테스트 메뉴
 */
static void menu_flash_rw_test(void)
{
    char pattern_str[16];
    int start_addr_mb;
    int size_kb;
    int status;
    uint32_t start_addr;
    uint32_t size;
    uint8_t pattern;

    debug_printf("\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("|      Flash 읽기/쓰기 테스트           |\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("Flash 용량: 8 MB (0x00000000 ~ 0x007FFFFF)\n");

    /* 시작 주소 입력 (MB 단위) */
    status = view_input_decimal("시작 주소(MB)", &start_addr_mb, 0, 7);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    start_addr = start_addr_mb * 1024 * 1024;

    /* 크기 입력 (KB 단위) */
    status = view_input_decimal("테스트 크기(KB)", &size_kb, 1, 8192);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    size = size_kb * 1024;

    /* 주소 범위 검증 */
    if ((start_addr + size) > FLASH_CAPACITY)
    {
        debug_printf("오류: 테스트 범위가 Flash 용량을 초과합니다!\n");
        debug_printf("시작: 0x%08X, 크기: %u bytes, 끝: 0x%08X\n",
                     start_addr, size, start_addr + size);
        debug_printf("최대 주소: 0x%08X\n", FLASH_CAPACITY - 1);
        return;
    }

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
    debug_printf("시작 주소: 0x%08X (%u MB)\n", start_addr, start_addr_mb);
    debug_printf("크기     : %u bytes (%u KB)\n", size, size_kb);
    debug_printf("끝 주소  : 0x%08X\n", start_addr + size - 1);
    debug_printf("패턴     : 0x%02X\n", pattern);

    /* 확인 */
    int confirm;
    status = view_confirm_continue("테스트를 시작하시겠습니까?", &confirm);
    if (status != MENU_OK || confirm != 1)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    /* 쓰기 및 검증 */
    debug_printf("\nFlash 쓰기/읽기 테스트 시작...\n\n");
    if (flash_write_and_verify(start_addr, size, pattern) == 0)
    {
        debug_printf("\n테스트 성공!\n");
    }
    else
    {
        debug_printf("\n테스트 실패!\n");
    }

    debug_printf("\n아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief Flash HEX DUMP 메뉴
 */
static void menu_flash_hex_dump(void)
{
    int start_addr_hex;
    int size_bytes;
    int status;
    uint32_t start_addr;
    uint32_t size;
    uint32_t current_addr;
    uint8_t *buffer;

    debug_printf("\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("|        Flash HEX DUMP                 |\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("Flash 용량: 8 MB (0x00000000 ~ 0x007FFFFF)\n");

    /* 시작 주소 입력 (16진수) */
    debug_printf("\n시작 주소 입력 (HEX, 예: 100000): 0x");
    char addr_str[16];
    status = debug_scanf_s("%s", addr_str, sizeof(addr_str));
    if (status == KEY_CODE_CTRL_C || status == KEY_CODE_CTRL_Q)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    start_addr = (uint32_t)strtol(addr_str, NULL, 16);

    /* 크기 입력 (바이트 단위) */
    status = view_input_decimal("읽을 크기(bytes)", &size_bytes, 1, 4096);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    size = size_bytes;

    /* 주소 범위 검증 */
    if ((start_addr + size) > FLASH_CAPACITY)
    {
        debug_printf("오류: 범위가 Flash 용량을 초과합니다!\n");
        return;
    }

    /* 버퍼 할당 */
    buffer = (uint8_t *)user_malloc(FLASH_CHUNK_SIZE);
    if (buffer == NULL)
    {
        debug_printf("메모리 할당 실패!\n");
        return;
    }

    debug_printf("\n========================================\n");
    debug_printf("Flash HEX DUMP\n");
    debug_printf("주소: 0x%08X, 크기: %u bytes\n", start_addr, size);
    debug_printf("========================================\n\n");

    /* 청크 단위로 읽기 및 출력 */
    current_addr = start_addr;
    while (current_addr < (start_addr + size))
    {
        uint32_t chunk_size = ((start_addr + size - current_addr) > FLASH_CHUNK_SIZE) ?
                              FLASH_CHUNK_SIZE : (start_addr + size - current_addr);

        /* Flash 읽기 */
        drv_flash_read(current_addr, buffer, chunk_size);

        /* HEX DUMP 출력 */
        print_flash_hex_dump(buffer, chunk_size, current_addr);

        current_addr += chunk_size;

        /* 512바이트마다 일시정지 */
        if ((current_addr - start_addr) % 512 == 0 && current_addr < (start_addr + size))
        {
            debug_printf("\n계속하려면 아무 키나 누르세요 (Ctrl+C: 중단)...");
            int key = get_key(0xFFFFFFFF);
            if (key == KEY_CODE_CTRL_C)
            {
                debug_printf("\n중단되었습니다.\n");
                break;
            }
            debug_printf("\n\n");
        }
    }

    user_free(buffer);

    debug_printf("\nHEX DUMP 완료!\n");
    debug_printf("아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief Flash 지우기 확인 메뉴 (0xFF 체크)
 */
static void menu_flash_erase_check(void)
{
    int start_addr_mb;
    int size_kb;
    int status;
    uint32_t start_addr;
    uint32_t size;
    uint32_t current_addr;
    uint32_t error_count;
    uint8_t *buffer;

    debug_printf("\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("|     Flash 지우기 확인 (0xFF 체크)     |\n");
    debug_printf("+---------------------------------------+\n");

    /* 시작 주소 입력 (MB 단위) */
    status = view_input_decimal("시작 주소(MB)", &start_addr_mb, 0, 7);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    start_addr = start_addr_mb * 1024 * 1024;

    /* 크기 입력 (KB 단위) */
    status = view_input_decimal("확인 크기(KB)", &size_kb, 1, 8192);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    size = size_kb * 1024;

    /* 주소 범위 검증 */
    if ((start_addr + size) > FLASH_CAPACITY)
    {
        debug_printf("오류: 범위가 Flash 용량을 초과합니다!\n");
        return;
    }

    /* 버퍼 할당 */
    buffer = (uint8_t *)user_malloc(FLASH_CHUNK_SIZE);
    if (buffer == NULL)
    {
        debug_printf("메모리 할당 실패!\n");
        return;
    }

    debug_printf("\n지우기 확인 시작...\n");
    debug_printf("주소: 0x%08X ~ 0x%08X (%u KB)\n\n",
                 start_addr, start_addr + size - 1, size_kb);

    error_count = 0;
    current_addr = start_addr;

    while (current_addr < (start_addr + size))
    {
        uint32_t chunk_size = ((start_addr + size - current_addr) > FLASH_CHUNK_SIZE) ?
                              FLASH_CHUNK_SIZE : (start_addr + size - current_addr);

        /* Flash 읽기 */
        drv_flash_read(current_addr, buffer, chunk_size);

        /* 0xFF 체크 */
        for (uint32_t i = 0; i < chunk_size; i++)
        {
            if (buffer[i] != 0xFF)
            {
                if (error_count < 10)  /* 처음 10개만 출력 */
                {
                    debug_printf("0xFF가 아닌 데이터 발견: 주소 0x%08X, 값 0x%02X\n",
                                 current_addr + i, buffer[i]);
                }
                error_count++;
            }
        }

        current_addr += chunk_size;

        /* 진행률 표시 (10% 단위) */
        uint32_t progress = ((current_addr - start_addr) * 100) / size;
        uint32_t prev_progress = ((current_addr - start_addr - chunk_size) * 100) / size;
        if ((progress / 10) != (prev_progress / 10))
        {
            debug_printf("진행률: %u%%\n", progress);
        }
    }

    user_free(buffer);

    debug_printf("\n========================================\n");
    debug_printf("확인 완료!\n");
    debug_printf("========================================\n");
    debug_printf("확인 영역  : 0x%08X ~ 0x%08X\n", start_addr, start_addr + size - 1);
    debug_printf("총 크기    : %u bytes (%u KB)\n", size, size_kb);
    debug_printf("오류 개수  : %u\n", error_count);

    if (error_count == 0)
    {
        debug_printf("\n결과: 모든 영역이 0xFF로 지워져 있습니다.\n");
    }
    else
    {
        debug_printf("\n결과: 지워지지 않은 데이터가 %u개 발견되었습니다.\n", error_count);
    }

    debug_printf("\n아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief Flash HEX DUMP 출력
 * @param buffer 버퍼 포인터
 * @param size 버퍼 크기
 * @param base_addr 기준 주소
 */
static void print_flash_hex_dump(uint8_t *buffer, uint32_t size, uint32_t base_addr)
{
    char ascii_buf[17];

    for (uint32_t i = 0; i < size; i += 16)
    {
        /* 주소 출력 */
        debug_printf("%08X: ", base_addr + i);

        /* HEX 출력 */
        for (int j = 0; j < 16; j++)
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
 * @brief Flash 쓰기 및 검증
 * @param start_addr 시작 주소
 * @param size 크기
 * @param pattern 패턴 값
 * @return 0: 성공, -1: 실패
 */
static int32_t flash_write_and_verify(uint32_t start_addr, uint32_t size, uint8_t pattern)
{
    uint8_t *write_buffer;
    uint8_t *read_buffer;
    uint32_t current_addr;
    uint32_t startClk;
    uint32_t endClk;
    uint32_t elapsed;
    int32_t err;

    /* 버퍼 할당 */
    write_buffer = (uint8_t *)user_malloc(FLASH_CHUNK_SIZE);
    if (write_buffer == NULL)
    {
        debug_printf("쓰기 버퍼 할당 실패!\n");
        return -1;
    }

    read_buffer = (uint8_t *)user_malloc(FLASH_CHUNK_SIZE);
    if (read_buffer == NULL)
    {
        debug_printf("읽기 버퍼 할당 실패!\n");
        user_free(write_buffer);
        return -1;
    }

    /* 쓰기 버퍼 초기화 */
    memset(write_buffer, pattern, FLASH_CHUNK_SIZE);

    /* ==================== Flash 쓰기 ==================== */
    debug_printf("\n[1/3] Flash 쓰기 중...\n");
    debug_printf("주소: 0x%08X, 크기: %u bytes, 패턴: 0x%02X\n",
                 start_addr, size, pattern);

    startClk = HAL_GetTick();
    current_addr = start_addr;

    while (current_addr < (start_addr + size))
    {
        uint32_t chunk_size = ((start_addr + size - current_addr) > FLASH_CHUNK_SIZE) ?
                              FLASH_CHUNK_SIZE : (start_addr + size - current_addr);

        err = drv_flash_write(current_addr, write_buffer, chunk_size);
        if (err < 0)
        {
            debug_printf("쓰기 오류! 주소: 0x%08X (Error: %d)\n", current_addr, err);
            user_free(write_buffer);
            user_free(read_buffer);
            return -1;
        }

        current_addr += chunk_size;

        /* 진행률 표시 (10% 단위) */
        uint32_t progress = ((current_addr - start_addr) * 100) / size;
        uint32_t prev_progress = ((current_addr - start_addr - chunk_size) * 100) / size;
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

    /* ==================== Flash 읽기 ==================== */
    debug_printf("\n[2/3] Flash 읽기 중...\n");

    startClk = HAL_GetTick();
    current_addr = start_addr;

    while (current_addr < (start_addr + size))
    {
        uint32_t chunk_size = ((start_addr + size - current_addr) > FLASH_CHUNK_SIZE) ?
                              FLASH_CHUNK_SIZE : (start_addr + size - current_addr);

        drv_flash_read(current_addr, read_buffer, chunk_size);
        current_addr += chunk_size;

        /* 진행률 표시 (10% 단위) */
        uint32_t progress = ((current_addr - start_addr) * 100) / size;
        uint32_t prev_progress = ((current_addr - start_addr - chunk_size) * 100) / size;
        if ((progress / 10) != (prev_progress / 10))
        {
            debug_printf("  진행률: %u%%\n", progress);
        }
    }

    endClk = HAL_GetTick();
    elapsed = endClk - startClk;

    debug_printf("읽기 완료: %u bytes, %u ms (%.2f KB/s)\n",
                 size, elapsed,
                 (size / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

    /* ==================== 데이터 검증 ==================== */
    debug_printf("\n[3/3] 데이터 검증 중...\n");

    current_addr = start_addr;
    uint32_t error_count = 0;

    while (current_addr < (start_addr + size))
    {
        uint32_t chunk_size = ((start_addr + size - current_addr) > FLASH_CHUNK_SIZE) ?
                              FLASH_CHUNK_SIZE : (start_addr + size - current_addr);

        drv_flash_read(current_addr, read_buffer, chunk_size);

        /* 데이터 비교 */
        for (uint32_t i = 0; i < chunk_size; i++)
        {
            if (read_buffer[i] != pattern)
            {
                if (error_count < 10)  /* 처음 10개만 출력 */
                {
                    debug_printf("데이터 불일치! 주소: 0x%08X, 예상: 0x%02X, 실제: 0x%02X\n",
                                 current_addr + i, pattern, read_buffer[i]);
                }
                error_count++;
            }
        }

        current_addr += chunk_size;

        /* 진행률 표시 (10% 단위) */
        uint32_t progress = ((current_addr - start_addr) * 100) / size;
        uint32_t prev_progress = ((current_addr - start_addr - chunk_size) * 100) / size;
        if ((progress / 10) != (prev_progress / 10))
        {
            debug_printf("  진행률: %u%%\n", progress);
        }
    }

    user_free(write_buffer);
    user_free(read_buffer);

    if (error_count > 0)
    {
        debug_printf("\n검증 실패! 총 %u개 오류 발견\n", error_count);
        return -1;
    }

    debug_printf("검증 성공! 모든 데이터가 일치합니다.\n");
    return 0;
}

/**
 * @brief Flash 테스트 메인 메뉴
 */
void test_flash(void)
{
    int choice;
    int status;

    while (1)
    {
        debug_printf("\n");
        debug_printf("+=======================================+\n");
        debug_printf("|         Flash 테스트 메뉴             |\n");
        debug_printf("+=======================================+\n");
        debug_printf("| 1. Flash 읽기/쓰기 테스트             |\n");
        debug_printf("| 2. Flash HEX DUMP                     |\n");
        debug_printf("| 3. Flash 지우기 확인 (0xFF)           |\n");
        debug_printf("| 0. 이전 메뉴                          |\n");
        debug_printf("|                                       |\n");
        debug_printf("|    CTRL+C: 이전, CTRL+Q: 종료         |\n");
        debug_printf("+=======================================+\n");
        debug_printf("Flash 정보: 8 MB (0x00 ~ 0x7FFFFF)\n");

        status = view_input_decimal("선택", &choice, 0, 3);

        if (status == MENU_ABORT)
        {
            debug_printf("\n프로그램을 종료합니다.\n");
            return;
        }
        else if (status == MENU_BACK || choice == MENU_FLASH_EXIT)
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
            case MENU_FLASH_RW_TEST:
                menu_flash_rw_test();
                break;

            case MENU_FLASH_HEX_DUMP:
                menu_flash_hex_dump();
                break;

            case MENU_FLASH_ERASE_CHECK:
                menu_flash_erase_check();
                break;

            default:
                debug_printf("잘못된 선택입니다.\n");
                break;
        }
    }
}
