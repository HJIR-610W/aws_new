

#include "driver_fram.h"

#include "fm25cl.h"
#include "driver_spi.h"
#include "driver_digitalOut.h"


typedef struct fram_api_s
{
  void (*read)(driver_t *driver,uint32_t offset,unsigned char *pBuff,uint16_t rLen);
  void (*write)(driver_t *driver,uint32_t offset,unsigned char *pBuff,uint16_t wLen);
}fram_api_t;


driver_t * driver_fram_open(int num)
{
  switch(num)
  {
    case FRAM_FM25LC:
    static driver_t fram_fm25lcl;//driver_fram_open을 하면 생성되는것
    const static fram_api_t fm25_api={.read = fm25cl_read,
                                .write = fm25cl_write};

    driver_t *fm25lc;
    fm25lc_cfg_t *fm25lc_cfg;

    fm25lc = fm25lc_open();// IC를 연다.

    fm25lc_cfg = (fm25lc_cfg_t *)fm25lc->cfg;

    fm25lc_cfg->spi_io = driver_spi_open(STM_SPI_1);//IC 사용해 필요한 하드웨어 연결
    fm25lc_cfg->cs_io  = driver_do_open(DO_FRAM_CS);//IC 사용에 필요한 하드웨여 연결
    
    fram_fm25lcl.api    = &fm25_api;//api 연결
    fram_fm25lcl.handle = fm25lc;   //하위 드라이버 연결(IC연결)

    if(fram_fm25lcl.sem == NULL)
    {
      fram_fm25lcl.sem = osSemaphoreNew(1, 1, NULL); 
    }


    fm25cl_init(fm25lc);

    return &fram_fm25lcl;
    
    break;
  }

  return 0;
}


void driver_fram_read(driver_t* driver, uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
   const fram_api_t* fram_api = ((driver_t *)driver)->api;

  if(driver->sem)
  {
    osSemaphoreAcquire(driver->sem, osWaitForever);
  }
  fram_api->read((((driver_t*)driver)->handle), offset, pBuff, rLen);

  if(driver->sem)
  {
    osSemaphoreRelease(driver->sem);
  }

  
}

void driver_fram_write(driver_t* driver, uint32_t offset, unsigned char* pBuff, uint16_t rLen)
{
   const fram_api_t* fram_api = ((driver_t *)driver)->api;
  if(driver->sem)
  {
    osSemaphoreAcquire(driver->sem, osWaitForever);
  }
  fram_api->write((((driver_t*)driver)->handle), offset, pBuff, rLen);
  
  if(driver->sem)
  {
    osSemaphoreRelease(driver->sem);
  }
}