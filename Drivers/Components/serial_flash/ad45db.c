

#include <string.h>


#include "cmsis_os.h"
#include "ad45db.h"

#include "driver_digitalOut.h"
#include "driver_spi.h"
#include "mcu_delay.h"


#define STATUS_REGISTER 	0xD7 //0x57

#define DEVICE_ID  				0x9F

driver_t *at45db_open(void)
{
  static driver_t ad45db;
  static at45db_cfg_t cfg;
  
  ad45db.cfg = &cfg;
  
  return &ad45db;
}


#define BYTE_DUMMY 				0x00 			// Dummy Byte

uint16_t _pageSize = 256;


void AT45_Delay(uint32_t usec)
{
	mcu_delay(usec);
}


void AT45_Write_Buffer(driver_t *drv,uint8_t buffer_choice, uint32_t address, const char * string, uint32_t buf_len)
{
  at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;

	uint8_t szCmd[4];

	driverex_spi_pend_sem(cfg->spi_io);

	driver_do_low(cfg->cs_io);

	if(buffer_choice)
	{
		szCmd[0] = 0x87;
	}
	else
	{
		szCmd[0] = 0x84;
	}

	szCmd[1] = BYTE_DUMMY;
	szCmd[2] = (uint8_t)((address>>8)&0x11);
	szCmd[3] = (uint8_t)address;

	driverex_spi_send_bytes(cfg->spi_io,szCmd,4);

	driverex_spi_send_bytes(cfg->spi_io,(uint8_t *)string,buf_len);

	driver_do_high(cfg->cs_io);

	driverex_spi_post_sem(cfg->spi_io);
	
}

static void AT45_PageWrite(driver_t *drv,uint32_t page)
{
	  at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;

#ifdef	AT45DB321/*PAGE SIZE == 512 */
	FlashSend_Byte((uint8_t)(page >>7));
	FlashSend_Byte((uint8_t)(page <<1));
#else

	driverex_spi_send_byte(cfg->spi_io,(uint8_t)(page >> 8));
	driverex_spi_send_byte(cfg->spi_io,(uint8_t)(page));

#endif
}

void AT45_BufferToMemory(driver_t *drv,uint8_t buffer_choice, uint32_t page)
{
  at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;
	
	
	driverex_spi_pend_sem(cfg->spi_io);

	driver_do_low(cfg->cs_io);

    if(buffer_choice)
    {
			driverex_spi_send_byte(cfg->spi_io,0x86);
    }
    else
    {
			driverex_spi_send_byte(cfg->spi_io,0x83);
    }

    AT45_PageWrite(drv,page);

		driverex_spi_send_byte(cfg->spi_io,BYTE_DUMMY);

	driver_do_high(cfg->cs_io);

	driverex_spi_post_sem(cfg->spi_io);
}


void AT45_RegRead(driver_t *drv,uint8_t cmd, uint8_t *info, uint8_t len)
{
	at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;


	memset(info,  0, len);
	driverex_spi_pend_sem(cfg->spi_io);

	driver_do_low(cfg->cs_io);
	driverex_spi_send_byte(cfg->spi_io,cmd);

	driverex_spi_read_bytes(cfg->spi_io,info,len);


	driver_do_high(cfg->cs_io);

	driverex_spi_post_sem(cfg->spi_io);
}


void AT45_RegWrite(driver_t *drv,uint8_t *cmd)
{
	at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;



	driverex_spi_pend_sem(cfg->spi_io);

	driver_do_low(cfg->cs_io);
	driverex_spi_send_bytes(cfg->spi_io,cmd,4);
;

	driver_do_high(cfg->cs_io);

	driverex_spi_post_sem(cfg->spi_io);
	
}


 void AT45_IsBusy(driver_t *drv)
{
	uint8_t temp[4];
	uint16_t usmSec = 0;

	while(1)
	{
		AT45_RegRead(drv,STATUS_REGISTER, temp, 4);



		if(temp[0] & 0x80) break;

		AT45_Delay(100);
		if(++usmSec > 100) break;
	}
}


/**
  * @brief  : usb인터럽트에서 호출하는 겨우 osDelay()를 사용하면 안된다.
  * @param  :
  * @retval :
  */
