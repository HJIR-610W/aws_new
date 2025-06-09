/**
 * @file driver_di.c
 * @brief 디지털 입력 드라이버 라우팅 레이어
 * @version 2.1
 * @date 2024
 * 
 * @details 논리적 DI 채널을 적절한 백엔드 드라이버로 라우팅하는 단순한 구현
 * 
 * @author wth
 */

#include "driver_di.h"
#include "driver_stm32_di.h"
#include "pcf8575.h"
#include "cmsis_os2.h"
#include <string.h>

// ============================================================================
// 상수 정의
// ============================================================================

#define INPUT_LOW                   0
#define INPUT_HIGH                  1
#define DEFAULT_POLLING_INTERVAL_MS 5

// ============================================================================
// 채널 맵핑 구조체
// ============================================================================

/**
 * @brief 백엔드 드라이버 타입
 */
typedef enum {
    DI_BACKEND_STM32_GPIO = 0,         ///< STM32 네이티브 GPIO
    DI_BACKEND_PCF8575                 ///< PCF8575 I2C 확장기
} di_backend_type_t;

/**
 * @brief DI 채널 맵핑 엔트리
 */
typedef struct {
    uint32_t logical_channel;          ///< 논리적 DI 채널 번호
    di_backend_type_t backend_type;    ///< 백엔드 드라이버 타입
    uint32_t physical_channel;         ///< 백엔드 드라이버의 물리적 채널
    const char *name;                  ///< 채널 이름
} di_channel_map_t;

/**
 * @brief DI 채널 맵핑 테이블
 * 
 * @note 새로운 채널을 추가하거나 맵핑을 변경하려면 이 테이블만 수정하면 됩니다.
 */
static const di_channel_map_t s_di_channel_map[] = {
    // STM32 GPIO 채널들
    { DI_0_ADC_RDY,     DI_BACKEND_STM32_GPIO, STM32_DI_0_ADC_RDY,     "ADC_READY" },
    { DI_1_RTC_IRQ,     DI_BACKEND_STM32_GPIO, STM32_DI_1_RTC_IRQ,     "RTC_IRQ" },
    { DI_RAIN_REED,     DI_BACKEND_STM32_GPIO, STM32_DI_RAIN_REED,     "RAIN_REED" },
    { DI_RAIN_HALL,     DI_BACKEND_STM32_GPIO, STM32_DI_RAIN_HALL,     "RAIN_HALL" },
    { DI_RAIN_HALL_ERR, DI_BACKEND_STM32_GPIO, STM32_DI_RAIN_HALL_ERR, "RAIN_HALL_ERR" },
    { DI_QUAD_UARTA_1,  DI_BACKEND_STM32_GPIO, STM32_DI_QUAD_UARTA_1,  "QUAD_UART_A1" },
    { DI_QUAD_UARTB_2,  DI_BACKEND_STM32_GPIO, STM32_DI_QUAD_UARTB_2,  "QUAD_UART_B2" },
    { DI_QUAD_UARTC_3,  DI_BACKEND_STM32_GPIO, STM32_DI_QUAD_UARTC_3,  "QUAD_UART_C3" },
    { DI_QUAD_UARTD_4,  DI_BACKEND_STM32_GPIO, STM32_DI_QUAD_UARTD_4,  "QUAD_UART_D4" },
    { DI_QUAD_UARTA_5,  DI_BACKEND_STM32_GPIO, STM32_DI_QUAD_UARTA_5,  "QUAD_UART_A5" },
    { DI_QUAD_UARTB_6,  DI_BACKEND_STM32_GPIO, STM32_DI_QUAD_UARTB_6,  "QUAD_UART_B6" },
    { DI_QUAD_UARTC_7,  DI_BACKEND_STM32_GPIO, STM32_DI_QUAD_UARTC_7,  "QUAD_UART_C7" },
    { DI_QUAD_UARTD_8,  DI_BACKEND_STM32_GPIO, STM32_DI_QUAD_UARTD_8,  "QUAD_UART_D8" },
    { DI_HART_CD,       DI_BACKEND_STM32_GPIO, STM32_DI_HART_CD,       "HART_CD" },
    { DI_USER_BTN,      DI_BACKEND_STM32_GPIO, STM32_DI_USER_BTN,      "USER_BUTTON" },
    { DI_BTM_STATUS,    DI_BACKEND_STM32_GPIO, STM32_DI_BTM_STATUS,    "BT_STATUS" },
    { DI_RAIN_DETECT,   DI_BACKEND_STM32_GPIO, STM32_DI_RAIN_DETECT,   "RAIN_DETECT" },
    
    // PCF8575 외부 채널들
    { DI_EXT_0,         DI_BACKEND_PCF8575,    DI_PCF8575_0,           "EXT_INPUT_0" },
    { DI_EXT_1,         DI_BACKEND_PCF8575,    DI_PCF8575_1,           "EXT_INPUT_1" },
    { DI_EXT_2,         DI_BACKEND_PCF8575,    DI_PCF8575_2,           "EXT_INPUT_2" },
    { DI_EXT_3,         DI_BACKEND_PCF8575,    DI_PCF8575_3,           "EXT_INPUT_3" },
    { DI_EXT_4,         DI_BACKEND_PCF8575,    DI_PCF8575_4,           "EXT_INPUT_4" },
    { DI_EXT_5,         DI_BACKEND_PCF8575,    DI_PCF8575_5,           "EXT_INPUT_5" }
};

