#include "io_interface.h"

/* printf 사용할 경우를 대비한 매핑 규칙 */
#define printf debug_printf

/* 내부 헬퍼: 기본 파라미터 체크 */
static int io_validate_handle(const io_if_t *io)
{
  if (io == NULL)
  {
    return IO_STATUS_INVALID_PARAM;
  }

  if (io->initialized == 0U)
  {
    return IO_STATUS_NOT_INITIALIZED;
  }

  if (io->ops == NULL)
  {
    return IO_STATUS_NOT_INITIALIZED;
  }

  if(io->dev_num ==-1)
  {
    return IO_STATUS_NOT_INITIALIZED;
  }

  return IO_STATUS_OK;
}

/* ------------------------------------------
 * io_init
 *  - 통신 타입, ops, context 설정
 * ------------------------------------------ */
int io_init(io_if_t *io,
            io_com_type_t type,
            const io_ops_t *ops,
            int dev_num)
{
  if ((io == NULL) || (ops == NULL))
  {
    return IO_STATUS_INVALID_PARAM;
  }

  io->type        = type;
  io->ops         = ops;
  io->dev_num     = dev_num;
  io->initialized = 1U;

  return IO_STATUS_OK;
}

/* ------------------------------------------
 * io_send
 * ------------------------------------------ */
int io_send(io_if_t *io,
            const uint8_t *data,
            size_t len)
{
  int ret;

  if ((io == NULL) || (data == NULL) || (len == 0U))
  {
    return IO_STATUS_INVALID_PARAM;
  }

  ret = io_validate_handle(io);
  if (ret != IO_STATUS_OK)
  {
    return ret;
  }

  if ((io->ops->send) == NULL)
  {
    return IO_STATUS_NOT_SUPPORTED;
  }

  return io->ops->send(io, data, len);
}

/* ------------------------------------------
 * io_recv
 * ------------------------------------------ */
int io_recv(io_if_t *io,
            uint8_t *data,
            size_t len,
            uint32_t timeout_ms)
{
  int ret;

  if ((io == NULL) || (data == NULL) || (len == 0U))
  {
    return IO_STATUS_INVALID_PARAM;
  }

  ret = io_validate_handle(io);
  if (ret != IO_STATUS_OK)
  {
    return ret;
  }

  if ((io->ops->recv) == NULL)
  {
    return IO_STATUS_NOT_SUPPORTED;
  }

  return io->ops->recv(io, data, len, timeout_ms);
}

/* ------------------------------------------
 * io_flush
 * ------------------------------------------ */
void io_flush(io_if_t *io)
{
  int ret;

  if (io == NULL)
  {
    return ;
  }

  ret = io_validate_handle(io);
  if (ret != IO_STATUS_OK)
  {
    return ;
  }

  if ((io->ops->flush) == NULL)
  {
    /* flush 미지원이면 성공으로 간주 */
    return ;
  }

   io->ops->flush(io);
}

 void io_inject(struct io_if *io,uint8_t *data,size_t len)
 {
    int ret;

  if ((io == NULL) || (data == NULL) || (len == 0U))
  {
    return ;
  }

  ret = io_validate_handle(io);
  if (ret != IO_STATUS_OK)
  {
    return ;
  }

  if ((io->ops->recv) == NULL)
  {
    return ;
  }

   io->ops->inject(io, data, len);
 }

/* ------------------------------------------
 * io_ioctl
 * ------------------------------------------ */
int io_ioctl(io_if_t *io,
             uint32_t cmd,
             void *arg)
{
  int ret;

  if (io == NULL)
  {
    return IO_STATUS_INVALID_PARAM;
  }

  ret = io_validate_handle(io);
  if (ret != IO_STATUS_OK)
  {
    return ret;
  }

  if ((io->ops->ioctl) == NULL)
  {
    return IO_STATUS_NOT_SUPPORTED;
  }

  return io->ops->ioctl(io, cmd, arg);
}

/* ------------------------------------------
 * io_is_initialized
 * ------------------------------------------ */
int io_is_initialized(const io_if_t *io)
{
  if (io == NULL)
  {
    return 0;
  }

  return (io->initialized != 0U) ? 1 : 0;
}
