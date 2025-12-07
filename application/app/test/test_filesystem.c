
#include "test_filesystem.h"

#include <string.h>
#include <stdio.h>

#include "app_file.h"
#include "debug_io.h"
#include "user_heap.h"
#include "console_utile.h"
#include "console_define.h"
#include "cli_key_code.h"

/* 고급 기능 활성화 매크로 (0: 비활성화, 1: 활성화) */
#ifndef TEST_FILE_ADVANCED
#define TEST_FILE_ADVANCED 1
#endif

/* 외부 함수 선언 */
extern int read_sd_sector(unsigned char *buffer, uint32_t sector);

/* 메뉴 정의 */
#define MENU_FILE_RW_TEST       1
#define MENU_FILE_EXPLORER      2
#define MENU_SECTOR_HEX_DUMP    3
#define MENU_DELETE_FILE        4
#define MENU_CREATE_TEST_FILE   5

#if TEST_FILE_ADVANCED
#define MENU_FILESYSTEM_INFO    6
#define MENU_ROOT_ENTRY_DUMP    7
#define MENU_FILE_FAT_TRACE     8
#endif

#define MENU_FILE_EXIT          0

#if TEST_FILE_ADVANCED
/* FAT32 구조체 정의 */
#pragma pack(push, 1)
typedef struct {
    uint8_t  jump_boot[3];           /* 부트 점프 명령 */
    uint8_t  oem_name[8];            /* OEM 이름 */
    uint16_t bytes_per_sector;       /* 섹터당 바이트 수 */
    uint8_t  sectors_per_cluster;    /* 클러스터당 섹터 수 */
    uint16_t reserved_sectors;       /* 예약된 섹터 수 */
    uint8_t  num_fats;               /* FAT 개수 */
    uint16_t root_entry_count;       /* 루트 디렉토리 엔트리 수 (FAT32는 0) */
    uint16_t total_sectors_16;       /* 전체 섹터 수 (16bit) */
    uint8_t  media_type;             /* 미디어 타입 */
    uint16_t fat_size_16;            /* FAT 크기 (16bit, FAT32는 0) */
    uint16_t sectors_per_track;      /* 트랙당 섹터 수 */
    uint16_t num_heads;              /* 헤드 수 */
    uint32_t hidden_sectors;         /* 숨겨진 섹터 수 */
    uint32_t total_sectors_32;       /* 전체 섹터 수 (32bit) */
    /* FAT32 확장 영역 */
    uint32_t fat_size_32;            /* FAT 크기 (32bit) */
    uint16_t ext_flags;              /* 확장 플래그 */
    uint16_t fs_version;             /* 파일시스템 버전 */
    uint32_t root_cluster;           /* 루트 디렉토리 클러스터 번호 */
    uint16_t fs_info_sector;         /* FSInfo 섹터 번호 */
    uint16_t backup_boot_sector;     /* 백업 부트 섹터 번호 */
    uint8_t  reserved[12];           /* 예약 영역 */
    uint8_t  drive_number;           /* 드라이브 번호 */
    uint8_t  reserved1;              /* 예약 */
    uint8_t  boot_signature;         /* 부트 시그니처 */
    uint32_t volume_id;              /* 볼륨 ID */
    uint8_t  volume_label[11];       /* 볼륨 레이블 */
    uint8_t  fs_type[8];             /* 파일시스템 타입 */
} fat32_boot_sector_t;

/* FAT32 디렉토리 엔트리 구조체 */
typedef struct {
    uint8_t  name[11];               /* 파일명 (8.3 형식) */
    uint8_t  attr;                   /* 파일 속성 */
    uint8_t  nt_reserved;            /* NT 예약 */
    uint8_t  crt_time_tenth;         /* 생성 시간 (1/10초) */
    uint16_t crt_time;               /* 생성 시간 */
    uint16_t crt_date;               /* 생성 날짜 */
    uint16_t lst_acc_date;           /* 마지막 접근 날짜 */
    uint16_t fst_clus_hi;            /* 시작 클러스터 (상위 16bit) */
    uint16_t wrt_time;               /* 쓰기 시간 */
    uint16_t wrt_date;               /* 쓰기 날짜 */
    uint16_t fst_clus_lo;            /* 시작 클러스터 (하위 16bit) */
    uint32_t file_size;              /* 파일 크기 */
} fat32_dir_entry_t;
#pragma pack(pop)