#define DI_CHANNEL_MAP_SIZE (sizeof(s_di_channel_map) / sizeof(s_di_channel_map[0]))

// ============================================================================
// 디바운싱용 상태 머신
// ============================================================================

typedef enum {
    FSM_STATE_IDLE,                    ///< 초기 상태, 버튼 누름 대기
    FSM_STATE_DEBOUNCING_PRESS,        ///< 버튼 누름 감지, 디바운싱 진행 중
    FSM_STATE_CONFIRMED_PRESS,         ///< 안정적인 버튼 누름 확인
    FSM_STATE_DEBOUNCING_RELEASE       ///< 버튼 놓음 감지, 디바운싱 진행 중
} fsm_state_t;

// ============================================================================
// 내부 함수들
// ============================================================================

/**
 * @brief 논리적 채널 번호로 채널 맵핑 찾기
 * 
 * @param logical_channel 논리적 채널 번호
 * @return 채널 맵핑 엔트리 포인터 또는 NULL
 */
static const di_channel_map_t *find_channel_map(uint32_t logical_channel)
{
    for (uint32_t i = 0; i < DI_CHANNEL_MAP_SIZE; i++) {
        if (s_di_channel_map[i].logical_channel == logical_channel) {
            return &s_di_channel_map[i];
        }
    }
    return NULL;
}

/**
 * @brief 채널 이름으로 채널 맵핑 찾기
 * 
 * @param channel_name 채널 이름
 * @return 채널 맵핑 엔트리 포인터 또는 NULL
 */
static const di_channel_map_t *find_channel_map_by_name(const char *channel_name)
{
    if (channel_name == NULL) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < DI_CHANNEL_MAP_SIZE; i++) {
        if (strcmp(s_di_channel_map[i].name, channel_name) == 0) {
            return &s_di_channel_map[i];
        }
    }
    return NULL;
}

/**
 * @brief 백엔드 드라이버 open 함수 호출
 * 
 * @param backend_type 백엔드 타입
 * @param physical_channel 물리적 채널 번호
 * @param opt 옵션
 * @return 드라이버 인스턴스 포인터 또는 NULL
 */
static driver_t *open_backend_driver(di_backend_type_t backend_type, 
                                     uint32_t physical_channel, 
                                     void *opt)
{
    switch (backend_type) {
        case DI_BACKEND_STM32_GPIO:
            return stm32_di_open(physical_channel, opt);
            
        case DI_BACKEND_PCF8575:
            return pcf8575_di_open(physical_channel, opt);
            
        default:
            return NULL;
    }
}

/**
 * @brief 버튼 상태 머신 구현
 * 
 * @param drv 드라이버 인스턴스
 * @param hold_time_ms 홀드 시간 (밀리초)
 * @param debounce_ms 디바운스 시간 (밀리초)
 * @return 조건 만족 시 true
 */
static bool button_state_machine(driver_t *drv, uint32_t hold_time_ms, uint32_t debounce_ms)
{
    fsm_state_t state = FSM_STATE_IDLE;
    uint32_t event_start_tick = 0;
    uint32_t press_confirm_tick = 0;
    
    // 이미 HIGH이면 즉시 false 반환
    if (driver_di_read(drv) == INPUT_HIGH) {
        return false;
    }
    
    while (true) {
        uint32_t current_tick = osKernelGetTickCount();
        int32_t read_result = driver_di_read(drv);
        
        if (read_result < 0) {
            return false;  // 읽기 에러
        }
        
        uint8_t input_state = (uint8_t)read_result;
        
        switch (state) {
            case FSM_STATE_IDLE:
                if (input_state == INPUT_LOW) {
                    state = FSM_STATE_DEBOUNCING_PRESS;
                    event_start_tick = current_tick;
                }
                break;
                
            case FSM_STATE_DEBOUNCING_PRESS:
                if (input_state == INPUT_HIGH) {
                    state = FSM_STATE_IDLE;  // 바운싱 감지
                } else if ((current_tick - event_start_tick) >= debounce_ms) {
                    state = FSM_STATE_CONFIRMED_PRESS;
                    press_confirm_tick = current_tick;
                    
                    if (hold_time_ms == 0) {
                        return true;  // 단순 누름 감지
                    }
                }
                break;
                
            case FSM_STATE_CONFIRMED_PRESS:
                if (input_state == INPUT_HIGH) {
                    state = FSM_STATE_DEBOUNCING_RELEASE;
                    event_start_tick = current_tick;
                } else {
                    if ((current_tick - press_confirm_tick) >= hold_time_ms) {
                        return true;  // 홀드 시간 조건 만족
                    }
                }
                break;
                
            case FSM_STATE_DEBOUNCING_RELEASE:
                if (input_state == INPUT_LOW) {
                    state = FSM_STATE_CONFIRMED_PRESS;  // 바운싱 감지
                } else if ((current_tick - event_start_tick) >= debounce_ms) {
                    return false;  // 확실한 릴리즈 - 홀드 시간 부족
                }
                break;
                
            default:
                state = FSM_STATE_IDLE;
                break;
        }
        
        // 무한 루프 방지 (타임아웃)
        if ((current_tick - event_start_tick) > (hold_time_ms + debounce_ms + 5000)) {
            return false;
        }
        
        osDelay(DEFAULT_POLLING_INTERVAL_MS);
    }
}

