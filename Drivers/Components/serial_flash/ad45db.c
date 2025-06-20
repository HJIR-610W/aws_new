

#include "ad45db.h"

#include <string.h>

#include "cmsis_os.h"
#include "driver_do.h"
#include "driver_flash_define.h"
#include "driver_spi.h"
#include "os_user_def.h"
#include "system_err.h"
#include "usDelay.h"


#define DEVICE_ID 0x9F
#define STATUS_REGISTER 0xD7  // 0x57


typedef struct ad45db_cfg_s
{
  driver_t *spi_io;
  driver_t *cs_io;

} at45db_cfg_t;


int32_t at45db_write(driver_t *drv, uint32_t offset, uint8_t *pData, uint32_t dataLen);
void at45db_read(driver_t *drv, uint32_t offset, uint8_t *pBuff, uint32_t buffSize,
                 uint32_t readLen) ;
void at45db_read_page(driver_t *drv, uint32_t ReadAddr, uint8_t *readbuff);
void at45db_write_page(driver_t *drv, uint32_t WriteAddr, uint8_t *writebuff);
void at45db_init(driver_t *drv);


    const static flash_api_t at45db_api = {
      .read_page = at45db_read_page,
      .write_page = at45db_write_page,
      .write = at45db_write,
      .read = at45db_read
    };

static driver_t ad45db;
static at45db_cfg_t cfg;

driver_t *at45db_open(int32_t num)
{
  if (ad45db.opened)
  {
    return &ad45db;
  }

  ad45db.opened = true;
  ad45db.api = &at45db_api;
  ad45db.cfg = &cfg;
  ad45db.instance_id = num;
  ad45db.driver_type = eDRIVER_FLASH;
  ad45db.name = "at45db";

  cfg.spi_io = driver_spi_open(STM_SPI_1);                // IC 사용해 필요한 하드웨어 연결
  cfg.cs_io  = driver_do_open(DO_FLASH_CS, 0);              // IC 사용에 필요한 하드웨여 연결

  OS_CREATE_BINARY_SEM(ad45db.sem);

  at45db_init(&ad45db);
  return &ad45db;
}


#define BYTE_DUMMY 				0x00 			// Dummy Byte

static uint16_t s_page_size = 256;


