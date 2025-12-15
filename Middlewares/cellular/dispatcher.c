/**
 * @file dispatcher.c
 * @brief AT 응답 디스패처 구현
 * @version 1.0.0
 * @date 2025-12-13
 */

#include "dispatcher.h"

#include <string.h>
#include <stdio.h>

#include "cellular_api.h"
#include "cmsis_os2.h"

#define DISP_MAX_SUBS 8

/**
 * @brief 보류 중인 동기 명령 요청을 위한 구조체
 */
typedef struct {
  int active; // 요청 활성 여부
  const char* const* p_ack_list; // ACK 목록 포인터
  uint32_t ack_list_cnt;         // ACK 목록 개수
  uint32_t* p_matched_index;     // 일치하는 ACK의 인덱스를 저장할 포인터
  uint8_t *resp_buf;             // 응답 버퍼 포인터
  size_t resp_buf_size;          // 응답 버퍼 크기
  size_t resp_len;               // 수신된 응답 길이
  osSemaphoreId_t sem;           // 응답 대기를 위한 세마포어 ID
} pending_t;

/**
 * @brief 비동기 구독자 정보를 위한 구조체
 */
typedef struct {
  int active; // 구독 활성 여부
  char prefix[32]; // 구독할 접두사
  void (*cb)(const uint8_t*, size_t, void*); // 콜백 함수 포인터
  void *ctx; // 사용자가 정의한 컨텍스트 포인터로, 콜백 함수 호출 시 함께 전달됩니다.
} sub_t;

static pending_t s_pending_sync_slot; // 동기 명령을 위한 단일 보류 슬롯
static sub_t s_subs[DISP_MAX_SUBS]; // 비동기 구독자 목록
static osMutexId_t s_lock = NULL; // 디스패처 내부 접근 보호를 위한 뮤텍스
static osMutexId_t s_modem_lock = NULL; // 모뎀 AT 명령 전송 보호를 위한 뮤텍스
static int s_initialized = 0; // 디스패처 초기화 여부

/**
 * @brief 디스패처 초기화
 * @return 성공 시 0, 실패 시 -1
 */
int dispatcher_init(void)
{
    int i;

    if (s_initialized) {
        return (s_lock && s_modem_lock) ? 0 : -1;
    }

    // 내부 접근 보호 뮤텍스 생성
    s_lock = osMutexNew(NULL);
    if (!s_lock) return -1;

    // 모뎀 AT 명령 전송 보호 뮤텍스 생성
    s_modem_lock = osMutexNew(NULL);
    if (!s_modem_lock) {
        osMutexDelete(s_lock);
        s_lock = NULL;
        return -1;
    }

    // 단일 보류 슬롯 초기화
    s_pending_sync_slot.active = 0;
    s_pending_sync_slot.sem = NULL;
    s_pending_sync_slot.resp_buf = NULL;

    // 비동기 구독자 목록 초기화
    for (i = 0; i < DISP_MAX_SUBS; i++) {
        s_subs[i].active = 0;
    }

    s_initialized = 1;
    return 0;
}

/**
 * @brief 수신된 프레임 처리 (AT 파서에서 호출됨)
 * @param p_data 수신된 데이터 버퍼 포인터
 * @param len 데이터 길이
 */
