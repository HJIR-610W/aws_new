/**
 ******************************************************************************
 * @file    at45db.h
 * @brief   AT45DB DataFlash 시리얼 플래시 메모리 드라이버 헤더
 * @details AT45DB321/641 시리얼 플래시 메모리 제어 드라이버 인터페이스
 *          - 지원 디바이스: AT45DB321, AT45DB641E
 *          - SPI 통신 인터페이스
 *          - 바이너리 페이지 모드 지원
 * @version 1.0.0
 * @date    2025-11-01
 ******************************************************************************
 */

#ifndef AT45DB_H
#define AT45DB_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief AT45DB 결과 코드
 */
typedef enum {
    AT45DB_OK = 0,              // 성공
    AT45DB_ERROR = -1,          // 오류
    AT45DB_TIMEOUT = -2,        // 타임아웃
    AT45DB_UNSUPPORTED = -3,    // 지원되지 않음
    AT45DB_INIT_FAILED = -4     // 초기화 실패
} at45db_result_t;

/**
 * @brief AT45DB 디바이스 정보 구조체
 */
typedef struct {
    uint8_t density_code;           // 밀도 코드
    uint32_t capacity_bits;         // 용량 (비트)
    uint32_t total_pages;           // 전체 페이지 수
    uint16_t page_size_standard;    // 표준 페이지 크기
    uint16_t page_size_binary;      // 바이너리 페이지 크기
    uint16_t block_size;
    const char* device_name;        // 디바이스 이름
} at45db_device_info_t;

/**
 * @brief AT45DB 칩 정보 구조체
 */
typedef struct {
    at45db_device_info_t device_info;   // 디바이스 정보
    uint16_t current_page_size;          // 현재 페이지 크기
    uint32_t block_size;
    uint32_t total_capacity_bytes;       // 전체 용량 (바이트)
    bool is_binary_mode;                 // 바이너리 모드 여부
    bool is_initialized;                 // 초기화 완료 여부
} at45db_chip_info_t;

/**
 * @brief AT45DB 초기화
 */
void at45db_init(void);

/**
 * @brief 칩 정보 가져오기
 * @param info: 칩 정보를 저장할 구조체 포인터
 * @return AT45DB_OK: 성공, 오류 코드: 실패
 */
at45db_result_t at45db_get_chip_info(at45db_chip_info_t *info);

/**
 * @brief 데이터 쓰기
 * @param offset: 쓰기 시작 오프셋
 * @param p_data: 쓸 데이터
 * @param data_len: 쓸 데이터 길이
 * @return 0: 성공
 */
int32_t at45db_write(uint32_t offset, uint8_t *p_data, uint32_t data_len);

/**
 * @brief 데이터 읽기
 * @param offset: 읽기 시작 오프셋
 * @param p_buff: 읽은 데이터를 저장할 버퍼
 * @param read_len: 읽을 데이터 길이
 */
void at45db_read(uint32_t offset, uint8_t *p_buff, uint32_t read_len);

/**
 * @brief 검증 및 재시도 기능이 있는 안전한 쓰기
 * @param offset: 쓰기 시작 오프셋
 * @param p_data: 쓸 데이터
 * @param data_len: 쓸 데이터 길이
 * @return 0: 성공, -1: 잘못된 매개변수, -2: 검증 실패
 */
int32_t at45db_write_safe(uint32_t offset, uint8_t *p_data, uint32_t data_len);


void at45db_read_page(uint32_t read_addr, uint8_t *read_buff);
void at45db_write_page(uint32_t write_addr, uint8_t *write_buff);;

int at45db_lfs_read(uint32_t block, uint32_t off, uint8_t *buffer, uint32_t size);
int at45db_lfs_prog(uint32_t block, uint32_t off, const uint8_t *buffer, uint32_t size);
int at45db_lfs_erase(uint32_t block);

/**
 * @brief RAM처럼 연속 쓰기 가능한 고급 쓰기 함수
 * @param offset: 쓰기 시작 오프셋 (절대 주소)
 * @param p_data: 쓸 데이터
 * @param data_len: 쓸 데이터 길이
 * @return 0: 성공, -1: 실패
 */
int32_t at45db_write_adv(uint32_t offset, uint8_t *p_data, uint32_t data_len);

/**
 * @brief RAM처럼 연속 읽기 가능한 고급 읽기 함수
 * @param address: 읽기 시작 주소 (절대 주소)
 * @param buffer: 읽은 데이터를 저장할 버퍼
 * @param size: 읽을 데이터 길이
 * @return 0: 성공, -1: 실패
 */
int32_t at45db_read_adv(uint32_t address, uint8_t *buffer, uint32_t size);

#endif