/**
 * @file test_sram.h
 * @brief SRAM 동적 메모리 할당 테스트 헤더
 */

#ifndef TEST_SRAM_H
#define TEST_SRAM_H

/**
 * @brief SRAM 테스트 메인 함수
 * @details user_malloc을 사용하여 동적 메모리를 할당하고 읽기/쓰기 테스트를 수행합니다.
 *
 *          제공 기능:
 *          1. SRAM 읽기/쓰기 테스트 - 사용자 지정 크기 및 패턴으로 테스트
 *          2. SRAM HEX DUMP - 할당된 메모리 내용을 16진수로 표시
 *          3. SRAM 다중 패턴 테스트 - 여러 패턴(0x00, 0xFF, 0xAA, 0x55, 0xCC, 0x33)으로 테스트
 *
 *          특징:
 *          - user_malloc/user_free 사용
 *          - 진행률 표시 (10% 단위)
 *          - 성능 측정 (KB/s)
 *          - 오류 감지 및 보고
 */
void test_sram(void);

#endif /* TEST_SRAM_H */
