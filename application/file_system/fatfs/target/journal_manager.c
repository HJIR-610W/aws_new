#include "journal_manager.h"

#include "drv_fram.h"

#define JOURNAL_USE 0

// 저널 레코드를 저장할 전역/정적 변수 (FRAM의 내용을 읽어오거나 FRAM에 기록하기 위한 버퍼 역할)
static journal_record_t g_journal_record;

/**
 * @brief FRAM의 특정 영역을 읽어옵니다.
 */
static bool read_fram_record(journal_record_t *record)
{
#if (JOURNAL_USE == 1)
    drv_fram_read(JOURNAL_FRAM_BASE_ADDR, (uint8_t*)record, sizeof(journal_record_t));
#endif
    return true;
}

/**
 * @brief FRAM의 특정 영역에 기록합니다.
 */
static bool write_fram_record(const journal_record_t *record) 
{
#if (JOURNAL_USE == 1)
     drv_fram_write(JOURNAL_FRAM_BASE_ADDR, ( uint8_t*)record, sizeof(journal_record_t));
#endif
     return true;
}

// --------------------------------------------------------------------------------

bool journal_init_and_recover(void) {
    // 1. FRAM에서 저널 레코드를 읽어옵니다.
    if (!read_fram_record(&g_journal_record))
    {
        return false; 
    }

    // 2. 유효성 플래그를 확인하여 복구할 데이터가 있는지 검사합니다.
    if (g_journal_record.is_valid == 0xAA) 
    {
        uint32_t lba = g_journal_record.sector_lba;
        const uint8_t *data = g_journal_record.data;

        // **장애 발생 전 SD 카드에 쓰기 시도했던 데이터를 다시 씁니다.**
        if (SD_write_sector(lba, data))
        {
            // 복구 쓰기 성공 -> Commit (FRAM 백업본 무효화)
            if (journal_commit()) {
                //printf("Journal Recovery Successful for LBA: %lu\n", lba);
                return true;
            } else {
                // Commit 실패 (FRAM 쓰기 문제). 이 경우 다음 리셋 시 다시 복구 시도.
                return false; 
            }
        } else {
            // 복구 쓰기 실패 (SD 카드 드라이버 문제).
            // 데이터는 FRAM에 그대로 남아있어 다음 리셋 시 다시 시도될 것입니다.
            return false;
        }
    } else {
        // 유효한 백업본이 없으므로 복구할 필요가 없습니다 (is_valid == 0xFF).
        return true;
    }
}

bool journal_backup_sector(uint32_t lba, const uint8_t *sector_data)
{
    g_journal_record.is_valid = 0xAA;
    g_journal_record.sector_lba = lba;
    
    for (int i = 0; i < JOURNAL_SECTOR_SIZE; i++)
    {
        g_journal_record.data[i] = sector_data[i];
    }
    

    if (write_fram_record(&g_journal_record))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool journal_commit(void)
{

    uint8_t commit_flag = 0xFF;
#if (JOURNAL_USE == 1)
    drv_fram_write(JOURNAL_FRAM_BASE_ADDR,(uint8_t *) &commit_flag, sizeof(uint8_t));
#endif
    g_journal_record.is_valid = 0xFF; 
    return true;

}