// ============================================================================
// 공개 API 구현
// ============================================================================

/**
 * @brief 논리적 채널 번호로 DI 드라이버 열기
 */
driver_t *driver_di_open(uint32_t logical_channel, void *opt)
{
    const di_channel_map_t *channel_map = find_channel_map(logical_channel);
    if (channel_map == NULL) {
        return NULL;
    }
    
    driver_t *driver = open_backend_driver(channel_map->backend_type, 
                                          channel_map->physical_channel, 
                                          opt);
    
    if (driver != NULL) {
      //  driver->type = DRIVER_TYPE_DI;
        driver->instance_id = logical_channel;
        driver->name = channel_map->name;
    }
    
    return driver;
}

/**
 * @brief 채널 이름으로 DI 드라이버 열기
 */
driver_t *driver_di_open_by_name(const char *channel_name, void *opt)
{
    const di_channel_map_t *channel_map = find_channel_map_by_name(channel_name);
    if (channel_map == NULL) {
        return NULL;
    }
    
    driver_t *driver = open_backend_driver(channel_map->backend_type, 
                                          channel_map->physical_channel, 
                                          opt);
    
    if (driver != NULL) {
        driver->type = DRIVER_TYPE_DI;
        driver->instance_id = channel_map->logical_channel;
        driver->name = channel_map->name;
    }
    
    return driver;
}

/**
 * @brief DI 드라이버 닫기
 */
void driver_di_close(driver_t *drv)
{
    if (drv == NULL || drv->api == NULL) {
        return;
    }
    
    const di_api_t *api = (const di_api_t *)drv->api;
    if (api->close != NULL) {
        api->close(drv);
    }
}

/**
 * @brief 디지털 입력 상태 읽기
 */
int32_t driver_di_read(driver_t *drv)
{
    if (drv == NULL || drv->api == NULL) {
        return -1;
    }
    
    const di_api_t *api = (const di_api_t *)drv->api;
    if (api->read == NULL) {
        return -1;
    }
    
    return api->read(drv);
}

/**
 * @brief DI 드라이버 설정
 */
void driver_di_set(driver_t *drv, di_set_option_t cmd, void *option)
{
    if (drv == NULL || drv->api == NULL) {
        return;
    }
    
    const di_api_t *api = (const di_api_t *)drv->api;
    if (api->set != NULL) {
        api->set(drv, cmd, option);
    }
}

/**
 * @brief 디바운싱과 함께 입력 읽기
 */
int32_t driver_di_read_debounced(driver_t *drv, uint32_t debounce_ms)
{
    if (drv == NULL) {
        return -1;
    }
    
    int32_t initial_state = driver_di_read(drv);
    if (initial_state < 0) {
        return initial_state;
    }
    
    if (debounce_ms == 0) {
        return initial_state;
    }
    
    uint32_t stable_start = osKernelGetTickCount();
    int32_t last_state = initial_state;
    
    while (true) {
        osDelay(DEFAULT_POLLING_INTERVAL_MS);
        
        int32_t current_state = driver_di_read(drv);
        if (current_state < 0) {
            return current_state;
        }
        
        if (current_state != last_state) {
            // 상태 변화 감지, 디바운스 타이머 재시작
            stable_start = osKernelGetTickCount();
            last_state = current_state;
        } else {
            // 상태 안정, 디바운스 시간 확인
            uint32_t elapsed = osKernelGetTickCount() - stable_start;
            if (elapsed >= debounce_ms) {
                return current_state;
            }
        }
        
        // 타임아웃 보호
        if ((osKernelGetTickCount() - stable_start) > (debounce_ms + 1000)) {
            return last_state;
        }
    }
}

/**
 * @brief 버튼 누름 감지 (홀드 타임 지원)
 */
bool driver_di_is_low(driver_t *drv, uint32_t hold_time_ms, uint32_t debounce_ms)
{
    if (drv == NULL) {
        return false;
    }
    
    return button_state_machine(drv, hold_time_ms, debounce_ms);
}