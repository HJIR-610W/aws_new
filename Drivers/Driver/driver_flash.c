
#include <string.h>
#include "driver_flash.h"
#include "driver_spi.h"
#include "driver_digitalOut.h"
#include "ad45db.h"

typedef struct flash_api_s
{
  void (*read)(driver_t *driver,uint32_t offset,unsigned char *pBuff,uint16_t rLen);
  void (*write)(driver_t *driver,uint32_t offset,unsigned char *pBuff,uint16_t wLen);
  void (*read_page)(driver_t *driver,uint32_t page_num,unsigned char *pBuff);
  void (*write_page)(driver_t *driver,uint32_t page_num,unsigned char *pBuff);
}flash_api_t;
   



void driver_flash_pend_sem(driver_t *spi)
{
  if(spi->sem)
  {
    osSemaphoreAcquire(spi->sem, osWaitForever);
  }
}

void _spi_post_sem(driver_t *spi)
{
  if(spi->sem)
  {
    osSemaphoreRelease(spi->sem);
  }
}





driver_t * driver_flash_open(int num)
{
  driver_t at45db;

  switch(num)
  {
    case FALSH_AT45DB:
    static driver_t flash_at45db;//driver_fram_open을 하면 생성되는것
    const static flash_api_t at45db_api={.read_page  = at45db_read_page,
                                         .write_page = at45db_write_page};

    driver_t *at45db_ic;
    at45db_cfg_t *p_at45db_cfg;

    at45db_ic = at45db_open();// IC를 연다.

    p_at45db_cfg = (at45db_cfg_t *)at45db_ic->cfg;

    p_at45db_cfg->spi_io = driver_spi_open(STM_SPI_1);//IC 사용해 필요한 하드웨어 연결
    p_at45db_cfg->cs_io  = driver_do_open(DO_FLASH_CS);//IC 사용에 필요한 하드웨여 연결
        
    flash_at45db.api    = &at45db_api;//api 연결
    flash_at45db.handle = at45db_ic;   //하위 드라이버 연결(IC연결)

    if(flash_at45db.sem == NULL)
    {
      flash_at45db.sem = osSemaphoreNew(1, 1, NULL); 
    }

    at45db_init(at45db_ic);
    return &flash_at45db;
  break;
  }

}




/**
 * @brief 플래시를 ram처럼  연속 접근 접근
*/
void driver_flash_read(driver_t *drv, uint32_t offset, uint8_t* pBuff,uint32_t buffSize, uint32_t readLen)
{
  uint8_t buff[512];
  uint16_t remain;
  uint32_t pageQuot;
  uint32_t pageRem;
  uint32_t dataQuot;
  uint32_t dataRem;
  uint32_t i;
  flash_api_t* api = (flash_api_t *)drv->api;


  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
    

    pageQuot = offset / 512;
    pageRem  = offset % 512;

    remain = 512 - pageRem; // 남은 영역

    api->read_page(drv->handle, pageQuot, buff);


    if(readLen > remain)
    {
        memcpy(&pBuff[0], &buff[pageRem], remain);

        dataQuot = (readLen - remain) / 512;
        dataRem = (readLen - remain) % 512;

        for (i = 0; i < dataQuot; i++)
        {
            api->read_page(drv->handle, pageQuot + i + 1, buff);
            memcpy(&pBuff[remain +i * 512],buff, 512);
        }

        if (dataRem)
        {
            api->read_page(drv->handle, pageQuot + i + 1, buff);
            memcpy(&pBuff[remain +i * 512], &buff[0], dataRem);
        }
    }
    else
    {
        memcpy(&pBuff[0] , &buff[pageRem], readLen);
    }

    
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
}





#define FALSH_PAGE_SIZE 512
int32_t driver_flash_write(driver_t *drv, uint32_t offset, uint8_t* pData, uint32_t dataLen)
{
    uint8_t buff[FALSH_PAGE_SIZE];
    uint16_t remain;
    uint32_t pageQuot;
    uint32_t pageRem;
    uint32_t dataQuot;
    uint32_t dataRem;
    uint32_t i;
  flash_api_t* api = (flash_api_t *)drv->api;

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
    


    pageQuot = offset / FALSH_PAGE_SIZE;
    pageRem  = offset % FALSH_PAGE_SIZE;

    remain = FALSH_PAGE_SIZE - pageRem; // 남은 영역

    //TODO: 무조건 처음부터 읽을 필요 없음, 512를 쓰는경우 무조건 쓰기해야함 
    if(pageRem)//remain이 512가 아니면 offset은 512배수가 아니기에 read write 동작 필요
    {
      api->read_page(drv->handle, pageQuot, buff);
    }
    else if(dataLen<FALSH_PAGE_SIZE)
    {
      api->read_page(drv->handle, pageQuot, buff);
    }

    if (dataLen > remain)
    {
        memcpy(&buff[pageRem], &pData[0], remain);
        api->write_page(drv->handle, pageQuot, buff);

        dataQuot = (dataLen - remain) / FALSH_PAGE_SIZE;
        dataRem  = (dataLen - remain) % FALSH_PAGE_SIZE;

        for ( i = 0; i < dataQuot; i++)
        {
            memcpy(buff, &pData[remain+i*FALSH_PAGE_SIZE], FALSH_PAGE_SIZE);
            api->write_page(drv->handle, pageQuot + i +1, buff);
        }

        if(dataRem)
        {
            api->read_page(drv->handle, pageQuot + i + 1, buff);
            memcpy(&buff[0], &pData[remain+i*FALSH_PAGE_SIZE],dataRem);
           api->write_page(drv->handle, pageQuot + i + 1, buff);
        }
    }
    else
    {
        memcpy(&buff[pageRem], &pData[0], dataLen);
        api->read_page(drv->handle, pageQuot, buff);
    }

  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
    
    return 0;
}