#ifndef JOURNAL_MANAGER_H
#define JOURNAL_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "config_memory_map.h"


#define JOURNAL_FRAM_BASE_ADDR   CONFIG_JOURNAL_START_ADDRESS 
// 섹터 크기 (FATFS의 기본 섹터 크기인 512바이트에 맞춤)
#define JOURNAL_SECTOR_SIZE      512 

// --- Journal Record Structure (FRAM에 저장될 구조) ---
// FRAM에 저장되는 전체 데이터 크기: 1 (유효성 플래그) + 4 (LBA 주소) + 512 (섹터 데이터) = 517 바이트
typedef struct 
{
    // 0xAA: 유효한 백업본이 있음을 표시 (Write Ahead Log 완료)
    // 0xFF: 백업본이 없거나 (초기 상태), Commit 완료됨을 표시
    uint8_t  is_valid;               
    // 백업된 섹터의 LBA(Logical Block Address, 섹터 번호)
    uint32_t sector_lba;             
    // 512 바이트 섹터 데이터
    uint8_t  data[JOURNAL_SECTOR_SIZE]; 
} journal_record_t;

// --- Function Prototypes ---

/**
 * @brief 저널 시스템을 초기화하고, 복구할 데이터가 있는지 확인합니다.
 * 유효한 백업본이 있다면 SD 카드에 복구하는 작업을 수행합니다.
 * @return true: 복구 작업 성공 (또는 복구할 데이터 없음), false: 복구 작업 실패
 */
bool journal_init_and_recover(void);

/**
 * @brief SD 카드 쓰기 전에 섹터 데이터와 LBA를 FRAM에 백업합니다 (WAL).
 * @param lba 백업할 섹터의 LBA (Logical Block Address)
 * @param sector_data 백업할 512바이트 섹터 데이터 포인터
 * @return true: 백업 성공, false: 백업 실패
 */
bool journal_backup_sector(uint32_t lba, const uint8_t *sector_data);

/**
 * @brief SD 카드 쓰기 성공 후, FRAM 백업본을 무효화(Commit)합니다.
 * @return true: Commit 성공, false: Commit 실패
 */
bool journal_commit(void);


extern bool FRAM_read(uint32_t addr, uint8_t* dest, uint32_t len);


extern bool FRAM_write(uint32_t addr, const uint8_t* src, uint32_t len);


extern bool SD_write_sector(uint32_t lba, const uint8_t *sector_data);

#endif 