/**
 * @file test_rs485.h
 * @brief RS485 다중 포트 동시 송수신 테스트 헤더
 */

#ifndef TEST_RS485_H
#define TEST_RS485_H

/**
 * @brief RS485 멀티포트 동시 테스트 (멀티 태스크 방식)
 * @details 4개 RS485 포트(RS485_C, RS485_D, RS485_A, RS485_B)를 각각 별도 태스크로 생성하여
 *          동시 송수신 테스트를 수행합니다.
 *          - 각 태스크는 1초마다 포트명을 송신
 *          - 0x0A(LF) 수신 시 데이터 에코
 *          - CTRL+C로 테스트 종료
 *          - 주의: RS485_A, RS485_B는 하드웨어 점퍼 설정 필요
 */
void test_rs485(void);

/**
 * @brief RS485 폴링 기반 테스트 (단일 태스크 방식)
 * @details 태스크를 생성하지 않고 단일 루프에서 모든 RS485 포트를 순차 폴링하여
 *          송수신 테스트를 수행합니다.
 *          - 메모리 절약 및 간단한 디버깅에 유리
 *          - 1초마다 모든 포트에 데이터 송신
 *          - 0x0A(LF) 수신 시 데이터 에코
 *          - CTRL+C로 테스트 종료
 */
void test_rs485_polling(void);

#endif /* TEST_RS485_H */