void dispatcher_handle_frame(const uint8_t* p_data, size_t len)
{
  // dispatcher_init()은 at_task 실행 전에 한 번 호출된다고 가정합니다.
  if (!s_lock) return; // 초기화가 제대로 되었다면 발생하지 않아야 함.

  osMutexAcquire(s_lock, osWaitForever);

  // 단일 보류 중인 동기 요청과 일치하는지 먼저 확인
  if (s_pending_sync_slot.active) {
      // 보류 슬롯의 ACK 목록에 있는 접두사 중 하나와 일치하는지 확인
      for (uint32_t j = 0; j < s_pending_sync_slot.ack_list_cnt; j++) {
          const char* prefix = s_pending_sync_slot.p_ack_list[j];
          if (prefix == NULL || prefix[0] == '\0') { // 와일드카드 일치
              // 와일드카드의 경우, 일단 수락하되 더 구체적인 일치가 있는지 계속 확인합니다.
              // 이 로직은 복잡할 수 있으며, 현재는 첫 번째 일치 항목이 우선합니다.
          }
          if (strncmp((const char*)p_data, prefix, strlen(prefix)) == 0) {
              // 일치하는 항목을 찾았습니다!
              size_t copy_len = (len < s_pending_sync_slot.resp_buf_size) ? len : s_pending_sync_slot.resp_buf_size;
              if (s_pending_sync_slot.resp_buf && copy_len > 0) {
                  memcpy(s_pending_sync_slot.resp_buf, p_data, copy_len);
              }
              s_pending_sync_slot.resp_len = copy_len;
              
              if (s_pending_sync_slot.p_matched_index) {
                  *s_pending_sync_slot.p_matched_index = j;
              }

              s_pending_sync_slot.active = 0;
              if (s_pending_sync_slot.sem) osSemaphoreRelease(s_pending_sync_slot.sem);
              osMutexRelease(s_lock);
              return;
          }
      }
      // 특정 접두사 일치를 찾지 못한 슬롯에 대한 와일드카드 일치 처리
      if (s_pending_sync_slot.ack_list_cnt == 0 || (s_pending_sync_slot.ack_list_cnt == 1 && (s_pending_sync_slot.p_ack_list[0] == NULL || s_pending_sync_slot.p_ack_list[0][0] == '\0'))) {
          size_t copy_len = (len < s_pending_sync_slot.resp_buf_size) ? len : s_pending_sync_slot.resp_buf_size;
          if (s_pending_sync_slot.resp_buf && copy_len > 0) {
              memcpy(s_pending_sync_slot.resp_buf, p_data, copy_len);
          }
          s_pending_sync_slot.resp_len = copy_len;
          if (s_pending_sync_slot.p_matched_index) {
              *s_pending_sync_slot.p_matched_index = 0;
          }
          s_pending_sync_slot.active = 0;
          if (s_pending_sync_slot.sem) osSemaphoreRelease(s_pending_sync_slot.sem);
          osMutexRelease(s_lock);
          return;
      }
  }

  // 보류 중인 일치 없음: 구독자에게 발행
  for (int i = 0; i < DISP_MAX_SUBS; i++) {
    if (s_subs[i].active) {
      if (s_subs[i].prefix[0] == '\0' ||
          strncmp((const char*)p_data, s_subs[i].prefix, strlen(s_subs[i].prefix)) == 0) {
        
        void (*p_cb)(const uint8_t*, size_t, void*) = s_subs[i].cb;
        void *p_ctx = s_subs[i].ctx;
        osMutexRelease(s_lock);
        if (p_cb) p_cb(p_data, len, p_ctx);
        return;
      }
    }
  }

  osMutexRelease(s_lock);
}

/**
 * @brief 비동기 명령 전송 (보내고 잊음).
 * @param p_cmd 명령 버퍼 포인터
 * @param cmd_len 명령 길이
 * @return 성공 시 0, 실패 시 -1
 */
int dispatcher_send_async(const uint8_t* p_cmd, size_t cmd_len)
{
  cellular_if_t* p_if = cellular_get_interface();
  if (!p_if || p_if->uart_io.send == NULL) return -1;
  p_if->uart_io.send(p_if->uart_handle, p_cmd, (int32_t)cmd_len);
  return 0;
}

