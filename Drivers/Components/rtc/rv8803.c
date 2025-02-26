#include "pcb_define.h"

#if (FREE_RTOS_USE)
#include "cmsis_os2.h"
#endif

#include "rv8803.h"
#include "driver_stm32_i2c.h"
#include "driver_di.h"

#include "bits_.h"
#include "built_.h"
#include "bcd.h"


#define RET_OK     0
#define RET_EINVAL 1
#define RET_IO_ERR 2

#define RV8803_SEC			0x00
#define RV8803_MIN			0x01
#define RV8803_HOUR			0x02
#define RV8803_WEEK			0x03
#define RV8803_DAY			0x04
#define RV8803_MONTH		0x05
#define RV8803_YEAR			0x06
#define RV8803_RAM			0x07
#define RV8803_ALARM_MIN	0x08
#define RV8803_ALARM_HOUR	0x09
#define RV8803_ALARM_WEEK_OR_DAY 0x0A
#define RV8803_EXT			0x0D
#define RV8803_FLAG			0x0E
#define RV8803_CTRL			0x0F
#define RV8803_OSC_OFFSET	0x2C

#define RV8803_EXT_WADA		BIT(6)

#define RV8803_FLAG_V1F		BIT(0)
#define RV8803_FLAG_V2F		BIT(1)
#define RV8803_FLAG_AF		BIT(3)
#define RV8803_FLAG_TF		BIT(4)
#define RV8803_FLAG_UF		BIT(5)

#define RV8803_CTRL_RESET	BIT(0)

#define RV8803_CTRL_EIE		BIT(2)
#define RV8803_CTRL_AIE		BIT(3)
#define RV8803_CTRL_TIE		BIT(4)
#define RV8803_CTRL_UIE		BIT(5)

#define GENMASK(h, l)		(((uint32_t(1) << ((h)-(l)+1)) - 1) << (l))

#define RX8803_CTRL_CSEL		GENMASK(7, 6)

#define RX8900_BACKUP_CTRL		0x18
#define RX8900_FLAG_SWOFF		BIT(2)
#define RX8900_FLAG_VDETOFF		BIT(3)

#define RTC_VL_DATA_INVALID	  _BITUL(0) /* Voltage too low, RTC data is invalid */
#define RTC_VL_BACKUP_LOW	  _BITUL(1) /* Backup voltage is low */
#define RTC_VL_BACKUP_EMPTY	  _BITUL(2) /* Backup empty or not present */
#define RTC_VL_ACCURACY_LOW	  _BITUL(3) /* Voltage is low, RTC accuracy is reduced */
#define RTC_VL_BACKUP_SWITCH  _BITUL(4) /* Backup switchover happened */


#define BIT_0(bit) (~GENMASK(bit, bit))   // bit 1개만 0으로 
#define BIT_FIELD_0(h,l) ~GENMASK(h, l)   // 범위 bit만 0으로




typedef struct rv8803_cfg_s
{
  uint8_t address;
  void *i2c_io;
  void *irq_io;
  void *sem;
}rv8803_cfg_t;

rv8803_cfg_t rv8803_cfg;
driver_t rv8803_driver;



void rv8803_close(driver_t *handle);
void rv8803_read(driver_t *handle,DATE_TIME_BUF *ct);
void rv8803_set(driver_t *handle, rtc_set_option_t option, void *value);
int32_t rv8803_init(driver_t *rv8803);


static int32_t rv8803_regs_init(driver_t *rv8803)
{
    uint8_t regs[3];
    uint8_t reg=0;
    int32_t err;
    rv8803_cfg_t *cfg = (rv8803_cfg_t *)rv8803->cfg;
    //offset 값  aging correction,0은 초기값
    
    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_OSC_OFFSET,&reg,1);

	if (err)
    {
		return RET_IO_ERR;
    }

    reg = 0x00;//초기화값
    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_CTRL,&reg,1);
    
	if(err)
    {
		return RET_IO_ERR;
    }

    regs[0] = 0;
    regs[1] = 0;
    regs[2] = 0;

    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_ALARM_MIN,regs,3);

	if (err)
    {
		return RET_IO_ERR;
    }

    reg = 0;
    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_RAM,&reg,1);

	return err;
}


static int rv8803_regs_reset(driver_t *rv8803)
{
    int32_t err;
    
    err = rv8803_regs_init(rv8803);
	if(err)
    {
	    return err;
    }

	//err =  rv8803_regs_configure(rv8803);

    return err;
}


