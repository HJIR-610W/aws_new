#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------
 * 통신 타입 정의
 * ------------------------------------------ */
typedef enum
{
  IO_COM_TYPE_NONE = 0,
  IO_COM_TYPE_RS232,
  IO_COM_TYPE_RS485,
  IO_COM_TYPE_TCP,
    IO_COM_TYPE_TELNET
} io_com_type_t;

/* ------------------------------------------
 * 공통 반환 상태 코드
 *  0  : 성공
 * <0 : 에러
 * ------------------------------------------ */
typedef enum
{
  IO_STATUS_OK             = 0,
  IO_STATUS_INVALID_PARAM  = -1,
  IO_STATUS_NOT_INITIALIZED= -2,
  IO_STATUS_NOT_SUPPORTED  = -3,
  IO_STATUS_TIMEOUT        = -4,
  IO_STATUS_IO_ERROR       = -5,
} io_status_t;

struct io_if;

/* ------------------------------------------
 * IO 연산 테이블 (vtable)
 *  각 드라이버가 구현해야 할 함수 포인터 집합
 * ------------------------------------------ */
typedef struct
{
  int (*send)(struct io_if *io,
              const uint8_t *data,
              size_t len);

  int (*recv)(struct io_if *io,
              uint8_t *data,
              size_t len,
              uint32_t timeout_ms);

  void (*flush)(struct io_if *io);

  int (*ioctl)(struct io_if *io,
               uint32_t cmd,
               void *arg);
  void (*inject)(struct io_if *io,
              uint8_t *data,
              size_t len);
} io_ops_t;

/* ------------------------------------------
 * IO 인터페이스 핸들
 *  - type    : 통신 타입 (RS232, RS485, TCP 등)
 *  - ops     : 실제 드라이버 함수 테이블
 *  - context : 드라이버별 private 데이터 포인터
 * ------------------------------------------ */
typedef struct io_if
{
  io_com_type_t type;
  const io_ops_t *ops;
  void *context;
  int dev_num;
  uint8_t initialized;
} io_if_t;

/* ------------------------------------------
 * Public API
 * ------------------------------------------ */

/* 핸들 초기화 */
int io_init(io_if_t *io,
            io_com_type_t type,
            const io_ops_t *ops,
            int dev_num);

/* 송신 (블로킹/논블로킹 여부는 드라이버 구현에 따름)
 * 반환값 : 전송한 바이트 수(>=0) 또는 음수 에러코드(io_status_t) */
int io_send(io_if_t *io,
            const uint8_t *data,
            size_t len);

/* 수신
 * 반환값 : 수신한 바이트 수(>=0) 또는 음수 에러코드(io_status_t) */
int io_recv(io_if_t *io,
            uint8_t *data,
            size_t len,
            uint32_t timeout_ms);

/* 출력 버퍼 flush (필요 없으면 드라이버에서 NULL 구현 가능) */
void io_flush(io_if_t *io);

/* 드라이버별 확장 기능 (baud 변경 등)
 * cmd / arg 포맷은 각 드라이버가 정의 */
int io_ioctl(io_if_t *io,
             uint32_t cmd,
             void *arg);

void io_inject(struct io_if *io,uint8_t *data,size_t len);

/* 초기화 여부 확인 헬퍼 (0: not initialized, 1: initialized) */
int io_is_initialized(const io_if_t *io);

#ifdef __cplusplus
}
#endif

#endif /* IO_INTERFACE_H */