/* FAT32 상수 정의 */
#define FAT32_EOC           0x0FFFFFF8  /* End of Cluster Chain */
#define FAT32_BAD_CLUSTER   0x0FFFFFF7  /* Bad Cluster */
#define ATTR_READ_ONLY      0x01
#define ATTR_HIDDEN         0x02
#define ATTR_SYSTEM         0x04
#define ATTR_VOLUME_ID      0x08
#define ATTR_DIRECTORY      0x10
#define ATTR_ARCHIVE        0x20
#define ATTR_LONG_NAME      (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#endif

/* 함수 프로토타입 */
static void menu_file_rw_test(void);
static void menu_file_explorer(void);
static void menu_sector_hex_dump(void);
static void menu_delete_file(void);
static void menu_create_test_file(void);
#if TEST_FILE_ADVANCED
static void menu_filesystem_info(void);
static void menu_root_entry_dump(void);
static void menu_file_fat_trace(void);
static uint32_t get_root_dir_first_sector(fat32_boot_sector_t *boot_sector);
static uint32_t get_next_cluster(uint8_t *fat_buffer, uint32_t current_cluster);
static uint32_t cluster_to_sector(fat32_boot_sector_t *boot_sector, uint32_t cluster);
static int find_file_in_directory(fat32_boot_sector_t *boot_sector, const char *filename,
                                   uint32_t dir_cluster, fat32_dir_entry_t *entry);
#endif
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

#if TEST_FILE_ADVANCED
/**
 * @brief 루트 디렉토리 첫 번째 섹터 계산
 * @param boot_sector 부트 섹터 구조체
 * @return 루트 디렉토리 첫 번째 섹터 번호
 */
static uint32_t get_root_dir_first_sector(fat32_boot_sector_t *boot_sector)
{
    uint32_t fat_size;
    uint32_t first_data_sector;
    uint32_t first_sector_of_cluster;

    /* FAT 크기 결정 */
    if (boot_sector->fat_size_16 != 0)
    {
        fat_size = boot_sector->fat_size_16;
    }
    else
    {
        fat_size = boot_sector->fat_size_32;
    }

    /* 첫 번째 데이터 섹터 계산 */
    first_data_sector = boot_sector->reserved_sectors +
                       (boot_sector->num_fats * fat_size);

    /* 루트 디렉토리의 첫 번째 섹터 계산 (클러스터 번호에서 섹터로 변환) */
    first_sector_of_cluster = ((boot_sector->root_cluster - 2) * boot_sector->sectors_per_cluster) + first_data_sector;

    return first_sector_of_cluster;
}

/**
 * @brief 파일시스템 정보 표시 메뉴
 */
