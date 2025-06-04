
#include "driver_di.h"

#include "cmsis_os.h"
#include "driver_stm32_di.h"
#include "pcf8575.h"
#include "os_user_def.h"
driver_t *driver_di_open(uint32_t num, void *opt)
{
  driver_t *driver = NULL;

  switch (num)
  {
    case DI_0_ADC_RDY:
      driver = stm32_di_open(STM32_DI_0_ADC_RDY, opt);
      break;
    case DI_1_RTC_IRQ:
      driver = stm32_di_open(STM32_DI_1_RTC_IRQ, opt);
      break;
    case DI_RAIN_REED:
      driver = stm32_di_open(STM32_DI_RAIN_REED, opt);
      break;
    case DI_RAIN_HALL:
      driver = stm32_di_open(STM32_DI_RAIN_HALL, opt);
      break;
    case DI_RAIN_HALL_ERR:
      driver = stm32_di_open(STM32_DI_RAIN_HALL_ERR, opt);
      break;
    case DI_QUAD_UARTA_1:
      driver = stm32_di_open(STM32_DI_QUAD_UARTA_1, opt);
      break;
    case DI_QUAD_UARTB_2:
      driver = stm32_di_open(STM32_DI_QUAD_UARTB_2, opt);
      break;
    case DI_QUAD_UARTC_3:
      driver = stm32_di_open(STM32_DI_QUAD_UARTC_3, opt);
      break;
    case DI_QUAD_UARTD_4:
      driver = stm32_di_open(STM32_DI_QUAD_UARTD_4, opt);
      break;
    case DI_QUAD_UARTA_5:
      driver = stm32_di_open(STM32_DI_QUAD_UARTA_5, opt);
      break;
    case DI_QUAD_UARTB_6:
      driver = stm32_di_open(STM32_DI_QUAD_UARTB_6, opt);
      break;
    case DI_QUAD_UARTC_7:
      driver = stm32_di_open(STM32_DI_QUAD_UARTC_7, opt);
      break;
    case DI_QUAD_UARTD_8:
      driver = stm32_di_open(STM32_DI_QUAD_UARTD_8, opt);
      break;
    case DI_EXT_0:
      driver = pcf8575_di_open(DI_PCF8575_0, 0);
      break;
    case DI_EXT_1:
      driver = pcf8575_di_open(DI_PCF8575_1, 0);
      break;
    case DI_EXT_2:
      driver = pcf8575_di_open(DI_PCF8575_2, 0);
      break;
    case DI_EXT_3:
      driver = pcf8575_di_open(DI_PCF8575_3, 0);
      break;
    case DI_EXT_4:
      driver = pcf8575_di_open(DI_PCF8575_4, 0);
      break;
    case DI_EXT_5:
      driver = pcf8575_di_open(DI_PCF8575_5, 0);
      break;

    case DI_HART_CD:
      driver = stm32_di_open(STM32_DI_HART_CD, opt);
      break;
    case DI_USER_BTN:
      driver = stm32_di_open(STM32_DI_USER_BTN, opt);
      break;
    case DI_BTM_STATUS:
      driver = stm32_di_open(STM32_DI_BTM_STATUS, opt);
      break;
    case DI_RAIN_DETECT:
      driver = stm32_di_open(STM32_DI_RAIN_DETECT, opt);
      break;
  }

  return driver;
}

void driver_di_close(driver_t *drv)
{
  const di_api_t *api = drv->api;

  api->close(drv);
}

int32_t driver_di_read(driver_t *drv)
{
  const di_api_t *api = drv->api;

  return api->read(drv);
}

void driver_di_set(driver_t *drv, di_set_option_t cmd, void *option)
{
  const di_api_t *api = drv->api;

  api->set(drv, cmd, option);
}



#define INPUT_LOW 0
#define INPUT_HIGH 1