void AT45_Delay(uint32_t usec)
{
  usDelay(usec);
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

  if(s_page_size == 512)
  {
    readCnt = 1;
    pageSize = 512;
  }

  for(i = 0; i <  readCnt; i++)
  {
    AT45_Write_Buffer(drv,0, 0, (const char *) writebuff, pageSize);
    usDelay(10);
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


  if(s_page_size == 512)
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

#include <stdint.h>

#define printf debug_printf

void debug_printf(const char *fmt, ...);  // 사용자 정의 출력 함수

#include <stdint.h>

#define printf debug_printf

void debug_printf(const char *fmt, ...);  // 사용자 정의 출력 함수

#include <stdint.h>

#define printf debug_printf

void debug_printf(const char *fmt, ...);  // 사용자 정의 디버그 출력 함수

void at45db_parse_chip_info(uint8_t *chip_info)
{
  const char *manufacturer = "알 수 없음";
  const char *family = "알 수 없음";
  const char *capacity_str = "알 수 없음";
  const char *subcode = "표준 (00h)";
  const char *variant = "00000";
  const char *revision_str = "Unknown";

  uint8_t id0 = chip_info[0];  // Manufacturer ID
  uint8_t id1 = chip_info[1];  // Device ID Byte 1
  uint8_t id2 = chip_info[2];  // Device ID Byte 2
  uint8_t edi_len = chip_info[3];
  uint8_t edi_byte1 = chip_info[4];

  if (id0 == 0x1F)
    manufacturer = "Atmel / Renesas";

  if ((id1 >> 5) == 0x01)
    family = "AT45DBxxx (DataFlash)";

  uint8_t density_code = id1 & 0x1F;

  switch (density_code)
  {
    case 0x00:
      capacity_str = "1 Mbit";
      break;
    case 0x01:
      capacity_str = "2 Mbit";
      break;
    case 0x02:
      capacity_str = "4 Mbit";
      break;
    case 0x03:
      capacity_str = "8 Mbit";
      break;
    case 0x04:
      capacity_str = "16 Mbit";
      break;
    case 0x05:
      capacity_str = "32 Mbit";
      break;
    case 0x08:
      capacity_str = "64 Mbit";
      break;
    case 0x09:
      capacity_str = "128 Mbit";
      break;
    case 0x0A:
      capacity_str = "256 Mbit";
      break;
    default:
      capacity_str = "알 수 없음";
      break;
  }

  DEBUG_PRINTF("Flash 메모리 \r\n");
  DEBUG_PRINTF("  Manufacturer ID : 0x%02X (%s)\r\n", id0, manufacturer);
  DEBUG_PRINTF("  Device ID Byte 1: 0x%02X\r\n", id1);
  DEBUG_PRINTF("    - Family Code : 0x%02X (%s)\r\n", id1 >> 5, family);
  DEBUG_PRINTF("    - Density Code: 0x%02X (%s)\r\n", density_code, capacity_str);

  DEBUG_PRINTF("  Device ID Byte 2: 0x%02X\r\n", id2);
  DEBUG_PRINTF("    - Sub Code     : 0x%02X (%s)\r\n", id2 >> 3, subcode);
  DEBUG_PRINTF("    - Variant Code : 0x%02X (%s)\r\n", id2 & 0x07, variant);

  DEBUG_PRINTF("  Extended Info Len: 0x%02X (EDI Byte Count)\r\n", edi_len);
  DEBUG_PRINTF("  EDI Byte[0]      : 0x%02X\r\n", edi_byte1);

  uint8_t rev_code = edi_byte1 & 0x1F;
  if (rev_code == 0)
    revision_str = "00000 (Initial Version)";
  else
    revision_str = "알 수 없는 버전";

  DEBUG_PRINTF("    - Device Revision : %02X (%s)\r\n", rev_code, revision_str);
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

  at45db_parse_chip_info(chip_info);

  if((chip_info[1]&0x07)== 0x7)// density code  bit4 ~bit0  0x03 32Mb, 
  {
    DEBUG_PRINTF("512페이지\r\n");
    s_page_size = 512;
  }

  osDelay(1);

  AT45_RegRead(drv,STATUS_REGISTER, &reg, 1);//01 0111 10
  
  //reg:BD 10111101

  if ((reg & 0x01) == 0)
  {
    DEBUG_PRINTF("256페이지\r\n");
    AT45_RegWrite(drv, (uint8_t *)protect_enable);
    AT45_IsBusy(drv);
    AT45_RegWrite(drv, (uint8_t *)page512table);  // 256page 설정
    AT45_IsBusy(drv);
  }


  //안되는 보드 0xDE   11 0111 10
  AT45_Delay(10);

}

#define FLASH_PAGE_SIZE 512

/**
 * @brief 플래시를 ram처럼 연속 접근
 */
void at45db_read(driver_t *drv, uint32_t offset, uint8_t *p_buff, uint32_t buff_size,
                 uint32_t read_len)
{
  uint8_t buff[FLASH_PAGE_SIZE];
  uint32_t page_quot = offset / FLASH_PAGE_SIZE;
  uint32_t page_rem = offset % FLASH_PAGE_SIZE;
  uint32_t remain = FLASH_PAGE_SIZE - page_rem;
  uint32_t read_cnt = 0;

  // 버퍼 크기 체크
  if (read_len > buff_size)
  {
    read_len = buff_size;
  }

  OS_PEND_SEM(drv->sem, osWaitForever);

  // Case 1: 시작 위치가 페이지의 중간인 경우 (unaligned)
  if (page_rem > 0)
  {
    at45db_read_page(drv, page_quot, buff);
    uint32_t first_read = (read_len < remain) ? read_len : remain;
    memcpy(&p_buff[0], &buff[page_rem], first_read);
    page_quot++;
    read_cnt += first_read;
  }

  // Case 2: 중간 full page들
  while ((read_len - read_cnt) >= FLASH_PAGE_SIZE)
  {
    at45db_read_page(drv, page_quot, buff);
    memcpy(&p_buff[read_cnt], buff, FLASH_PAGE_SIZE);
    page_quot++;
    read_cnt += FLASH_PAGE_SIZE;
  }

  // Case 3: 마지막 페이지 일부 읽기 (if 남은 게 있다면)
  if (read_len > read_cnt)
  {
    at45db_read_page(drv, page_quot, buff);
    memcpy(&p_buff[read_cnt], buff, read_len - read_cnt);
  }

  OS_POST_SEM(drv->sem);
}


int32_t at45db_write(driver_t *drv, uint32_t offset, uint8_t *p_data, uint32_t data_len)
{
  uint8_t buff[FLASH_PAGE_SIZE];
  uint32_t page_quot = offset / FLASH_PAGE_SIZE;
  uint32_t page_rem = offset % FLASH_PAGE_SIZE;
  uint32_t remain = FLASH_PAGE_SIZE - page_rem;
  uint32_t written = 0;

  OS_PEND_SEM(drv->sem, osWaitForever);

  // 시작 위치가 페이지의 중간인 경우 (unaligned)
  if (page_rem > 0)
  {
    at45db_read_page(drv, page_quot, buff);
    uint32_t first_write = (data_len < remain) ? data_len : remain;
    memcpy(&buff[page_rem], &p_data[0], first_write);
    at45db_write_page(drv, page_quot, buff);
    page_quot++;
    written += first_write;
  }

  //  중간 full page들
  while ((data_len - written) >= FLASH_PAGE_SIZE)
  {
    memcpy(buff, &p_data[written], FLASH_PAGE_SIZE);
    at45db_write_page(drv, page_quot, buff);
    page_quot++;
    written += FLASH_PAGE_SIZE;
  }

  //  마지막 페이지 일부 쓰기 (if 남은 게 있다면)
  if (data_len > written)
  {
    at45db_read_page(drv, page_quot, buff);
    memcpy(buff, &p_data[written], data_len - written);
    at45db_write_page(drv, page_quot, buff);
  }

  OS_POST_SEM(drv->sem);
  return 0;
}