/**
 * @param en 0 disable, 1 enable
 * 1초 주기 인터럽트
*/
int32_t rv8803_set_periodTimeUpdateIrq(driver_t *rv8803,uint8_t en)
{
    uint8_t reg;
    int32_t err;
    rv8803_cfg_t *cfg = (rv8803_cfg_t *)rv8803->cfg;

    err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_EXT, &reg,1);
    //초기에 읽으면 0x40
	if(err)
    {
		return RET_IO_ERR;
    }


    reg = reg & ~(1<<5);//USEL = 0, 1sec 
    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_EXT,&reg,1);

    if(err)
    {
        return RET_IO_ERR;
    }
	

    err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_FLAG, &reg,1);
    //초기에 읽으면 0x04
	if(err)
    {
		return RET_IO_ERR;
    }


    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_FLAG,&reg,1);

    if(err)
    {
        return RET_IO_ERR;
    }

    err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_CTRL, &reg,1);

	if(err)
    {
		return RET_IO_ERR;
    }


    if(en)
    {
        reg = reg | (1<<5);
    }
    else
    {
        reg = reg | ~(1<<5);
    }

    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_CTRL,&reg,1);

    if(err)
    {
        return RET_IO_ERR;
    }

    return err;

}





rtc_api_t rv8803_api={.read = rv8803_read,.close = rv8803_close,.set = rv8803_set};



driver_t *rv8803_open(void)
{
  if(rv8803_driver.opened)
  {
    return &rv8803_driver;
  }

  rv8803_driver.opened = true;
  rv8803_cfg.i2c_io = driver_stm32_i2c_open(STM32_I2C_1,0);
  rv8803_cfg.irq_io = driver_di_open(DI_1_RTC_IRQ,0);
  rv8803_cfg.address = 0x32;
  rv8803_driver.cfg = &rv8803_cfg;
  rv8803_driver.api = &rv8803_api;

  if(rv8803_driver.sem==NULL)
  {
#if FREE_RTOS_USE
    rv8803_driver.sem = osSemaphoreNew(1, 1, NULL); 
#endif
  }

  rv8803_init(&rv8803_driver);

  return &rv8803_driver;
}



void rv8803_close(driver_t *handle)
{

}
void rv8803_read(driver_t *rv8803,DATE_TIME_BUF *ct)
{
	uint8_t date1[7];
	uint8_t date2[7];
    uint8_t reg;
    uint8_t *date = date1;
    int32_t err;
    rv8803_cfg_t *cfg = (rv8803_cfg_t *)rv8803->cfg;

  err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_FLAG, &reg,1);

	if(err)
    {
		//return RET_IO_ERR;
    }
    
    //Vdd와 가 낮다. POR 시에는 1로 됨, 데이터가 유요하지 않음0x27
	if(reg & RV8803_FLAG_V2F)
    {
       // return RET_EINVAL;
	}

  err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_SEC, date,7);

	if(err)
    {
	//	return RET_IO_ERR;
    }

    // 주의:초를 읽었더니 59초 이면 한번더 읽는다.
    // 읽고자 하는 값이 01:59임, 그런데 읽는 도중에도 시간은 변함, 59초를 읽는데 도중에 초가 증가하면
    // 02:00 이 되며 분을 읽게 되면 2을 읽게됨 원하는 값은 01:59 지만 실제는 02:59가 되어버림
    // 단,7바이트 읽기가 1초안에 수행 되어야함
	if((date1[RV8803_SEC] & 0x7f) == bin2bcd(59)) 
    {
      err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_SEC, date2,7);
		if (err)
        {
			//return RET_IO_ERR;
        }
        //읽었더니 초가 59가 아니면 이 값이 유요한값, 59와 같다면 이전에 읽은값이 유요한값
		if ((date[RV8803_SEC] & 0x7f) != bin2bcd(59))
        {
			date = date2;
        }
	}

	ct->Sec    = bcd2bin(date[RV8803_SEC] & 0x7f);
	ct->Min    = bcd2bin(date[RV8803_MIN] & 0x7f);
	ct->Hour   = bcd2bin(date[RV8803_HOUR] & 0x3f);
	ct->Week   = (uint8_t)(31- clz(date[RV8803_WEEK] & 0x7f));
	ct->Day    = bcd2bin(date[RV8803_DAY] & 0x3f);
	ct->Month  = bcd2bin(date[RV8803_MONTH] & 0x1f) ;
	ct->Year   = bcd2bin(date[RV8803_YEAR]) + 2000;

	//return RET_OK;
}



