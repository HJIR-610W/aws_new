


#include "fm25cl.h"
#include "driver_do.h"
#include "driver_spi.h"

#define FRAM_LOCK_USE 1 /* Mutex 사용할지 선택 */

#define FRAM_1024     0 /* FRAM 용량  1이면 1024용량 사용*/

#define WRSR 	0x01
#define WRITE 0x02
#define READ 	0x03
#define WRDI 	0x04
#define RDSR 	0x05
#define WREN 	0x06



driver_t g_fm25cl;
fm25lc_cfg_t g_fm25lc_cfg;

driver_t *fm25lc_open(void)
{
  g_fm25cl.cfg = &g_fm25lc_cfg;
  return &g_fm25cl;
}


static void fram_cmd(driver_t *fm25cl,uint8_t cmd)
{
   fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;

    driver_do_low(cfg->cs_io);    

    driverex_spi_send_byte(cfg->spi_io,cmd);
   driver_do_high(cfg->cs_io);
}



void fm25cl_write(driver_t *fm25cl,uint32_t offset,uint8_t *pData,uint16_t wLen)
{
  fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;

    uint32_t i;


 
#if !FRAM_1024
   fram_cmd(fm25cl,WREN);
#endif

    driver_do_low(cfg->cs_io);

    driverex_spi_send_byte(cfg->spi_io,WRITE);
#if FRAM_1024
    FRAM_SPI_WRITE_BYTE((addr>>16)&0xFF);
#endif
    driverex_spi_send_byte(cfg->spi_io,(offset>>8)&0xFF);
    driverex_spi_send_byte(cfg->spi_io,offset&0xFF);

    osDelay(1);
    driverex_spi_send_bytes(cfg->spi_io,pData,wLen);
    driver_do_high(cfg->cs_io);
   
    



}

void fm25cl_read(driver_t *fm25cl,uint32_t offset,uint8_t *pBuff,uint16_t rLen)
{
  fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;
 uint8_t data=0;
    


    driver_do_low(cfg->cs_io);

    uint32_t i;



        driverex_spi_send_byte(cfg->spi_io,READ);
#if FRAM_1024
        driverex_spi_send_byte(cfg->spi_io,(offset>>16)&0xFF);
#endif    
        driverex_spi_send_byte(cfg->spi_io,(offset>>8)&0xFF);
        driverex_spi_send_byte(cfg->spi_io,offset&0xFF);

    for(i=0;i<rLen;i++)
    {
        pBuff[i] = driverex_spi_read_byte(cfg->spi_io);

    }
    
    driver_do_high(cfg->cs_io);
  

}

uint8_t fm25cl_read_status(driver_t *fm25cl)
{
  fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;
  uint8_t data=0;
    

    driver_do_low(cfg->cs_io);
    driverex_spi_send_byte(cfg->spi_io,RDSR);
    data = driverex_spi_read_byte(cfg->spi_io);
    driver_do_high(cfg->cs_io);

    return data;
    
}

uint8_t data;
void fm25cl_init(driver_t *fm25cl)
{
  fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;

  data = fm25cl_read_status(fm25cl);
}