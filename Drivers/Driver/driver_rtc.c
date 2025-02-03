#include "driver_rtc.h"
#include "driver_spi.h"
#include "driver_di.h"
#include "driver_do.h"

#include "ds1306.h"


typedef struct rtc_api_s
{
  void (*read)(driver_t *driver,DATE_TIME_BUF *t);
  void (*write)(driver_t *driver,DATE_TIME_BUF *t);
}rtc_api_t;


driver_t * driver_rtc_open(int num)
{
    static driver_t rtc_ds1306;//driver_fram_open을 하면 생성되는것
    const static rtc_api_t rtc_api={.read = ds1306_read};
    driver_t *drv;
    ds1306_cfg_t *cfg;
    switch(num)
  {
    case RTC_DS1306:




    drv = ds1306_open();

    cfg = (ds1306_cfg_t *)drv->cfg;

    cfg->spi_io = driver_spi_open(STM_SPI_1);
    cfg->cs_io  = driver_do_open(DO_RTC_CS,0);
    cfg->irq_io = driver_di_open(DI_1_RTC_IRQ,0);

    rtc_ds1306.api    = &rtc_api;
    rtc_ds1306.handle = drv;   

    if(rtc_ds1306.sem == NULL)
    {
      rtc_ds1306.sem = osSemaphoreNew(1, 1, NULL); 
    }

    ds1306_init(drv);
  
    return &rtc_ds1306;
    
    break;
  }

  return 0;
}

void driver_rtc_read(driver_t* driver, DATE_TIME_BUF *t)
{
   const rtc_api_t* fram_api = ((driver_t *)driver)->api;

  if(driver->sem)
  {
    osSemaphoreAcquire(driver->sem, osWaitForever);//불필요 검토
  }
  fram_api->read((((driver_t*)driver)->handle), t);

  if(driver->sem)
  {
    osSemaphoreRelease(driver->sem);
  }

}




void driver_rtc_set(driver_t *driver,uint8_t cmd,void *opt)
{
  driver_t* drv = ((driver_t *)driver)->handle;
  ds1306_cfg_t *cfg = drv->cfg;
    driver_t *rain_pulse;
        di_isr_set_cfg_t *isr_cfg = &((rtc_set_irq_cfg_t *)opt)->cfg;
  switch (cmd)
  {
    case eRTC_SET_IRQ:



    driver_di_set(cfg->irq_io,DI_SET_INTERRUPT,isr_cfg);
    break;
  
  default:
    break;
  }
}
