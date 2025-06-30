/**
 * @file driver_di.h
 * @brief 디지털 입력 드라이버 인터페이스
 * @version 2.1
 * @date 2024
 */

#ifndef DRIVER_DI_H
#define DRIVER_DI_H

#include "driver_di_def.h"
#include "driver_interface.h"
#include <stdbool.h>

// ============================================================================
// 디지털 입력 채널 정의
// ============================================================================

// STM32 네이티브 GPIO 채널
#define DI_0_ADC_RDY    0           ///< ADC 준비 신호
#define DI_1_RTC_IRQ    1           ///< RTC 인터럽트
#define DI_RAIN_REED    2           ///< 강우량계 리드 스위치
#define DI_RAIN_HALL    3           ///< 강우량계 홀 센서
#define DI_RAIN_HALL_ERR 4          ///< 강우 홀 센서 에러
#define DI_QUAD_UARTA_1 5           ///< 쿼드 UART A1 인터럽트
#define DI_QUAD_UARTB_2 6           ///< 쿼드 UART B2 인터럽트
#define DI_QUAD_UARTC_3 7           ///< 쿼드 UART C3 인터럽트
#define DI_QUAD_UARTD_4 8           ///< 쿼드 UART D4 인터럽트
#define DI_QUAD_UARTA_5 9           ///< 쿼드 UART A5 인터럽트
#define DI_QUAD_UARTB_6 10          ///< 쿼드 UART B6 인터럽트
#define DI_QUAD_UARTC_7 11          ///< 쿼드 UART C7 인터럽트
#define DI_QUAD_UARTD_8 12          ///< 쿼드 UART D8 인터럽트
#define DI_HART_CD      13          ///< HART 캐리어 감지
#define DI_USER_BTN     14          ///< 사용자 인터페이스 버튼
#define DI_BTM_STATUS   15          ///< 블루투스 모듈 상태
#define DI_RAIN_DETECT  16          ///< 강우 감지 센서

// 외부 입력 채널 (PCF8575)
#define DI_EXT_0        100         ///< 외부 입력 0 (도어 센서)
#define DI_EXT_1        101         ///< 외부 입력 1
#define DI_EXT_2        102         ///< 외부 입력 2
#define DI_EXT_3        103         ///< 외부 입력 3
#define DI_EXT_4        105         ///< 외부 입력 4
#define DI_EXT_5        106         ///< 외부 입력 5

// ============================================================================
// 공개 API 함수들
// ============================================================================

/**
 * @brief 논리적 채널 번호로 DI 드라이버 열기
 * 
 * @param logical_channel 논리적 DI 채널 번호 (DI_xxx 상수 사용)
 * @param opt 선택적 설정 매개변수
 * @return 드라이버 인스턴스 포인터 또는 에러 시 NULL
 */
driver_t *driver_di_open(uint32_t logical_channel, void *opt);

/**
 * @brief 채널 이름으로 DI 드라이버 열기
 * 
 * @param channel_name 채널 이름
 * @param opt 선택적 설정 매개변수  
 * @return 드라이버 인스턴스 포인터 또는 에러 시 NULL
 */
driver_t *driver_di_open_by_name(const char *channel_name, void *opt);

/**
 * @brief DI 드라이버 인스턴스 닫기
 * 
 * @param drv 드라이버 인스턴스 포인터
 */
void driver_di_close(driver_t *drv);

/**
 * @brief 디지털 입력 상태 읽기
 * 
 * @param drv 드라이버 인스턴스 포인터
 * @return 입력 상태 (0=LOW, 1=HIGH) 또는 음수 에러 코드
 */
int32_t driver_di_read(driver_t *drv);

/**
 * @brief DI 드라이버 설정
 * 
 * @param drv 드라이버 인스턴스 포인터
 * @param cmd 설정 명령
 * @param option 설정 옵션 데이터
 */
void driver_di_set(driver_t *drv, di_set_option_t cmd, void *option);

/**
 * @brief 디바운싱과 함께 입력 읽기
 * 
 * @param drv 드라이버 인스턴스 포인터
 * @param debounce_ms 디바운스 시간 (밀리초)
 * @return 디바운스된 입력 상태 (0=LOW, 1=HIGH) 또는 음수 에러 코드
 */
int32_t driver_di_read_debounced(driver_t *drv, uint32_t debounce_ms);

/**
 * @brief 버튼 누름 감지 (홀드 타임 지원)
 * 
 * @param drv 드라이버 인스턴스 포인터
 * @param hold_time_ms 필요한 홀드 시간 (밀리초) (0 = 단순 누름 감지)
 * @param debounce_ms 디바운스 시간 (밀리초)
 * @return 조건이 만족되면 true, 아니면 false
 */
bool driver_di_is_low(driver_t *drv, uint32_t hold_time_ms, uint32_t debounce_ms);

#endif /* DRIVER_DI_H */