#include "driver_rtc.h"
#include "driver_spi.h"
#include "driver_digitalIn.h"
#include "driver_digitalOut.h"

#include "ds1306.h"







typedef struct rtc_api_s
{
  void (*read)(driver_t *driver,DATE_TIME_BUF *t);
  void (*write)(driver_t *driver,DATE_TIME_BUF *t);
}rtc_api_t;


driver_t * driver_rtc_open(int num)
{
  switch(num)
  {
    case RTC_DS1306:
    static driver_t rtc_ds1306;//driver_fram_open을 하면 생성되는것
    const static rtc_api_t rtc_api={.read = ds1306_read};

    driver_t *drv;
    ds1306_cfg_t *cfg;

    drv = ds1306_open();

    cfg = (ds1306_cfg_t *)drv->cfg;

    cfg->spi_io = driver_spi_open(STM_SPI_1);
    cfg->cs_io  = driver_do_open(DO_RTC_CS);
    cfg->irq_io = driver_di_open(DI_RTC_IRQ);

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
