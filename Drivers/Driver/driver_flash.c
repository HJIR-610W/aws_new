
#include <string.h>
#include "driver_flash.h"
#include "driver_spi.h"
#include "driver_do.h"
#include "at45db.h"

#include "driver_flash_define.h"

#include "system_err.h"

driver_t * driver_flash_open(int num)
{
  driver_t *driver = NULL;
  
  switch(num)
  {
    case FALSH_AT45DB:
      driver = at45db_open(num);  // IC를 연다.
      break;
  }

  return driver;
}

void driver_flash_read(driver_t *drv, uint32_t offset, uint8_t *pBuff, uint32_t buffSize,
                       uint32_t readLen)
{
  flash_api_t *api = (flash_api_t *)drv->api;

  if (drv == NULL || api == NULL)
  {
    ERROR_PRINTF("flash drv==NULL");
  }

  api->read(drv,  offset, pBuff,  buffSize,  readLen);
}

int32_t driver_flash_write(driver_t *drv, uint32_t offset, uint8_t *pData, uint32_t dataLen)
{
  flash_api_t *api = (flash_api_t *)drv->api;

  if (drv == NULL|| api == NULL)
  {
    ERROR_PRINTF("flash drv==NULL");
  }

  api->write(drv, offset, pData, dataLen);

  return 0;

}
