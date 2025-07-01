
#include "cmsis_os2.h"
#include "ds1306.h"

#include "driver_do.h"
#include "driver_spi.h"
#include "driver_spi.h"
#include "driver_di.h"
#include "driver_do.h"

#define DS1306_SECONDS  0x00
#define DS1306_MINUTES  0x01
#define DS1306_HOURS    0x02
#define DS1306_DATE     0x03
#define DS1306_DAY      0x04
#define DS1306_MONTH    0x05
#define DS1306_YEAR     0x06


typedef struct ds1306_cfg_s
{
  void *spi_io;
  void *cs_io;
  void *irq_io;
  void *sem;
}ds1306_cfg_t;

ds1306_cfg_t ds1306_cfg;
driver_t ds1306_driver;


void close(driver_t *handle);
int32_t ds1306_read(driver_t *handle,DATE_TIME_BUF *ct);
void ds1306_set(driver_t *handle, rtc_set_option_t option, void *value);


rtc_api_t rtc_api={.read = ds1306_read,.close = close,.set = ds1306_set};







int32_t ds1306_read_reg(driver_t *ds1306, uint8_t reg,uint8_t *rval)
{
  int32_t err=0;
  ds1306_cfg_t *cfg=(ds1306_cfg_t*)ds1306->cfg;

  driver_spi_pend_sem(cfg->spi_io);

  driver_do_high(cfg->cs_io);


  driver_spi_send_byte(cfg->spi_io,reg);
    
  *rval = driver_spi_read_byte(cfg->spi_io);

  driver_do_low(cfg->cs_io);

  driver_spi_post_sem(cfg->spi_io);
  
  return err;
}


int32_t ds1306_write_reg(driver_t *ds1306,uint8_t reg,uint8_t val)
{
    int32_t err=0;
  ds1306_cfg_t *cfg=(ds1306_cfg_t*)ds1306->cfg;

  driver_spi_pend_sem(cfg->spi_io);

  driver_do_high(cfg->cs_io);
    
  reg = reg + 0x80;
  driver_spi_send_byte(cfg->spi_io,reg);
  driver_spi_send_byte(cfg->spi_io,val);

  driver_do_low(cfg->cs_io);
 
   driver_spi_post_sem(cfg->spi_io);
  return err;

}

#define DS1306_WRITE 0x80 // 쓰기 명령어 (명령어의 최상위 비트를 1로 설정)
#define DS1306_CONTROL_REG 0x0F // 제어 레지스터 주
#define DS1306_READ 0x00           // 읽기 명령어 (명령어의 최상위 비트를 0으로 설정)
#define DS1306_SECONDS_REG 0x00    // 초 레지스터 주소
#define DS1306_MINUTES_REG 0x01    // 분 레지스터 주소
#define DS1306_HOURS_REG 0x02      // 시 레지스터 주소

// BCD 데이터를 이진수로 변환하는 함수
uint8_t BCD_to_Decimal(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
uint8_t DecToBCD(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

// DS1306에서 현재 시간 읽기 함수
void ds1306_read_time(driver_t *ds1306, DATE_TIME_BUF *t) {
    uint8_t time_data[7] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // 초, 분, 시, 일, 월, 요일, 년 데이터를 저장할 배열
    uint8_t reg_address = DS1306_READ | DS1306_SECONDS_REG;            // 시작 레지스터 주소 (초)
    ds1306_cfg_t *cfg = (ds1306_cfg_t *)ds1306->cfg;

    // SPI 동기화
    driver_spi_pend_sem(cfg->spi_io);
    driver_do_high(cfg->cs_io);

    // 시작 레지스터 주소 전송 (읽기 모드)
    driver_spi_send_byte(cfg->spi_io, reg_address);

    // 초, 분, 시, 일, 월, 요일, 년 데이터를 수신
    driver_spi_read_bytes(cfg->spi_io, time_data, 7);

    // SPI 통신 종료
    driver_do_low(cfg->cs_io);
    driver_spi_post_sem(cfg->spi_io);

    // BCD 데이터를 이진수로 변환
    t->Sec  = BCD_to_Decimal(time_data[DS1306_SECONDS]); // 초
    t->Min  = BCD_to_Decimal(time_data[DS1306_MINUTES]); // 분
    t->Hour = BCD_to_Decimal(time_data[DS1306_HOURS]); // 시

    //t->Day  = BCD_to_Decimal(time_data[3]); // 요일 (Day of Week)

    t->Day  = BCD_to_Decimal(time_data[DS1306_DAY]); // 일
    t->Month= BCD_to_Decimal(time_data[DS1306_MONTH]); // 월

    t->Year = BCD_to_Decimal(time_data[DS1306_YEAR])+2000; // 년
}

void ds1306_set_time(driver_t *driver,DATE_TIME_BUF *ct)
{
  ds1306_write_reg(driver,DS1306_YEAR,DecToBCD(ct->Year%100));
  ds1306_write_reg(driver,DS1306_MONTH,DecToBCD(ct->Month));
  ds1306_write_reg(driver,DS1306_DAY,DecToBCD(ct->Day));
  ds1306_write_reg(driver,DS1306_HOURS,DecToBCD(ct->Hour));
  ds1306_write_reg(driver,DS1306_MINUTES  ,DecToBCD(ct->Min));
  ds1306_write_reg(driver,DS1306_SECONDS  ,DecToBCD(ct->Sec));
}


void ds1306_init(driver_t *ds1306)
{
  uint8_t val;

  ds1306_write_reg(ds1306,0x0f,00);  //WP,1Hz,AIE1,AIE0 
  ds1306_write_reg(ds1306,0x11,00);//TRICKLE CHARGE REGISTER
  ds1306_read_reg(ds1306,0x10,&val);//상태 레지스터터
  ds1306_read_reg(ds1306,0x07,&val);
   
}



driver_t *ds1306_open(void)
{
  if(ds1306_driver.opened)
  {
    return &ds1306_driver;
  }

  ds1306_driver.opened = true;
  ds1306_cfg.spi_io = driver_spi_open(STM_SPI_1);
  ds1306_cfg.cs_io  = driver_do_open(DO_RTC_CS,0);
  ds1306_cfg.irq_io = driver_di_open(DI_1_RTC_IRQ,0);
  
  ds1306_driver.cfg = &ds1306_cfg;
  ds1306_driver.api = &rtc_api;

  if(ds1306_driver.sem==NULL)
  {
    ds1306_driver.sem = osSemaphoreNew(1, 1, NULL); 
  }

  ds1306_init(&ds1306_driver);

  return &ds1306_driver;
}



void close(driver_t *handle)
{

}

int32_t ds1306_read(driver_t *driver,DATE_TIME_BUF *ct)
{
  if(driver->sem)
  {
   osSemaphoreAcquire(driver->sem, osWaitForever);
  }
  ds1306_read_time(driver,ct);
  
  if(driver->sem)
  {
   osSemaphoreRelease(driver->sem);
  }
  
  return 0;
}

void ds1306_set(driver_t *driver, rtc_set_option_t option, void *value)
{
  DATE_TIME_BUF *ct;

  if(driver->sem)
  {
   osSemaphoreAcquire(driver->sem, osWaitForever);
  }
  switch (option)
  {
  case eRTC_SET_TIME:
    ct = value;
    ds1306_set_time(driver,ct);
    break;
  case eRTC_SET_IRQ:

    break;
  }

  if(driver->sem)
  {
   osSemaphoreRelease(driver->sem);
  }
}