int32_t rv8803_set_clock(driver_t *rv8803, uint8_t hour, uint8_t min, uint8_t sec)
{
uint8_t date[7];
	int32_t ret;
    uint8_t reg;
    int32_t err;
    rv8803_cfg_t *cfg = (rv8803_cfg_t *)rv8803->cfg;
    err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_CTRL, &reg,1);

    if(err)
    {
        return RET_IO_ERR;
    }

    reg = reg | RV8803_CTRL_RESET;

    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_CTRL,&reg,1);

	if(err)
	{
		return RET_IO_ERR;
	}

	date[0]   = bin2bcd(sec);
	date[1]   = bin2bcd(min);
	date[2]  =  bin2bcd(hour);


  err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_SEC,date,3);

	if(err)
	{
		return RET_IO_ERR;
	}


    reg = reg & ~RV8803_CTRL_RESET;
    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_CTRL,&reg,1);

	if(err)
	{
		return RET_IO_ERR;
	}

	//mutex_lock(&rv8803->flags_lock);

  err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_FLAG, &reg,1);

    if(err)
    {
        return RET_IO_ERR;
    }


	if (reg & RV8803_FLAG_V2F)//Voltage Low Flag2
    {
		ret = rv8803_regs_reset(rv8803);
		if (ret)
        {
			return ret;
		}
	}

    reg = reg & ~(RV8803_FLAG_V1F | RV8803_FLAG_V2F);

    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_FLAG,&reg,1);

	return err;
}








int32_t rv8803_set_date(driver_t *rv8803,uint16_t year, int8_t mon, uint8_t day)
{
	uint8_t date[7];
	int32_t ret;
    uint8_t reg;
    int32_t err;
    rv8803_cfg_t *cfg = (rv8803_cfg_t *)rv8803->cfg;

    err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_CTRL, &reg,1);

    if(err)
    {
        return RET_IO_ERR;
    }

    reg = reg | RV8803_CTRL_RESET;

    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_CTRL,&reg,1);

	if(err)
	{
		return RET_IO_ERR;
	}

	date[0]  = bin2bcd(day);
	date[1]  = bin2bcd(mon);
	date[2]  = bin2bcd(year % 100);


  err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_DAY,date,3);

	if(err)
	{
		return RET_IO_ERR;
	}


    reg = reg & ~RV8803_CTRL_RESET;
    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_CTRL,&reg,1);

	if(err)
	{
		return RET_IO_ERR;
	}

	//mutex_lock(&rv8803->flags_lock);

  err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_FLAG, &reg,1);

    if(err)
    {
        return RET_IO_ERR;
    }


	if (reg & RV8803_FLAG_V2F)//Voltage Low Flag2
    {
		ret = rv8803_regs_reset(rv8803);
		if (ret)
        {
			return ret;
		}
	}

    reg = reg & ~(RV8803_FLAG_V1F | RV8803_FLAG_V2F);

    err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_FLAG,&reg,1);

	return err;
}
void rv8803_set_time(driver_t *driver,DATE_TIME_BUF *ct)
{
  rv8803_set_date(driver,ct->Year,ct->Month,ct->Day);
  rv8803_set_clock(driver,ct->Hour,ct->Min,ct->Sec);

}
void rv8803_set(driver_t *driver, rtc_set_option_t option, void *value)
{
  DATE_TIME_BUF *ct;
#if FREE_RTOS_USE
  if(driver->sem)
  {
   osSemaphoreAcquire(driver->sem, osWaitForever);
  }
#endif
  switch (option)
  {
  case eRTC_SET_TIME:
    ct = value;
    rv8803_set_time(driver,ct);
    break;
  case eRTC_SET_IRQ:

    break;
  }

#if FREE_RTOS_USE
  if(driver->sem)
  {
   osSemaphoreRelease(driver->sem);
  }
#endif
}

int32_t rv8803_init(driver_t *rv8803)
{
  
  uint8_t reg;
  int32_t err;
  int32_t ret;
  DATE_TIME_BUF nt={.Year=2000,.Month = 1,.Day =1,.Hour = 0,.Min = 0,.Sec =0};
  rv8803_cfg_t *cfg = (rv8803_cfg_t *)rv8803->cfg;

  err = stm32_i2c_read(cfg->i2c_io,cfg->address,RV8803_FLAG,&reg,1);


 
  if(err)
  {
      return RET_IO_ERR;
  }


if (reg & RV8803_FLAG_V2F)//Voltage Low Flag2
  {
  ret = rv8803_regs_reset(rv8803);
  if (ret)
      {
    return ret;
  }
      reg = reg & ~(RV8803_FLAG_V1F | RV8803_FLAG_V2F);

      err =  stm32_i2c_send(cfg->i2c_io, cfg->address,RV8803_FLAG,&reg,1);

      rv8803_set_time(rv8803,&nt);

}
return 0;
}