/**
 * @brief 동기 명령 전송: 명령 바이트를 보내고 접두사와 일치하는 응답을 기다림
 * @param p_cmd 명령 버퍼 포인터
 * @param cmd_len 명령 길이
 * @param p_ack_list ACK 접두사 목록 (문자열 배열)
 * @param ack_list_cnt ACK 목록 개수
 * @param p_matched_index 일치하는 ACK의 인덱스를 저장할 포인터
 * @param p_resp_buf 응답 버퍼 포인터
 * @param resp_buf_size 응답 버퍼 크기
 * @param timeout_ms 타임아웃 시간 (밀리초)
 * @return 응답 버퍼에 복사된 바이트 수 (>0) 또는 타임아웃/오류 시 -1
 */
int dispatcher_send_sync(const uint8_t* p_cmd, size_t cmd_len, const char* const* p_ack_list, uint32_t ack_list_cnt, uint32_t* p_matched_index,
                         uint8_t* p_resp_buf, size_t resp_buf_size, uint32_t timeout_ms)
{
  int32_t return_code = -1;
  osStatus_t status;
  int modem_lock_acquired = 0;

  // dispatcher_init()은 at_task 실행 전에 호출된다고 가정합니다.

  if (!s_modem_lock) return -1;

  // 모뎀 전송 뮤텍스 획득 (모든 동기 AT 명령을 직렬화)
  status = osMutexAcquire(s_modem_lock, osWaitForever);
  if (status != osOK) return -1;
  modem_lock_acquired = 1;

  // 디스패처 내부 상태 뮤텍스 획득
  osMutexAcquire(s_lock, osWaitForever);

  // 단일 보류 슬롯이 활성 상태인지 확인
  if (s_pending_sync_slot.active) {
    osMutexRelease(s_lock);
    goto exit; // 다른 동기 명령이 이미 보류 중 (modem_lock이 유효하다면 발생하지 않아야 함)
  }

  // 보류 슬롯 초기화
  s_pending_sync_slot.active = 1;
  s_pending_sync_slot.p_ack_list = p_ack_list;
  s_pending_sync_slot.ack_list_cnt = ack_list_cnt;
  s_pending_sync_slot.p_matched_index = p_matched_index;
  s_pending_sync_slot.resp_buf = p_resp_buf;
  s_pending_sync_slot.resp_buf_size = resp_buf_size;
  s_pending_sync_slot.resp_len = 0;
  s_pending_sync_slot.sem = osSemaphoreNew(1, 0, NULL);
  if (s_pending_sync_slot.sem == NULL) {
    s_pending_sync_slot.active = 0;
    osMutexRelease(s_lock);
    goto exit;
  }

  // 명령 전송
  {
    cellular_if_t* p_if = cellular_get_interface();
    if (!p_if || p_if->uart_io.send == NULL) {
      osSemaphoreDelete(s_pending_sync_slot.sem);
      s_pending_sync_slot.sem = NULL;
      s_pending_sync_slot.active = 0;
      osMutexRelease(s_lock);
      return_code = -1;
      goto exit;
    }
    p_if->uart_io.send(p_if->uart_handle, p_cmd, (int32_t)cmd_len);
  }

  osMutexRelease(s_lock);

  // 응답 대기
  if (osSemaphoreAcquire(s_pending_sync_slot.sem, timeout_ms) == osOK) {
    return_code = (int)s_pending_sync_slot.resp_len;
  } else {
    // 타임아웃
    return_code = -1;
    osMutexAcquire(s_lock, osWaitForever);
    s_pending_sync_slot.active = 0; // 타임아웃 시 비활성화
    osMutexRelease(s_lock);
  }

  // 세마포어 정리
  if (s_pending_sync_slot.sem) {
    osSemaphoreDelete(s_pending_sync_slot.sem);
    s_pending_sync_slot.sem = NULL;
  }

exit:
  if (modem_lock_acquired) {
    osMutexRelease(s_modem_lock);
  }
  return return_code;
}

/**
 * @brief 명령 전송 없이 모든 수신 프레임 대기
 * @param p_resp_buf 응답 버퍼 포인터
 * @param resp_buf_size 응답 버퍼 크기
 * @param timeout_ms 타임아웃 시간 (밀리초)
 * @return 수신된 바이트 수 (>0) 또는 타임아웃/오류 시 -1
 */
