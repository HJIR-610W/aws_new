

#include "fm25cl.h"

#include "driver_do.h"
#include "driver_fram_define.h"
#include "driver_stm32_spi.h"
#include "os_user_def.h"
#include "system_err.h"

#define FRAM_LOCK_USE 1 /* Mutex 사용할지 선택 */

#define FRAM_1024     0 /* FRAM 용량  1이면 1024용량 사용*/

#define WRSR 	0x01
#define WRITE 0x02
#define READ 	0x03
#define WRDI 	0x04
#define RDSR 	0x05
#define WREN 	0x06

typedef struct fm25lc_cfg_s
{
  driver_t *spi_io;
  driver_t *cs_io;
  driver_t *sem;
} fm25lc_cfg_t;

void fm25cl_read(driver_t *fm25cl, uint32_t offset, uint8_t *pBuff,
                 uint16_t rLen) ;
void fm25cl_write(driver_t *fm25cl, uint32_t offset, uint8_t *pData, uint16_t wLen);


    driver_t g_fm25cl;
fm25lc_cfg_t g_fm25lc_cfg;
const fram_api_t fram_api = {.read = fm25cl_read, .write = fm25cl_write};

driver_t *fm25lc_open(void)
{
  if (g_fm25cl.opened)
  {
    return &g_fm25cl;
  }
  g_fm25cl.opened = true;
  g_fm25cl.api = &fram_api;
  
  g_fm25cl.cfg = &g_fm25lc_cfg;

  g_fm25lc_cfg.spi_io = driver_spi_open(STM_SPI_1);     // IC 사용해 필요한 하드웨어 연결
  g_fm25lc_cfg.cs_io = driver_do_open(DO_FRAM_CS, 0);   // IC 사용에 필요한 하드웨여 연결

  OS_CREATE_BINARY_SEM(g_fm25lc_cfg.sem);

  return &g_fm25cl;
}


 void fram_cmd(driver_t *fm25cl,uint8_t cmd)
{
   fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;

  driver_do_low(cfg->cs_io);    

  driver_spi_send_byte(cfg->spi_io,cmd);
  driver_do_high(cfg->cs_io);
}



void fm25cl_write(driver_t *fm25cl,uint32_t offset,uint8_t *pData,uint16_t wLen)
{
  fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;

  
#if !FRAM_1024
   fram_cmd(fm25cl,WREN);
#endif

    driver_do_low(cfg->cs_io);

    driver_spi_send_byte(cfg->spi_io,WRITE);
#if FRAM_1024
    FRAM_SPI_WRITE_BYTE((addr>>16)&0xFF);
#endif
    driver_spi_send_byte(cfg->spi_io,(offset>>8)&0xFF);
    driver_spi_send_byte(cfg->spi_io,offset&0xFF);

    osDelay(1);
    driver_spi_send_bytes(cfg->spi_io,pData,wLen);
    driver_do_high(cfg->cs_io);

}

void fm25cl_read(driver_t *fm25cl,uint32_t offset,uint8_t *pBuff,uint16_t rLen)
{
  fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;
  uint32_t i;

  driver_do_low(cfg->cs_io);

  driver_spi_send_byte(cfg->spi_io, READ);
#if FRAM_1024
        driver_spi_send_byte(cfg->spi_io,(offset>>16)&0xFF);
#endif    
        driver_spi_send_byte(cfg->spi_io,(offset>>8)&0xFF);
        driver_spi_send_byte(cfg->spi_io,offset&0xFF);

    for(i=0;i<rLen;i++)
    {
        pBuff[i] = driver_spi_read_byte(cfg->spi_io);

    }
    
    driver_do_high(cfg->cs_io);
  

}




void fm25_status_parse(uint8_t status)
{

  DEBUG_PRINTF("Status Register: 0x%02X\r\n", status);

  // WPEN: Write Protect Enable (Bit 7)
  if (status & (1 << 7))
    DEBUG_PRINTF("  WPEN = 1 → WP 핀의 쓰기 보호 기능이 활성화됨\r\n");
  else
    DEBUG_PRINTF("  WPEN = 0 → WP 핀 무시\r\n");

  // Bits 6~4: Don't care, always 0
  if (status & 0x70)
    DEBUG_PRINTF("  [경고] Bit 4~6이 0이 아님 (예상치 못한 값)\r\n");

  // BP1/BP0: Block Protect
  uint8_t bp = (status >> 2) & 0x03;
  const char *bp_desc;
  switch (bp)
  {
    case 0:
      bp_desc = "보호 안함";
      break;
    case 1:
      bp_desc = "상위 1/4 (0x1800~0x1FFF) 보호";
      break;
    case 2:
      bp_desc = "상위 1/2 (0x1000~0x1FFF) 보호";
      break;
    case 3:
      bp_desc = "전체 보호 (0x0000~0x1FFF)";
      break;
    default:
      bp_desc = "알 수 없음";
      break;
  }
  DEBUG_PRINTF("  BP1:BP0 = %d:%d → %s\r\n", (bp >> 1) & 1, bp & 1, bp_desc);

  // WEL: Write Enable Latch (Bit 1)
  if (status & (1 << 1))
    DEBUG_PRINTF("  WEL = 1 → 쓰기 가능 상태\r\n");
  else
    DEBUG_PRINTF("  WEL = 0 → 쓰기 비활성화 상태\r\n");

  // Bit 0: 항상 0 (읽기 전용)
  if (status & 0x01)
    DEBUG_PRINTF("  [주의] Bit 0이 1로 설정됨 (비정상 상태)\r\n");
}

uint8_t fm25cl_read_status(driver_t *fm25cl)
{
  fm25lc_cfg_t *cfg=(fm25lc_cfg_t*)fm25cl->cfg;
  uint8_t data=0;
    

    driver_do_low(cfg->cs_io);
    driver_spi_send_byte(cfg->spi_io,RDSR);
    data = driver_spi_read_byte(cfg->spi_io);
    driver_do_high(cfg->cs_io);

    fm25_status_parse(data);


    return data;
    
}

uint8_t data;
void fm25cl_init(driver_t *fm25cl)
{

  data = fm25cl_read_status(fm25cl);
}