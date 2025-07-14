

#include "fm25cl.h"

#include "bsp_do.h"
#include "bsp_spi.h"
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

typedef struct fm25lc_instance_s
{
  int spi_num;
  int cs_do_num;
  void *sem;
  bool opened;
} fm25lc_instance_t;

void fm25cl_read( uint32_t offset, uint8_t *pBuff,uint16_t rLen) ;
void fm25cl_write( uint32_t offset, uint8_t *pData, uint16_t wLen);
uint8_t fm25cl_read_status(void);


fm25lc_instance_t fm25lc_inst;


void fm25lc_init(void)
{
  uint8_t data;
  
  if (fm25lc_inst.opened)
  {
    return;
  }
  fm25lc_inst.opened = true;
  fm25lc_inst.spi_num = BSP_SPI_1;     // IC 사용해 필요한 하드웨어 연결
  fm25lc_inst.cs_do_num = BSP_DO_FRAM_CS;

  OS_CREATE_BINARY_SEM(fm25lc_inst.sem);
  bsp_spi_init(fm25lc_inst.spi_num);

  data = fm25cl_read_status();
}


 void fram_cmd(uint8_t cmd)
{
  bsp_do_low(fm25lc_inst.cs_do_num);
  bsp_spi_send_byte(fm25lc_inst.spi_num, cmd);
  bsp_do_high(fm25lc_inst.cs_do_num);
}

void fm25cl_write(uint32_t offset,uint8_t *pData,uint16_t wLen)
{
  
#if !FRAM_1024
   fram_cmd(WREN);
#endif
    bsp_do_low(fm25lc_inst.cs_do_num);
    bsp_spi_send_byte(fm25lc_inst.spi_num,WRITE);
#if FRAM_1024
    FRAM_SPI_WRITE_BYTE((addr>>16)&0xFF);
#endif
    bsp_spi_send_byte(fm25lc_inst.spi_num,(offset>>8)&0xFF);
    bsp_spi_send_byte(fm25lc_inst.spi_num,offset&0xFF);

    osDelay(1);
    bsp_spi_send_bytes(fm25lc_inst.spi_num,pData,wLen);
    bsp_do_high(fm25lc_inst.cs_do_num);

}

void fm25cl_read(uint32_t offset,uint8_t *pBuff,uint16_t rLen)
{
  uint32_t i;

  OS_PEND_SEM(fm25lc_inst.sem, osWaitForever);
  bsp_do_low(fm25lc_inst.cs_do_num);

  bsp_spi_send_byte(fm25lc_inst.spi_num, READ);
#if FRAM_1024
        bsp_spi_send_byte(fm25lc_inst.spi_num,(offset>>16)&0xFF);
#endif    
        bsp_spi_send_byte(fm25lc_inst.spi_num,(offset>>8)&0xFF);
        bsp_spi_send_byte(fm25lc_inst.spi_num,offset&0xFF);

    for(i=0;i<rLen;i++)
    {
        pBuff[i] = bsp_spi_read_byte(fm25lc_inst.spi_num);

    }
    
    bsp_do_high(fm25lc_inst.cs_do_num);

    OS_POST_SEM(fm25lc_inst.sem);
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

uint8_t fm25cl_read_status(void)
{
  uint8_t data=0;

  bsp_do_low(fm25lc_inst.cs_do_num);
  bsp_spi_send_byte(fm25lc_inst.spi_num, RDSR);
  data = bsp_spi_read_byte(fm25lc_inst.spi_num);
  bsp_do_high(fm25lc_inst.cs_do_num);

  fm25_status_parse(data);


  return data;

}