int dispatcher_wait_any(uint8_t* p_resp_buf, size_t resp_buf_size, uint32_t timeout_ms)
{
  int32_t return_code = -1;

  // dispatcher_init()은 at_task 실행 전에 호출된다고 가정합니다.

  osMutexAcquire(s_lock, osWaitForever);

  // 단일 보류 슬롯이 활성 상태인지 확인
  if (s_pending_sync_slot.active) {
    osMutexRelease(s_lock);
    return -1; // 다른 동기 명령이 이미 보류 중이거나 dispatcher_send_sync가 사용 중입니다.
  }

  s_pending_sync_slot.active = 1;
  s_pending_sync_slot.resp_buf = p_resp_buf;
  s_pending_sync_slot.resp_buf_size = resp_buf_size;
  s_pending_sync_slot.resp_len = 0;
  s_pending_sync_slot.p_ack_list = NULL;
  s_pending_sync_slot.ack_list_cnt = 0;
  s_pending_sync_slot.p_matched_index = NULL;
  s_pending_sync_slot.sem = osSemaphoreNew(1, 0, NULL);
  if (s_pending_sync_slot.sem == NULL) {
    s_pending_sync_slot.active = 0;
    osMutexRelease(s_lock);
    return -1;
  }

  osMutexRelease(s_lock);

  if (osSemaphoreAcquire(s_pending_sync_slot.sem, timeout_ms) == osOK) {
    return_code = (int)s_pending_sync_slot.resp_len;
  } else {
    return_code = -1;
    osMutexAcquire(s_lock, osWaitForever);
    s_pending_sync_slot.active = 0;
    osMutexRelease(s_lock);
  }

  if (s_pending_sync_slot.sem) {
    osSemaphoreDelete(s_pending_sync_slot.sem);
    s_pending_sync_slot.sem = NULL;
  }

  return return_code;
}

/**
 * @brief URC (비동기 응답) 구독
 * @param p_prefix 구독할 URC 접두사
 * @param p_cb 콜백 함수 포인터
 * @param p_ctx 콜백 함수에 전달될 컨텍스트 포인터
 * @return 구독 ID 또는 -1 (실패)
 */
int dispatcher_subscribe(const char* p_prefix, void (*p_cb)(const uint8_t*, size_t, void*), void* p_ctx)
{
  int i;
  // dispatcher_init()은 at_task 실행 전에 호출된다고 가정합니다.

  // 사용자 요청에 따라 접두사가 NULL인 경우 즉시 반환
  if (p_prefix == NULL) {
      return -1;
  }
  
  osMutexAcquire(s_lock, osWaitForever);
  for (i = 0; i < DISP_MAX_SUBS; i++) {
    if (!s_subs[i].active) {
      s_subs[i].active = 1;
      s_subs[i].cb = p_cb;
      s_subs[i].ctx = p_ctx;
      if (p_prefix) strncpy(s_subs[i].prefix, p_prefix, 32-1);
      else s_subs[i].prefix[0] = '\0';
      s_subs[i].prefix[32-1] = '\0';
      osMutexRelease(s_lock);
      return i;
    }
  }
  osMutexRelease(s_lock);
  return -1;
}

/**
 * @brief URC 구독 해지
 * @param sub_id 구독 ID
 */
void dispatcher_unsubscribe(int sub_id)
{
  if (!s_lock) return;
  if (sub_id < 0 || sub_id >= DISP_MAX_SUBS) return;
  osMutexAcquire(s_lock, osWaitForever);
  s_subs[sub_id].active = 0;
  s_subs[sub_id].cb = NULL;
  s_subs[sub_id].ctx = NULL;
  s_subs[sub_id].prefix[0] = '\0';
  osMutexRelease(s_lock);
}