// WriteAddr : Page Address (512)
void at45db_write_page(driver_t *drv,uint32_t WriteAddr, uint8_t *writebuff)
{
	uint16_t i;
	uint16_t pageSize=256;
	uint8_t readCnt=2;

	if(_pageSize == 512)
	{
		readCnt = 1;
    pageSize = 512;
	}

	for(i = 0; i <  readCnt; i++)
	{
		AT45_Write_Buffer(drv,0, 0, (const char *) writebuff, pageSize);
		mcu_delay(10);
		AT45_BufferToMemory(drv,0, WriteAddr * readCnt + i);
		AT45_IsBusy(drv);
		writebuff += pageSize;
	}


}

void AT45_MemoryToBuffer(driver_t *drv,uint8_t buffer_choice, uint32_t page)
{
  
  	at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;
    

	driverex_spi_pend_sem(cfg->spi_io);

	driver_do_low(cfg->cs_io);
  
    if(buffer_choice)
    {
			driverex_spi_send_byte(cfg->spi_io,0x55);
    }
    else
    {

			driverex_spi_send_byte(cfg->spi_io,0x53);
    }

  AT45_PageWrite(drv,page);

	driverex_spi_send_byte(cfg->spi_io,BYTE_DUMMY);
	
	driver_do_high(cfg->cs_io);

	driverex_spi_post_sem(cfg->spi_io);
}
void AT45_Read_Buffer(driver_t *drv,uint8_t buffer_choice, uint32_t address, char * string,uint16_t buf_len )
{
	char szCmd[5];
  	at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;


	driverex_spi_pend_sem(cfg->spi_io);

	driver_do_low(cfg->cs_io);

		if(buffer_choice)
		{
			szCmd[0] = 0xD6;
		}
		else
		{
			szCmd[0] = 0xD4;
		}
		szCmd[1] = BYTE_DUMMY;
		szCmd[2] = (uint8_t)((address>>8)&0x11);
		szCmd[3] = (uint8_t)address;
		szCmd[4] = BYTE_DUMMY;


	driverex_spi_send_bytes(cfg->spi_io,(uint8_t *)szCmd,5);
	driverex_spi_read_bytes(cfg->spi_io,(uint8_t *)string,buf_len);
	
	driver_do_high(cfg->cs_io);
	driverex_spi_post_sem(cfg->spi_io);


}
void at45db_read_page(driver_t *drv,uint32_t ReadAddr,uint8_t *readbuff)
{
	uint16_t i;
	uint16_t pageSize=256;
	uint8_t readCnt=2;


	if(_pageSize == 512)
	{
		readCnt = 1;//
        pageSize = 512;
	}
	
	for(i = 0; i < readCnt; i++)
	{
		AT45_MemoryToBuffer(drv,0, (uint32_t) ReadAddr * readCnt + i);
		AT45_IsBusy(drv);
		AT45_Read_Buffer(drv,0, 0, (char*) readbuff,pageSize);
		AT45_IsBusy(drv);
		readbuff += pageSize;
	}
}

const char *_eicpart=NULL;//"AT45DB641E-SHN2B"; /* IC 파트*/
void at45db_init(driver_t *drv)
{
	static const char protect_enable[4] = {
	0x3d, 0x2a, 0x7f, 0xa9
	};

// Configure “Power of 2” (Binary) Page Size
static const char page512table[4] = {
	0x3d, 0x2a, 0x80, 0xa6
};

    uint8_t reg=0;
    uint8_t chip_info[5]={0,0,0,0,0};

    
      // Device ID Information
  // Byte[0] Manufacturer ID  0x1F
  // Byte[1] Device ID        0x28
  // Byte[2] Device ID        0x00
  // Byte[3] Extended Device Information String Leghth 0x01
  // Byte[4] EDI Byte 1                                0x00 
    
    
		AT45_RegRead(drv,DEVICE_ID, chip_info, 5);
					if(chip_info[1]&0x07== 0x7)// density code  bit4 ~bit0  0x03 32Mb, 
	{
		_pageSize = 512;
		_eicpart = "AT45DB321D-SU";
	}
    else
    {
      _eicpart="AT45DB641E-SHN2B";//1F 28 00 01
    }

		osDelay(1);

			AT45_RegRead(drv,STATUS_REGISTER, &reg, 1);//01 0111 10
    
    //reg:BD 10111101

    //안되는 보드 0xDE   11 0111 10
	AT45_Delay(10);
	if((reg & 0x01) == 0)
	{
		AT45_RegWrite(drv,(uint8_t *)protect_enable);
		AT45_IsBusy(drv);
		AT45_RegWrite(drv,(uint8_t *)page512table);//256page 설정
		AT45_IsBusy(drv);
	}
}

