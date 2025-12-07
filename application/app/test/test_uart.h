/**
 * @file test_uart.h
 * @brief UART 다중 포트 동시 송수신 테스트 헤더
 */

#ifndef TEST_UART_H
#define TEST_UART_H

/**
 * @brief UART 멀티포트 동시 테스트 (멀티 태스크 방식)
 * @details 8개 UART 포트(VHF, USER0-3, RS485_A/B, CDMA)를 각각 별도 태스크로 생성하여
 *          동시 송수신 테스트를 수행합니다.
 *          - 각 태스크는 1초마다 포트명을 송신
 *          - 0x0A(LF) 수신 시 데이터 에코
 *          - CTRL+C로 테스트 종료
 */
void test_uart(void);

/**
 * @brief UART 폴링 기반 테스트 (단일 태스크 방식)
 * @details 태스크를 생성하지 않고 단일 루프에서 모든 UART 포트를 순차 폴링하여
 *          송수신 테스트를 수행합니다.
 *          - 메모리 절약 및 간단한 디버깅에 유리
 *          - 1초마다 모든 포트에 데이터 송신
 *          - 0x0A(LF) 수신 시 데이터 에코
 *          - CTRL+C로 테스트 종료
 */
void test_uart_polling(void);

#endif /* TEST_UART_H */
