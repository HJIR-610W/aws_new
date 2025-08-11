

#include "driver_interface.h"

const char *get_drv_err_name(int num)
{
  switch (num)
  {
    case DRV_ERR_NONE:
      return "DRV_ERR_NONE";
    case DRV_ERR_HANDLE:
      return "DRV_ERR_HANDLE";
    case DRV_ERR_TIMEOUT:
      return "DRV_ERR_TIMEOUT";
    case DRV_ERR_RECV_DATA:
      return "DRV_ERR_RECV_DATA";
    case DRV_ERR_NOT_READY:
      return "DRV_ERR_NOT_READY";
    default:
      return "UNKNOWN";
  }
}