static void menu_filesystem_info(void)
{
    uint8_t *buffer;
    fat32_boot_sector_t *boot_sector;
    int ret;
    uint32_t fat_size;
    uint32_t total_sectors;
    uint32_t data_sectors;
    uint32_t total_clusters;
    uint32_t root_dir_first_sector;
    char oem_name[9];
    char volume_label[12];
    char fs_type[9];

    debug_printf("\n");
    debug_printf("+=======================================+\n");
    debug_printf("|       파일시스템 정보                 |\n");
    debug_printf("+=======================================+\n");

    /* 버퍼 할당 (섹터 크기 512 bytes) */
    buffer = (uint8_t *)pvPortMalloc(512);
    if (buffer == NULL)
    {
        debug_printf("메모리 할당 실패!\n");
        return;
    }

    /* 부트 섹터(섹터 0) 읽기 */
    debug_printf("\n부트 섹터 읽기 중...\n");
    ret = read_sd_sector(buffer, 0);

    if (ret != 0)
    {
        debug_printf("부트 섹터 읽기 실패! (Error: %d)\n", ret);
        vPortFree(buffer);
        return;
    }

    boot_sector = (fat32_boot_sector_t *)buffer;

    /* FAT 크기 결정 */
    if (boot_sector->fat_size_16 != 0)
    {
        fat_size = boot_sector->fat_size_16;
    }
    else
    {
        fat_size = boot_sector->fat_size_32;
    }

    /* 전체 섹터 수 결정 */
    if (boot_sector->total_sectors_16 != 0)
    {
        total_sectors = boot_sector->total_sectors_16;
    }
    else
    {
        total_sectors = boot_sector->total_sectors_32;
    }

    /* 데이터 섹터 및 클러스터 계산 */
    data_sectors = total_sectors - (boot_sector->reserved_sectors +
                   (boot_sector->num_fats * fat_size));
    total_clusters = data_sectors / boot_sector->sectors_per_cluster;

    /* 루트 디렉토리 첫 번째 섹터 계산 */
    root_dir_first_sector = get_root_dir_first_sector(boot_sector);

    /* 문자열 복사 (null terminator 추가) */
    memcpy(oem_name, boot_sector->oem_name, 8);
    oem_name[8] = '\0';
    memcpy(volume_label, boot_sector->volume_label, 11);
    volume_label[11] = '\0';
    memcpy(fs_type, boot_sector->fs_type, 8);
    fs_type[8] = '\0';

    /* 파일시스템 정보 출력 */
    debug_printf("\n========================================\n");
    debug_printf("기본 정보:\n");
    debug_printf("========================================\n");
    debug_printf("OEM 이름            : %s\n", oem_name);
    debug_printf("볼륨 레이블         : %s\n", volume_label);
    debug_printf("파일시스템 타입     : %s\n", fs_type);
    debug_printf("볼륨 ID             : 0x%08X\n", boot_sector->volume_id);

    debug_printf("\n========================================\n");
    debug_printf("섹터/클러스터 정보:\n");
    debug_printf("========================================\n");
    debug_printf("섹터당 바이트 수    : %u bytes\n", boot_sector->bytes_per_sector);
    debug_printf("클러스터당 섹터 수  : %u\n", boot_sector->sectors_per_cluster);
    debug_printf("클러스터 크기       : %u KB\n",
                 (boot_sector->bytes_per_sector * boot_sector->sectors_per_cluster) / 1024);
    debug_printf("전체 섹터 수        : %u\n", total_sectors);
    debug_printf("전체 용량           : %u MB\n",
                 (total_sectors * boot_sector->bytes_per_sector) / (1024 * 1024));

    debug_printf("\n========================================\n");
    debug_printf("FAT 정보:\n");
    debug_printf("========================================\n");
    debug_printf("예약된 섹터 수      : %u\n", boot_sector->reserved_sectors);
    debug_printf("FAT 개수            : %u\n", boot_sector->num_fats);
    debug_printf("FAT 크기            : %u 섹터\n", fat_size);
    debug_printf("FAT 시작 섹터       : %u\n", boot_sector->reserved_sectors);
    debug_printf("FAT 전체 크기       : %u KB\n",
                 (fat_size * boot_sector->num_fats * boot_sector->bytes_per_sector) / 1024);

    debug_printf("\n========================================\n");
    debug_printf("데이터 영역:\n");
    debug_printf("========================================\n");
    debug_printf("데이터 섹터 수      : %u\n", data_sectors);
    debug_printf("전체 클러스터 수    : %u\n", total_clusters);
    debug_printf("루트 클러스터 번호  : %u\n", boot_sector->root_cluster);
    debug_printf("루트 디렉토리 섹터  : %u (0x%08X)\n",
                 root_dir_first_sector, root_dir_first_sector);
    debug_printf("데이터 시작 섹터    : %u\n",
                 boot_sector->reserved_sectors + (boot_sector->num_fats * fat_size));

    debug_printf("\n========================================\n");
    debug_printf("기타 정보:\n");
    debug_printf("========================================\n");
    debug_printf("미디어 타입         : 0x%02X\n", boot_sector->media_type);
    debug_printf("트랙당 섹터 수      : %u\n", boot_sector->sectors_per_track);
    debug_printf("헤드 수             : %u\n", boot_sector->num_heads);
    debug_printf("숨겨진 섹터 수      : %u\n", boot_sector->hidden_sectors);
    debug_printf("FSInfo 섹터         : %u\n", boot_sector->fs_info_sector);
    debug_printf("백업 부트 섹터      : %u\n", boot_sector->backup_boot_sector);
    debug_printf("드라이브 번호       : 0x%02X\n", boot_sector->drive_number);
    debug_printf("부트 시그니처       : 0x%02X\n", boot_sector->boot_signature);

    vPortFree(buffer);

    debug_printf("\n아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief Root Entry HEX DUMP 메뉴
 */
static void menu_root_entry_dump(void)
{
    uint8_t *buffer;
    fat32_boot_sector_t *boot_sector;
    int ret;
    uint32_t root_dir_sector;
    int sector_count;
    int status;

    debug_printf("\n");
    debug_printf("+=======================================+\n");
    debug_printf("|      Root Entry HEX DUMP              |\n");
    debug_printf("+=======================================+\n");

    /* 버퍼 할당 (섹터 크기 512 bytes) */
    buffer = (uint8_t *)pvPortMalloc(512);
    if (buffer == NULL)
    {
        debug_printf("메모리 할당 실패!\n");
        return;
    }

    /* 부트 섹터(섹터 0) 읽기 */
    debug_printf("\n부트 섹터 읽기 중...\n");
    ret = read_sd_sector(buffer, 0);

    if (ret != 0)
    {
        debug_printf("부트 섹터 읽기 실패! (Error: %d)\n", ret);
        vPortFree(buffer);
        return;
    }

    boot_sector = (fat32_boot_sector_t *)buffer;

    /* 루트 디렉토리 첫 번째 섹터 계산 */
    root_dir_sector = get_root_dir_first_sector(boot_sector);

    debug_printf("\n루트 디렉토리 시작 섹터: %u (0x%08X)\n", root_dir_sector, root_dir_sector);
    debug_printf("클러스터당 섹터 수: %u\n", boot_sector->sectors_per_cluster);

    /* 읽을 섹터 개수 입력 */
    status = view_input_decimal("읽을 섹터 개수", &sector_count, 1, 256);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
        debug_printf("취소되었습니다.\n");
        vPortFree(buffer);
        return;
    }

    debug_printf("\n루트 엔트리 섹터 %u 부터 %d개 섹터 읽기 시작...\n\n",
                 root_dir_sector, sector_count);

    /* 섹터 읽기 및 HEX DUMP */
    for (int i = 0; i < sector_count; i++)
    {
        uint32_t current_sector = root_dir_sector + i;

        debug_printf("========================================\n");
        debug_printf("섹터 번호: %u (0x%08X)\n", current_sector, current_sector);
        debug_printf("오프셋: 루트 + %d 섹터\n", i);
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

    debug_printf("\nRoot Entry HEX DUMP 완료!\n");
    debug_printf("아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}

/**
 * @brief FAT에서 다음 클러스터 번호 가져오기
 * @param fat_buffer FAT 버퍼
 * @param current_cluster 현재 클러스터 번호
 * @return 다음 클러스터 번호 (EOC이면 0xFFFFFFFF)
 */
static uint32_t get_next_cluster(uint8_t *fat_buffer, uint32_t current_cluster)
{
    uint32_t fat_offset;
    uint32_t next_cluster;

    /* FAT32에서 각 엔트리는 4바이트 */
    fat_offset = current_cluster * 4;

    /* FAT 엔트리 읽기 (Little Endian) */
    next_cluster = fat_buffer[fat_offset] |
                  (fat_buffer[fat_offset + 1] << 8) |
                  (fat_buffer[fat_offset + 2] << 16) |
                  (fat_buffer[fat_offset + 3] << 24);

    /* 상위 4비트는 예약 영역이므로 마스크 */
    next_cluster &= 0x0FFFFFFF;

    return next_cluster;
}

/**
 * @brief 클러스터 번호를 섹터 번호로 변환
 * @param boot_sector 부트 섹터 구조체
 * @param cluster 클러스터 번호
 * @return 섹터 번호
 */
static uint32_t cluster_to_sector(fat32_boot_sector_t *boot_sector, uint32_t cluster)
{
    uint32_t fat_size;
    uint32_t first_data_sector;
    uint32_t first_sector_of_cluster;

    /* FAT 크기 결정 */
    if (boot_sector->fat_size_16 != 0)
    {
        fat_size = boot_sector->fat_size_16;
    }
    else
    {
        fat_size = boot_sector->fat_size_32;
    }

    /* 첫 번째 데이터 섹터 계산 */
    first_data_sector = boot_sector->reserved_sectors +
                       (boot_sector->num_fats * fat_size);

    /* 클러스터를 섹터로 변환 */
    first_sector_of_cluster = ((cluster - 2) * boot_sector->sectors_per_cluster) + first_data_sector;

    return first_sector_of_cluster;
}

/**
 * @brief 디렉토리에서 파일 찾기
 * @param boot_sector 부트 섹터 구조체
 * @param filename 찾을 파일명 (예: "TEST.TXT")
 * @param dir_cluster 디렉토리 시작 클러스터
 * @param entry 찾은 디렉토리 엔트리 저장 위치
 * @return 0: 성공, -1: 실패
 */
static int find_file_in_directory(fat32_boot_sector_t *boot_sector, const char *filename,
                                   uint32_t dir_cluster, fat32_dir_entry_t *entry)
{
    char short_name[12];
    uint8_t *buffer;
    uint32_t current_cluster;
    uint32_t sector;
    int ret;
    int i;
    int j;
    int found = 0;

    /* 8.3 형식으로 변환 (공백으로 패딩) */
    memset(short_name, ' ', 11);
    short_name[11] = '\0';

    /* 파일명 복사 */
    for (i = 0, j = 0; filename[i] != '\0' && filename[i] != '.'; i++, j++)
    {
        if (j >= 8) break;
        short_name[j] = filename[i];
    }

    /* 확장자 복사 */
    if (filename[i] == '.')
    {
        i++;
        for (j = 8; filename[i] != '\0'; i++, j++)
        {
            if (j >= 11) break;
            short_name[j] = filename[i];
        }
    }

    /* 버퍼 할당 */
    buffer = (uint8_t *)pvPortMalloc(512);
    if (buffer == NULL)
    {
        return -1;
    }

    current_cluster = dir_cluster;

    /* 디렉토리 클러스터 순회 */
    while (current_cluster < FAT32_EOC)
    {
        /* 클러스터의 각 섹터 읽기 */
        for (i = 0; i < boot_sector->sectors_per_cluster; i++)
        {
            sector = cluster_to_sector(boot_sector, current_cluster) + i;
            ret = read_sd_sector(buffer, sector);

            if (ret != 0)
            {
                vPortFree(buffer);
                return -1;
            }

            /* 섹터 내의 모든 디렉토리 엔트리 검사 (섹터당 16개) */
            for (j = 0; j < 16; j++)
            {
                fat32_dir_entry_t *dir_entry = (fat32_dir_entry_t *)&buffer[j * 32];

                /* 빈 엔트리면 종료 */
                if (dir_entry->name[0] == 0x00)
                {
                    vPortFree(buffer);
                    return -1;
                }

                /* 삭제된 엔트리는 건너뜀 */
                if (dir_entry->name[0] == 0xE5)
                {
                    continue;
                }

                /* 롱 파일명 엔트리는 건너뜀 */
                if ((dir_entry->attr & ATTR_LONG_NAME) == ATTR_LONG_NAME)
                {
                    continue;
                }

                /* 볼륨 레이블은 건너뜀 */
                if (dir_entry->attr & ATTR_VOLUME_ID)
                {
                    continue;
                }

                /* 파일명 비교 */
                if (memcmp(dir_entry->name, short_name, 11) == 0)
                {
                    memcpy(entry, dir_entry, sizeof(fat32_dir_entry_t));
                    found = 1;
                    break;
                }
            }

            if (found)
            {
                break;
            }
        }

        if (found)
        {
            break;
        }

        /* 다음 클러스터 (여기서는 루트만 검색하므로 break) */
        break;
    }

    vPortFree(buffer);

    return found ? 0 : -1;
}

/**
 * @brief 파일 FAT 체인 추적 메뉴
 */
static void menu_file_fat_trace(void)
{
    char filename[64];
    uint8_t *buffer;
    uint8_t *fat_buffer;
    fat32_boot_sector_t *boot_sector;
    fat32_dir_entry_t file_entry;
    uint32_t fat_size;
    uint32_t fat_start_sector;
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t next_cluster;
    uint32_t cluster_count;
    uint32_t sector;
    int ret;
    int status;
    int i;

    debug_printf("\n");
    debug_printf("+=======================================+\n");
    debug_printf("|      파일 FAT 체인 추적               |\n");
    debug_printf("+=======================================+\n");

    /* 버퍼 할당 */
    buffer = (uint8_t *)pvPortMalloc(512);
    if (buffer == NULL)
    {
        debug_printf("메모리 할당 실패!\n");
        return;
    }

    /* 부트 섹터 읽기 */
    debug_printf("\n부트 섹터 읽기 중...\n");
    ret = read_sd_sector(buffer, 0);

    if (ret != 0)
    {
        debug_printf("부트 섹터 읽기 실패! (Error: %d)\n", ret);
        vPortFree(buffer);
        return;
    }

    boot_sector = (fat32_boot_sector_t *)buffer;

    /* FAT 정보 계산 */
    if (boot_sector->fat_size_16 != 0)
    {
        fat_size = boot_sector->fat_size_16;
    }
    else
    {
        fat_size = boot_sector->fat_size_32;
    }

    fat_start_sector = boot_sector->reserved_sectors;

    debug_printf("FAT 시작 섹터: %u\n", fat_start_sector);
    debug_printf("FAT 크기: %u 섹터 (%u KB)\n", fat_size, (fat_size * 512) / 1024);

    /* 파일명 입력 */
    debug_printf("\n파일명 입력 (예: TEST.TXT, 대문자로): ");
    status = debug_scanf_s("%s", filename, sizeof(filename));
    if (status == KEY_CODE_CTRL_C || status == KEY_CODE_CTRL_Q)
    {
        debug_printf("취소되었습니다.\n");
        vPortFree(buffer);
        return;
    }

    /* 루트 디렉토리에서 파일 찾기 */
    debug_printf("\n루트 디렉토리에서 파일 검색 중...\n");
    ret = find_file_in_directory(boot_sector, filename, boot_sector->root_cluster, &file_entry);

    if (ret != 0)
    {
        debug_printf("파일을 찾을 수 없습니다: %s\n", filename);
        vPortFree(buffer);
        return;
    }

    /* 파일 정보 출력 */
    first_cluster = ((uint32_t)file_entry.fst_clus_hi << 16) | file_entry.fst_clus_lo;

    debug_printf("\n========================================\n");
    debug_printf("파일 정보:\n");
    debug_printf("========================================\n");
    debug_printf("파일명          : %.8s.%.3s\n", file_entry.name, &file_entry.name[8]);
    debug_printf("크기            : %u bytes (%.2f KB)\n",
                 file_entry.file_size, (float)file_entry.file_size / 1024.0f);
    debug_printf("시작 클러스터   : %u (0x%08X)\n", first_cluster, first_cluster);
    debug_printf("시작 섹터       : %u (0x%08X)\n",
                 cluster_to_sector(boot_sector, first_cluster),
                 cluster_to_sector(boot_sector, first_cluster));
    debug_printf("속성            : 0x%02X ", file_entry.attr);
    if (file_entry.attr & ATTR_READ_ONLY) debug_printf("[읽기전용] ");
    if (file_entry.attr & ATTR_HIDDEN) debug_printf("[숨김] ");
    if (file_entry.attr & ATTR_SYSTEM) debug_printf("[시스템] ");
    if (file_entry.attr & ATTR_DIRECTORY) debug_printf("[디렉토리] ");
    if (file_entry.attr & ATTR_ARCHIVE) debug_printf("[보관] ");
    debug_printf("\n");

    /* 디렉토리 엔트리 HEX DUMP (32바이트) */
    debug_printf("\n========================================\n");
    debug_printf("디렉토리 엔트리 HEX DUMP (32 bytes):\n");
    debug_printf("========================================\n");
    debug_printf("Offset  : 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  ASCII\n");
    debug_printf("--------+------------------------+------------------------+----------------\n");

    /* 첫 번째 16바이트 */
    debug_printf("00000000: ");
    for (i = 0; i < 8; i++)
    {
        debug_printf("%02X ", ((uint8_t *)&file_entry)[i]);
    }
    debug_printf(" ");
    for (i = 8; i < 16; i++)
    {
        debug_printf("%02X ", ((uint8_t *)&file_entry)[i]);
    }
    debug_printf(" |");
    for (i = 0; i < 16; i++)
    {
        uint8_t c = ((uint8_t *)&file_entry)[i];
        debug_printf("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    debug_printf("|\n");

    /* 두 번째 16바이트 */
    debug_printf("00000010: ");
    for (i = 16; i < 24; i++)
    {
        debug_printf("%02X ", ((uint8_t *)&file_entry)[i]);
    }
    debug_printf(" ");
    for (i = 24; i < 32; i++)
    {
        debug_printf("%02X ", ((uint8_t *)&file_entry)[i]);
    }
    debug_printf(" |");
    for (i = 16; i < 32; i++)
    {
        uint8_t c = ((uint8_t *)&file_entry)[i];
        debug_printf("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    debug_printf("|\n");

    /* 엔트리 필드별 상세 정보 */
    debug_printf("\n========================================\n");
    debug_printf("엔트리 필드 상세:\n");
    debug_printf("========================================\n");
    debug_printf("[00-0A] 파일명      : %.8s.%.3s\n", file_entry.name, &file_entry.name[8]);
    debug_printf("[0B]    속성        : 0x%02X\n", file_entry.attr);
    debug_printf("[0C]    NT 예약     : 0x%02X\n", file_entry.nt_reserved);
    debug_printf("[0D]    생성 1/10초 : 0x%02X\n", file_entry.crt_time_tenth);
    debug_printf("[0E-0F] 생성 시간   : 0x%04X\n", file_entry.crt_time);
    debug_printf("[10-11] 생성 날짜   : 0x%04X\n", file_entry.crt_date);
    debug_printf("[12-13] 접근 날짜   : 0x%04X\n", file_entry.lst_acc_date);
    debug_printf("[14-15] 클러스터(H) : 0x%04X\n", file_entry.fst_clus_hi);
    debug_printf("[16-17] 쓰기 시간   : 0x%04X\n", file_entry.wrt_time);
    debug_printf("[18-19] 쓰기 날짜   : 0x%04X\n", file_entry.wrt_date);
    debug_printf("[1A-1B] 클러스터(L) : 0x%04X\n", file_entry.fst_clus_lo);
    debug_printf("[1C-1F] 파일 크기   : 0x%08X (%u bytes)\n", file_entry.file_size, file_entry.file_size);

    /* FAT 버퍼 할당 (섹터 단위로 읽기) */
    fat_buffer = (uint8_t *)pvPortMalloc(512);
    if (fat_buffer == NULL)
    {
        debug_printf("FAT 버퍼 할당 실패!\n");
        vPortFree(buffer);
        return;
    }

    /* FAT 체인 추적 */
    debug_printf("\n========================================\n");
    debug_printf("FAT 체인 정보:\n");
    debug_printf("========================================\n");
    debug_printf("No.  Cluster   Sector     FAT섹터  FAT오프셋\n");
    debug_printf("---- --------- ---------- -------- ---------\n");

    current_cluster = first_cluster;
    cluster_count = 0;

    while (current_cluster < FAT32_EOC)
    {
        uint32_t fat_offset;
        uint32_t fat_sector;
        uint32_t fat_sector_offset;

        /* FAT 오프셋 및 섹터 계산 */
        fat_offset = current_cluster * 4;
        fat_sector = fat_start_sector + (fat_offset / 512);
        fat_sector_offset = fat_offset % 512;

        /* 해당 클러스터의 실제 섹터 번호 */
        sector = cluster_to_sector(boot_sector, current_cluster);

        /* 클러스터 정보 출력 */
        debug_printf("%-4u %9u %10u %8u %9u\n",
                     cluster_count,
                     current_cluster,
                     sector,
                     fat_sector,
                     fat_sector_offset);

        cluster_count++;

        /* FAT 섹터 읽기 */
        ret = read_sd_sector(fat_buffer, fat_sector);
        if (ret != 0)
        {
            debug_printf("FAT 섹터 읽기 실패! (Sector: %u)\n", fat_sector);
            break;
        }

        /* 다음 클러스터 가져오기 */
        next_cluster = get_next_cluster(&fat_buffer[fat_sector_offset % 512], 0);

        /* EOC 체크 */
        if (next_cluster >= FAT32_EOC)
        {
            debug_printf("\n=> 클러스터 체인 종료 (EOC: 0x%08X)\n", next_cluster);
            break;
        }

        /* Bad Cluster 체크 */
        if (next_cluster == FAT32_BAD_CLUSTER)
        {
            debug_printf("\n=> Bad Cluster 발견!\n");
            break;
        }

        current_cluster = next_cluster;

        /* 너무 많은 클러스터 방지 (1000개 제한) */
        if (cluster_count >= 1000)
        {
            debug_printf("\n=> 클러스터 개수가 너무 많습니다 (1000개 제한)\n");
            break;
        }

        /* 10개마다 일시정지 */
        if (cluster_count % 10 == 0)
        {
            debug_printf("\n계속하려면 아무 키나 누르세요 (Ctrl+C: 중단)...");
            int key = get_key(0xFFFFFFFF);
            if (key == KEY_CODE_CTRL_C)
            {
                debug_printf("\n중단되었습니다.\n");
                break;
            }
            debug_printf("\n");
        }
    }

    debug_printf("\n========================================\n");
    debug_printf("요약:\n");
    debug_printf("========================================\n");
    debug_printf("총 클러스터 개수: %u\n", cluster_count);
    debug_printf("총 섹터 개수    : %u\n", cluster_count * boot_sector->sectors_per_cluster);
    debug_printf("할당된 크기     : %u bytes (%.2f KB)\n",
                 cluster_count * boot_sector->sectors_per_cluster * 512,
                 (float)(cluster_count * boot_sector->sectors_per_cluster * 512) / 1024.0f);
    debug_printf("실제 파일 크기  : %u bytes (%.2f KB)\n",
                 file_entry.file_size, (float)file_entry.file_size / 1024.0f);
    debug_printf("낭비 공간       : %u bytes\n",
                 (cluster_count * boot_sector->sectors_per_cluster * 512) - file_entry.file_size);

    vPortFree(fat_buffer);
    vPortFree(buffer);

    debug_printf("\n아무 키나 누르세요...");
    get_key(0xFFFFFFFF);
}
#endif /* TEST_FILE_ADVANCED */

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
#if TEST_FILE_ADVANCED
        debug_printf("| 6. 파일시스템 정보                    |\n");
        debug_printf("| 7. Root Entry HEX DUMP                |\n");
        debug_printf("| 8. 파일 FAT 체인 추적                 |\n");
#endif
        debug_printf("| 0. 이전 메뉴                          |\n");
        debug_printf("|                                       |\n");
        debug_printf("|    CTRL+C: 이전, CTRL+Q: 종료         |\n");
        debug_printf("+=======================================+\n");

#if TEST_FILE_ADVANCED
        status = view_input_decimal("선택", &choice, 0, 8);
#else
        status = view_input_decimal("선택", &choice, 0, 5);
#endif

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

#if TEST_FILE_ADVANCED
            case MENU_FILESYSTEM_INFO:
                menu_filesystem_info();
                break;

            case MENU_ROOT_ENTRY_DUMP:
                menu_root_entry_dump();
                break;

            case MENU_FILE_FAT_TRACE:
                menu_file_fat_trace();
                break;
#endif

            default:
                debug_printf("잘못된 선택입니다.\n");
                break;
        }
    }
}
