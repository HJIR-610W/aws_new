

#include "ds1306.h"

#include "driver_digitalOut.h"
#include "driver_spi.h"


driver_t *ds1306_open(void)
{
  static driver_t ds1306;
  static ds1306_cfg_t cfg;
  
  ds1306.cfg = &cfg;
  
  return &ds1306;
}





int32_t ds1306_read_reg(driver_t *ds1306, uint8_t reg,uint8_t *rval)
{
  int32_t err;
  ds1306_cfg_t *cfg=(ds1306_cfg_t*)ds1306->cfg;



  driverex_spi_pend_sem(cfg->spi_io);

  driver_do_high(cfg->cs_io);


  driverex_spi_send_byte(cfg->spi_io,reg);
    
  *rval = driver_spi_read_byte(cfg->spi_io);

  driver_do_low(cfg->cs_io);

  driverex_spi_post_sem(cfg->spi_io);
  
  return err;
}


int32_t ds1306_write_reg(driver_t *ds1306,uint8_t reg,uint8_t val)
{
    int32_t err;
  ds1306_cfg_t *cfg=(ds1306_cfg_t*)ds1306->cfg;

  driverex_spi_pend_sem(cfg->spi_io);

  driver_do_high(cfg->cs_io);
    
  reg = reg + 0x80;
  driverex_spi_send_byte(cfg->spi_io,reg);
  driverex_spi_send_byte(cfg->spi_io,val);

  driver_do_low(cfg->cs_io);
 
   driverex_spi_post_sem(cfg->spi_io);
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


// DS1306에서 현재 시간 읽기 함수
void ds1306_read(driver_t *ds1306,DATE_TIME_BUF *t)
{
  uint8_t time_data[3]={0xff,0xff,0xff};  // 초, 분, 시 데이터를 저장할 배열
    uint8_t reg_address = DS1306_READ | DS1306_SECONDS_REG; // 시작 레지스터 주소 (초)
  ds1306_cfg_t *cfg=(ds1306_cfg_t*)ds1306->cfg;


  driverex_spi_pend_sem(cfg->spi_io);

    driver_do_high(cfg->cs_io);

    // 시작 레지스터 주소 전송 (읽기 모드)
    driverex_spi_send_byte(cfg->spi_io,reg_address);
    // 초, 분, 시 데이터를 수신

    driverex_spi_read_bytes(cfg->spi_io,time_data,3);
    
   driver_do_low(cfg->cs_io);
  driverex_spi_post_sem(cfg->spi_io);
    // BCD 데이터를 이진수로 변환
    t->Sec = BCD_to_Decimal(time_data[0]);
    t->Min = BCD_to_Decimal(time_data[1]);
    t->Hour = BCD_to_Decimal(time_data[2]);
}


uint8_t min;
uint8_t hour;
uint8_t sec;

void ds1306_init(driver_t *ds1306)
{
  ds1306_cfg_t *cfg=(ds1306_cfg_t*)ds1306->cfg;
    uint8_t val;



    ds1306_write_reg(ds1306,0x0f,00);   
    
    ds1306_write_reg(ds1306,0x11,00);

 	ds1306_read_reg(ds1306,0x10,&val);
	ds1306_read_reg(ds1306,0x07,&val);
   
}
