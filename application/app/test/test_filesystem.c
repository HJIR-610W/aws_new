
#include "test_filesystem.h"

#include <string.h>
#include <stdio.h>

#include "app_file.h"
#include "debug_io.h"
#include "user_heap.h"
#include "console_utile.h"
#include "console_define.h"
#include "cli_key_code.h"

/* 외부 함수 선언 */
extern int read_sd_sector(unsigned char *buffer, uint32_t sector);

/* 메뉴 정의 */
#define MENU_FILE_RW_TEST      1
#define MENU_FILE_EXPLORER     2
#define MENU_SECTOR_HEX_DUMP   3
#define MENU_DELETE_FILE       4
#define MENU_CREATE_TEST_FILE  5
#define MENU_FILE_EXIT              0

/* 함수 프로토타입 */
static void menu_file_rw_test(void);
static void menu_file_explorer(void);
static void menu_sector_hex_dump(void);
static void menu_delete_file(void);
static void menu_create_test_file(void);
static void print_hex_dump(uint8_t *buffer, uint32_t size, uint32_t base_addr);
static void print_file_info(const char *path);

/**
 * @brief 파일 읽기/쓰기 테스트 메뉴
 */
static void menu_file_rw_test(void)
{
    char filename[128];
    int file_size_kb;
    int status;
    uint32_t file_size;
    FRESULT fr;

    debug_printf("\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("|       파일 쓰기/읽기 테스트           |\n");
    debug_printf("+---------------------------------------+\n");

    /* 파일명 입력 */
    debug_printf("파일명 입력 (예: 0:test.txt): ");
    status = debug_scanf_s("%s", filename, sizeof(filename));
    if (status == KEY_CODE_CTRL_C || status == KEY_CODE_CTRL_Q)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    /* 파일 크기 입력 (KB 단위) */
    status = view_input_decimal("파일 크기(KB)", &file_size_kb, 1, 10240);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    file_size = file_size_kb * 1024;

    debug_printf("\n파일: %s, 크기: %d KB (%u bytes)\n", filename, file_size_kb, file_size);
    debug_printf("파일 쓰기/읽기 속도 테스트를 시작합니다...\n\n");

    /* 파일 읽기/쓰기 속도 테스트 */
    fr = test_file_rw_speed(filename, file_size);

    if (fr == FR_OK)
    {
        debug_printf("\n테스트 성공!\n");
    }
    else
    {
        debug_printf("\n테스트 실패! (Error: %d)\n", fr);
    }

    debug_printf("\n아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief 파일 탐색기 메뉴 (윈도우 탐색기처럼)
 */
static void menu_file_explorer(void)
{
    char current_path[128] = "0:";
    char input_path[128];
    int status;

    debug_printf("\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("|         파일 시스템 탐색기            |\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("| 명령어:                               |\n");
    debug_printf("|  경로 입력 - 디렉토리 변경           |\n");
    debug_printf("|  Ctrl+C    - 이전 메뉴                |\n");
    debug_printf("+---------------------------------------+\n");

    while (1)
    {
        debug_printf("\n현재 경로: %s\n", current_path);
        debug_printf("----------------------------------------\n");

        /* 현재 디렉토리 목록 출력 */
        list_directory(current_path);

        debug_printf("----------------------------------------\n");
        debug_printf("경로 입력 (또는 Ctrl+C): ");

        /* 경로 입력 */
        status = debug_scanf_s("%s", input_path, sizeof(input_path));

        if (status == KEY_CODE_CTRL_C)
        {
            debug_printf("\n탐색기를 종료합니다.\n");
            break;
        }
        else if (status == KEY_CODE_CTRL_Q)
        {
            debug_printf("\n프로그램을 종료합니다.\n");
            break;
        }

        /* 입력된 경로 복사 */
        if (status > 0)
        {
            strncpy(current_path, input_path, sizeof(current_path) - 1);
            current_path[sizeof(current_path) - 1] = '\0';
        }
    }
}

/**
 * @brief 섹터 HEX DUMP 메뉴
 */
static void menu_sector_hex_dump(void)
{
    int start_sector;
    int sector_count;
    int status;
    uint32_t current_sector;
    uint8_t *buffer;
    int ret;

    debug_printf("\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("|         섹터 HEX DUMP                 |\n");
    debug_printf("+---------------------------------------+\n");

    /* 시작 섹터 입력 */
    status = view_input_decimal("시작 섹터 번호", &start_sector, 0, 0x7FFFFFFF);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    /* 섹터 개수 입력 */
    status = view_input_decimal("읽을 섹터 개수", &sector_count, 1, 256);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    /* 버퍼 할당 (섹터 크기 512 bytes) */
    buffer = (uint8_t *)pvPortMalloc(512);
    if (buffer == NULL)
    {
        debug_printf("메모리 할당 실패!\n");
        return;
    }

    debug_printf("\n섹터 %d 부터 %d개 섹터 읽기 시작...\n\n", start_sector, sector_count);

    /* 섹터 읽기 및 HEX DUMP */
    for (int i = 0; i < sector_count; i++)
    {
        current_sector = start_sector + i;

        debug_printf("========================================\n");
        debug_printf("섹터 번호: %u (0x%08X)\n", current_sector, current_sector);
        debug_printf("========================================\n");

        /* 섹터 읽기 */
        ret = read_sd_sector(buffer, current_sector);

        if (ret == 0)
        {
            /* HEX DUMP 출력 */
            print_hex_dump(buffer, 512, current_sector * 512);
        }
        else
        {
            debug_printf("섹터 읽기 실패! (Error: %d)\n", ret);
        }

        debug_printf("\n");

        /* 다음 섹터를 위해 잠시 대기 (너무 많은 출력 방지) */
        if ((i + 1) % 4 == 0 && (i + 1) < sector_count)
        {
            debug_printf("계속하려면 아무 키나 누르세요 (Ctrl+C: 중단)...");
            int key = get_key(0xFFFFFFFF);
            if (key == KEY_CODE_CTRL_C)
            {
                debug_printf("\n중단되었습니다.\n");
                break;
            }
            debug_printf("\n\n");
        }
    }

    vPortFree(buffer);

    debug_printf("\n섹터 HEX DUMP 완료!\n");
    debug_printf("아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief 파일 삭제 메뉴
 */
static void menu_delete_file(void)
{
    char filename[128];
    int status;
    int confirm;
    FRESULT fr;

    debug_printf("\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("|           파일 삭제                   |\n");
    debug_printf("+---------------------------------------+\n");

    /* 파일명 입력 */
    debug_printf("삭제할 파일명 입력 (예: 0:test.txt): ");
    status = debug_scanf_s("%s", filename, sizeof(filename));
    if (status == KEY_CODE_CTRL_C || status == KEY_CODE_CTRL_Q)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    /* 파일 정보 출력 */
    print_file_info(filename);

    /* 삭제 확인 */
    status = view_confirm_continue("정말 삭제하시겠습니까?", &confirm);
    if (status != MENU_OK || confirm != 1)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    /* 파일 삭제 */
    debug_printf("\n파일 삭제 중...\n");
    fr = delete_file(filename);

    if (fr == FR_OK)
    {
        debug_printf("파일이 성공적으로 삭제되었습니다.\n");
    }
    else
    {
        debug_printf("파일 삭제 실패! (Error: %d)\n", fr);
    }

    debug_printf("\n아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief 테스트 파일 생성 메뉴
 */
static void menu_create_test_file(void)
{
    char filename[128];
    int file_size_kb;
    int status;
    uint32_t file_size;
    uint8_t *buffer;
    FRESULT fr;

    debug_printf("\n");
    debug_printf("+---------------------------------------+\n");
    debug_printf("|        테스트 파일 생성               |\n");
    debug_printf("+---------------------------------------+\n");

    /* 파일명 입력 */
    debug_printf("파일명 입력 (예: 0:test.bin): ");
    status = debug_scanf_s("%s", filename, sizeof(filename));
    if (status == KEY_CODE_CTRL_C || status == KEY_CODE_CTRL_Q)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    /* 파일 크기 입력 (KB 단위) */
    status = view_input_decimal("파일 크기(KB)", &file_size_kb, 1, 10240);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        return;
    }

    file_size = file_size_kb * 1024;

    /* 버퍼 할당 (1KB씩 쓰기) */
    buffer = (uint8_t *)pvPortMalloc(1024);
    if (buffer == NULL)
    {
        debug_printf("메모리 할당 실패!\n");
        return;
    }

    /* 테스트 패턴 생성 (0~255 반복) */
    for (int i = 0; i < 1024; i++)
    {
        buffer[i] = i % 256;
    }

    debug_printf("\n파일 생성 중: %s (%d KB)...\n", filename, file_size_kb);

    /* 파일 생성 및 쓰기 */
    for (uint32_t offset = 0; offset < file_size; offset += 1024)
    {
        uint32_t write_size = (file_size - offset) > 1024 ? 1024 : (file_size - offset);

        if (offset == 0)
        {
            fr = write_file(filename, buffer, write_size, 0);
        }
        else
        {
            fr = append_file(filename, buffer, write_size);
        }

        if (fr != FR_OK)
        {
            debug_printf("파일 쓰기 실패! (Offset: %u, Error: %d)\n", offset, fr);
            vPortFree(buffer);
            return;
        }

        /* 진행률 표시 (10% 단위) */
        if ((offset * 10 / file_size) != ((offset + write_size) * 10 / file_size))
        {
            debug_printf("진행률: %u%%\n", (offset + write_size) * 100 / file_size);
        }
    }

    vPortFree(buffer);

    debug_printf("\n파일 생성 완료!\n");
    print_file_info(filename);

    debug_printf("\n아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief HEX DUMP 출력
 * @param buffer 버퍼 포인터
 * @param size 버퍼 크기
 * @param base_addr 기준 주소
 */
static void print_hex_dump(uint8_t *buffer, uint32_t size, uint32_t base_addr)
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
 * @brief 파일 정보 출력
 * @param path 파일 경로
 */
static void print_file_info(const char *path)
{
    FSIZE_t file_size;
    FRESULT fr;

    fr = get_file_size(path, &file_size);

    if (fr == FR_OK)
    {
        debug_printf("\n파일 정보:\n");
        debug_printf("  경로: %s\n", path);
        debug_printf("  크기: %u bytes (%.2f KB)\n", (uint32_t)file_size, (float)file_size / 1024.0f);
    }
    else
    {
        debug_printf("\n파일을 찾을 수 없습니다. (Error: %d)\n", fr);
    }
}

/**
 * @brief 파일시스템 테스트 메인 메뉴
 */
void test_filesystem(void)
{
    int choice;
    int status;

    while (1)
    {
        debug_printf("\n");
        debug_printf("+=======================================+\n");
        debug_printf("|       파일시스템 디버깅 메뉴          |\n");
        debug_printf("+=======================================+\n");
        debug_printf("| 1. 파일 쓰기/읽기 테스트              |\n");
        debug_printf("| 2. 파일 시스템 탐색기                 |\n");
        debug_printf("| 3. 섹터 HEX DUMP                      |\n");
        debug_printf("| 4. 파일 삭제                          |\n");
        debug_printf("| 5. 테스트 파일 생성                   |\n");
        debug_printf("| 0. 이전 메뉴                          |\n");
        debug_printf("|                                       |\n");
        debug_printf("|    CTRL+C: 이전, CTRL+Q: 종료         |\n");
        debug_printf("+=======================================+\n");

        status = view_input_decimal("선택", &choice, 0, 5);

        if (status == MENU_ABORT)
        {
            debug_printf("\n프로그램을 종료합니다.\n");
            return;
        }
        else if (status == MENU_BACK || choice == MENU_FILE_EXIT)
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
            case MENU_FILE_RW_TEST:
                menu_file_rw_test();
                break;

            case MENU_FILE_EXPLORER:
                menu_file_explorer();
                break;

            case MENU_SECTOR_HEX_DUMP:
                menu_sector_hex_dump();
                break;

            case MENU_DELETE_FILE:
                menu_delete_file();
                break;

            case MENU_CREATE_TEST_FILE:
                menu_create_test_file();
                break;

            default:
                debug_printf("잘못된 선택입니다.\n");
                break;
        }
    }
}
