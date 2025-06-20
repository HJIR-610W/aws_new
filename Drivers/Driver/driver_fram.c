

#include "driver_fram.h"

#include "driver_fram_define.h"
#include "fm25cl.h"

driver_t * driver_fram_open(int num)
{
  driver_t *driver=NULL;
  
  switch(num)
  {
    case FRAM_FM25LC:
      driver = fm25lc_open();  // IC¸¦ ¿¬´Ù.
      break;
  }

  return driver;
}


void driver_fram_read(driver_t* driver, uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
  fram_api_t* api = (fram_api_t *)driver->api;

  api->read(driver, offset, pBuff, rLen);
 
}

void driver_fram_write(driver_t* driver, uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
  fram_api_t *api = (fram_api_t *)driver->api;

  api->write(driver, offset, pBuff, rLen);

}