typedef enum
{
  eFSM_STATE_IDLE,               // 초기 상태, 버튼 눌림 대기
  eFSM_STATE_DEBOUNCING_PRESS,   // 버튼 눌림 감지, 디바운싱 진행 중
  eFSM_STATE_CONFIRMED_PRESS,    // 안정적인 버튼 눌림 확인, 홀드 시간 카운트 중
  eFSM_STATE_DEBOUNCING_RELEASE  // 버튼 떼어짐 감지 (홀드 중), 디바운싱 진행 중
} eINPUT_STATE_t;

bool is_input_low_long(driver_t *di, uint32_t hold_time_ms, uint32_t debounce_ms)
{
  eINPUT_STATE_t current_fsm_state = eFSM_STATE_IDLE;
  uint32_t event_start_tick = 0;          
  uint32_t stable_press_confirm_tick = 0; 

  // 폴링 간격 (debounce_ms 보다 충분히 작아야 함)
  const uint32_t polling_interval_ms = 5;
  if(driver_di_read(di)==INPUT_HIGH)
  {
    return false;
  }
  while (true)
  {
    uint32_t current_tick = OS_GET_TICK();
    uint8_t button_raw_state = driver_di_read(di);

    switch (current_fsm_state)
    {
      case eFSM_STATE_IDLE:
        if (button_raw_state == INPUT_LOW)
        {
          current_fsm_state = eFSM_STATE_DEBOUNCING_PRESS;
          event_start_tick = current_tick;  // 디바운싱 시작 시간 기록
        }
        break;

      case eFSM_STATE_DEBOUNCING_PRESS:
        if (button_raw_state == INPUT_HIGH)
        {
          // 디바운싱 중 버튼이 떼어짐 (바운스 또는 짧은 클릭)
          current_fsm_state = eFSM_STATE_IDLE;
        }
        else if ((current_tick - event_start_tick) >= debounce_ms)
        {
          // 디바운싱 시간 동안 계속 눌림 상태 유지 -> 안정적인 눌림으로 간주
          current_fsm_state = eFSM_STATE_CONFIRMED_PRESS;
          stable_press_confirm_tick = current_tick;  // 안정적인 눌림 확인 시간 기록

          if (hold_time_ms == 0)//단지 눌렀는지 안눌러졌는지만 확인인
          {
            return true;
          }
        }
        break;

      case eFSM_STATE_CONFIRMED_PRESS:
        if (button_raw_state == INPUT_HIGH)
        {
          // 홀드 시간 카운트 중 버튼이 떼어짐
          current_fsm_state = eFSM_STATE_DEBOUNCING_RELEASE;
          event_start_tick = current_tick;  // 떼어짐 디바운싱 시작 시간 기록
        }
        else
        {
          // 계속 눌려있는 상태, 홀드 시간 충족 확인
          if ((current_tick - stable_press_confirm_tick) >= hold_time_ms)
          {
            return true;  // 장시간 눌림 성공
          }
        }
        break;

      case eFSM_STATE_DEBOUNCING_RELEASE:
        if (button_raw_state == INPUT_LOW)
        {
          // 떼어짐 디바운싱 중 다시 눌림 (바운스) -> 이전의 안정된 눌림 상태로 복귀
          current_fsm_state = eFSM_STATE_CONFIRMED_PRESS;
          // event_start_tick은 CONFIRMED_PRESS로 돌아갈 때 특별히 갱신할 필요 없음
          // stable_press_confirm_tick은 유효하게 유지됨
        }
        else if ((current_tick - event_start_tick) >= debounce_ms)
        {
          // 디바운싱 시간 동안 계속 떼어진 상태 유지 -> 안정적인 떼어짐으로 간주
          // 장시간 눌림 조건을 만족하지 못하고 버튼이 떼어졌으므로 false 반환
          return false;
        }
        break;

      default:
        // 비정상 상태, 초기화
        current_fsm_state = eFSM_STATE_IDLE;
        break;
    }

    osDelay(polling_interval_ms);  // CPU 사용량 감소를 위한 폴링 지연
  }
}
bool driver_di_is_low(driver_t *di,uint32_t hold_time_ms,uint32_t debounce_ms)
{
  return is_input_low_long(di,hold_time_ms,debounce_ms);
}