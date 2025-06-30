
#include "ads1220.h"

#include "adc_calibration.h"
#include "ads1220_reg.h"
#include "cmsis_os2.h"
#include "dev_io.h"
#include "driver_adc.h"
#include "driver_adc_define.h"
#include "driver_di.h"
#include "driver_do.h"
#include "driver_mux.h"
#include "driver_spi.h"
#include "mcu_interrupt.h"
#include "os_user_def.h"
#include "usDelay.h"
#include "util_memory.h"
#include "system_err.h"

typedef struct ads1220_cfg_s
{
  driver_t *spi_io;
  driver_t *cs_io;
  driver_t *irq_io;
  void *sem;
  uint8_t diffChCnt;
  uint8_t singleChCnt;
}ads1220_cfg_t;



void read_reg(driver_t *drv,uint8_t startAddress,uint8_t numRegs, uint8_t *pBuff);


osSemaphoreId_t g_dataReadySem=NULL;

void parse_ads1220_register(driver_t *drv)
{
  uint8_t reg;

  DEBUG_PRINTF("\r\n==== ADS1220 레지스터 설정값==== \r\n");
  // Register 0: MUX[7:4], GAIN[3:1], PGA_BYPASS[0]
  read_reg(drv, ADS1220_REG_0, 1, &reg);
  DEBUG_PRINTF("REG0 (0x%02X): 0x%02X\r\n", ADS1220_REG_0, reg);

  DEBUG_PRINTF("  MUX       [7:4]: ");
  switch ((reg >> 4) & 0x0F)
  {
    case 0x0:
      DEBUG_PRINTF("0000 - AIN0 - AIN1 (기본값)");
      break;
    case 0x1:
      DEBUG_PRINTF("0001 - AIN0 - AIN2");
      break;
    case 0x2:
      DEBUG_PRINTF("0010 - AIN0 - AIN3");
      break;
    case 0x3:
      DEBUG_PRINTF("0011 - AIN1 - AIN2");
      break;
    case 0x4:
      DEBUG_PRINTF("0100 - AIN1 - AIN3");
      break;
    case 0x5:
      DEBUG_PRINTF("0101 - AIN2 - AIN3");
      break;
    case 0x6:
      DEBUG_PRINTF("0110 - AIN1 - AIN0");
      break;
    case 0x7:
      DEBUG_PRINTF("0111 - AIN3 - AIN2");
      break;
    case 0x8:
      DEBUG_PRINTF("1000 - AIN0 - AVSS");
      break;
    case 0x9:
      DEBUG_PRINTF("1001 - AIN1 - AVSS");
      break;
    case 0xA:
      DEBUG_PRINTF("1010 - AIN2 - AVSS");
      break;
    case 0xB:
      DEBUG_PRINTF("1011 - AIN3 - AVSS");
      break;
    case 0xC:
      DEBUG_PRINTF("1100 - REFP0 - REFN0");
      break;
    case 0xD:
      DEBUG_PRINTF("1101 - AVDD - AVSS (모니터)");
      break;
    case 0xE:
      DEBUG_PRINTF("1110 - AINP - AINN shorted");
      break;
    case 0xF:
      DEBUG_PRINTF("1111 - Reserved");
      break;
  }
  DEBUG_PRINTF("        // 입력 다중 선택\r\n");

  DEBUG_PRINTF("  GAIN      [3:1]: ");
  switch ((reg >> 1) & 0x07)
  {
    case 0:
      DEBUG_PRINTF("000 - Gain = 1 (기본값)");
      break;
    case 1:
      DEBUG_PRINTF("001 - Gain = 2");
      break;
    case 2:
      DEBUG_PRINTF("010 - Gain = 4");
      break;
    case 3:
      DEBUG_PRINTF("011 - Gain = 8");
      break;
    case 4:
      DEBUG_PRINTF("100 - Gain = 16");
      break;
    case 5:
      DEBUG_PRINTF("101 - Gain = 32");
      break;
    case 6:
      DEBUG_PRINTF("110 - Gain = 64");
      break;
    case 7:
      DEBUG_PRINTF("111 - Gain = 128");
      break;
  }
  DEBUG_PRINTF("              // PGA 이득 설정\r\n");

  DEBUG_PRINTF("  PGA Bypass[0]  : %s             // 내부 저잡음 PGA 우회 여부\r\n",
            (reg & 0x01) ? "1 - 우회함 (Bypassed)" : "0 - 사용함 (Enabled)");

  // Register 1: DR[7:5], MODE[4:3], CM[2], TS[1], BCS[0]
  read_reg(drv, ADS1220_REG_1, 1, &reg);
  DEBUG_PRINTF("REG1 (0x%02X): 0x%02X\r\n", ADS1220_REG_1, reg);

  DEBUG_PRINTF("  Data rate [7:5]: ");
  switch ((reg >> 5) & 0x07)
  {
    case 0:
      DEBUG_PRINTF("000 - 20 SPS (Normal)\r\n");
      break;
    case 1:
      DEBUG_PRINTF("001 - 45 SPS (Normal)\r\n");
      break;
    case 2:
      DEBUG_PRINTF("010 - 90 SPS (Normal)\r\n");
      break;
    case 3:
      DEBUG_PRINTF("011 - 175 SPS (Normal)\r\n");
      break;
    case 4:
      DEBUG_PRINTF("100 - 330 SPS (Normal)\r\n");
      break;
    case 5:
      DEBUG_PRINTF("101 - 600 SPS (Normal)\r\n");
      break;
    case 6:
      DEBUG_PRINTF("110 - 1000 SPS (Normal)\r\n");
      break;
    case 7:
      DEBUG_PRINTF("111 - Reserved\r\n");
      break;
  }
  DEBUG_PRINTF("                        // 출력 샘플링 속도 설정\r\n");

  DEBUG_PRINTF("  Mode      [4:3]: ");
  switch ((reg >> 3) & 0x03)
  {
    case 0:
      DEBUG_PRINTF("00 - Normal mode\r\n");
      break;
    case 1:
      DEBUG_PRINTF("01 - Duty-cycle mode\r\n");
      break;
    case 2:
      DEBUG_PRINTF("10 - Turbo mode\r\n");
      break;
    case 3:
      DEBUG_PRINTF("11 - Reserved\r\n");
      break;
  }
  DEBUG_PRINTF("                        // 변환 클럭 동작 모드 설정\r\n");

  DEBUG_PRINTF("  CM        [2]  : %s                // 공통 모드 제거 기능\r\n",
            (reg & 0x04) ? "1 - Enabled" : "0 - Disabled");
  DEBUG_PRINTF("  Temp Sensor[1] : %s              // 내부 온도 센서 사용\r\n",
            (reg & 0x02) ? "1 - Enabled" : "0 - Disabled");
  DEBUG_PRINTF("  Burn-out  [0]  : %s                // 10uA 번아웃 전류 소스\r\n",
            (reg & 0x01) ? "1 - On" : "0 - Off (기본값)");

  // Register 2: VREF[7:6], 50/60[5:4], PSW[3], IDAC[2:0]
  read_reg(drv, ADS1220_REG_2, 1, &reg);
  DEBUG_PRINTF("REG2 (0x%02X): 0x%02X\r\n", ADS1220_REG_2, reg);

  DEBUG_PRINTF("  VREF      [7:6]: ");
  switch ((reg >> 6) & 0x03)
  {
    case 0:
      DEBUG_PRINTF("00 - Internal 2.048V (기본값)");
      break;
    case 1:
      DEBUG_PRINTF("01 - External REF0 사용");
      break;
    case 2:
      DEBUG_PRINTF("10 - AIN0/REFP1, AIN3/REFN1");
      break;
    case 3:
      DEBUG_PRINTF("11 - AVDD - AVSS 사용");
      break;
  }
  DEBUG_PRINTF("        // 기준 전압 선택\r\n");

  DEBUG_PRINTF("  50/60Hz Rej[5:4]: ");
  switch ((reg >> 4) & 0x03)
  {
    case 0:
      DEBUG_PRINTF("00 - 필터 비활성화(기본값)");
      break;
    case 1:
      DEBUG_PRINTF("01 - 50Hz & 60Hz 동시 제거");
      break;
    case 2:
      DEBUG_PRINTF("10 - 50Hz 제거만");
      break;
    case 3:
      DEBUG_PRINTF("11 - 60Hz 제거만");
      break;
  }
  DEBUG_PRINTF("    // FIR 필터 구성\r\n");

  DEBUG_PRINTF("  PSW       [3]  : %s           // Low-side 스위치 동작 설정\r\n",
            (reg & 0x08) ? "1 - 자동 동작" : "0 - 항상 열림(기본값)");

  DEBUG_PRINTF("  IDAC Curr[2:0]: ");
  switch (reg & 0x07)
  {
    case 0:
      DEBUG_PRINTF("000 - Off (기본값)");
      break;
    case 1:
      DEBUG_PRINTF("001 - 10 uA");
      break;
    case 2:
      DEBUG_PRINTF("010 - 50 uA");
      break;
    case 3:
      DEBUG_PRINTF("011 - 100 uA");
      break;
    case 4:
      DEBUG_PRINTF("100 - 250 uA");
      break;
    case 5:
      DEBUG_PRINTF("101 - 500 uA");
      break;
    case 6:
      DEBUG_PRINTF("110 - 1000 uA");
      break;
    case 7:
      DEBUG_PRINTF("111 - 1500 uA");
      break;
  }
  DEBUG_PRINTF("           // IDAC1 및 IDAC2 전류 설정\r\n");

  // Register 3: IDAC1[7:5], IDAC2[4:2], GPIO_DIR[1], GPIO_DAT[0]
  read_reg(drv, ADS1220_REG_3, 1, &reg);
  // Register 3: I1MUX[7:5], I2MUX[4:2], DRDYM[1], Reserved[0]
  read_reg(drv, ADS1220_REG_3, 1, &reg);
  DEBUG_PRINTF("REG3 (0x%02X): 0x%02X\r\n", ADS1220_REG_3, reg);

  DEBUG_PRINTF("  IDAC1 MUX [7:5]: ");
  switch ((reg >> 5) & 0x07)
  {
    case 0:
      DEBUG_PRINTF("000 - Disabled (기본값)");
      break;
    case 1:
      DEBUG_PRINTF("001 - AIN0/REFP1");
      break;
    case 2:
      DEBUG_PRINTF("010 - AIN1");
      break;
    case 3:
      DEBUG_PRINTF("011 - AIN2");
      break;
    case 4:
      DEBUG_PRINTF("100 - AIN3/REFN1");
      break;
    case 5:
      DEBUG_PRINTF("101 - REFP0");
      break;
    case 6:
      DEBUG_PRINTF("110 - REFN0");
      break;
    case 7:
      DEBUG_PRINTF("111 - Reserved");
      break;
  }
  DEBUG_PRINTF("        // IDAC1 라우팅 채널 설정\r\n");

  DEBUG_PRINTF("  IDAC2 MUX [4:2]: ");
  switch ((reg >> 2) & 0x07)
  {
    case 0:
      DEBUG_PRINTF("000 - Disabled (기본값)");
      break;
    case 1:
      DEBUG_PRINTF("001 - AIN0/REFP1");
      break;
    case 2:
      DEBUG_PRINTF("010 - AIN1");
      break;
    case 3:
      DEBUG_PRINTF("011 - AIN2");
      break;
    case 4:
      DEBUG_PRINTF("100 - AIN3/REFN1");
      break;
    case 5:
      DEBUG_PRINTF("101 - REFP0");
      break;
    case 6:
      DEBUG_PRINTF("110 - REFN0");
      break;
    case 7:
      DEBUG_PRINTF("111 - Reserved");
      break;
  }
  DEBUG_PRINTF("        // IDAC2 라우팅 채널 설정\r\n");

  DEBUG_PRINTF("  DRDY Mode  [1] : %s              // DRDY 핀 동작 방식\r\n",
            (reg & 0x02) ? "1 - DOUT/DRDY와 DRDY 동시에 출력" : "0 - DRDY 전용 핀 사용 (기본값)");

  DEBUG_PRINTF("  Reserved   [0] : %d                    // 예약비트 (항상 0)\r\n", reg & 0x01);

  DEBUG_PRINTF("\r\n==========\r\n");
}

void write_reg(driver_t *drv,uint8_t startAddress,uint8_t numRegs,uint8_t *pData)
{
    uint32_t i;
    uint8_t data;
    ads1220_cfg_t *cfg=(ads1220_cfg_t*)drv->cfg;


    driverex_spi_pend_sem(cfg->spi_io);

    driver_do_low(cfg->cs_io);

    usDelay(50);

    data = ADS1220_CMD_WREG | (((startAddress<<2) & 0x0c) |((numRegs-1)&0x03));
 
    driverex_spi_send_byte(cfg->spi_io,data);

    for (i=0; i< numRegs; i++)
    {
        driverex_spi_send_byte(cfg->spi_io,*pData++);
    }
   
    driver_do_high( cfg->cs_io);

    driverex_spi_post_sem(cfg->spi_io);
}

void read_reg(driver_t *drv,uint8_t startAddress,uint8_t numRegs, uint8_t *pBuff)
{
    uint32_t i;
    uint8_t val=0;
    uint8_t data;
    ads1220_cfg_t *cfg=(ads1220_cfg_t*)drv->cfg;

    driverex_spi_pend_sem(cfg->spi_io);

    driver_do_low(cfg->cs_io);

    usDelay(50);
    
    data = (ADS1220_CMD_RREG | (((startAddress<<2) & 0x0c) |((numRegs-1)&0x03)));

    driverex_spi_send_byte(cfg->spi_io,data);

    for (i=0; i< numRegs; i++)
    {
        val = driverex_spi_read_byte(cfg->spi_io);
        *pBuff++ = val;
    }
   
    driver_do_high( cfg->cs_io);
    
    driverex_spi_post_sem(cfg->spi_io);

}

void ads1220_start_conv(driver_t *drv)
{
    ads1220_cfg_t *cfg=(ads1220_cfg_t*)drv->cfg;

    driverex_spi_pend_sem(cfg->spi_io);

    driver_do_low( cfg->cs_io);
    
    usDelay(50);
    driverex_spi_send_byte(cfg->spi_io,ADS1220_CMD_SYNC);
   
    driver_do_high( cfg->cs_io);
    
   driverex_spi_post_sem(cfg->spi_io);
    
}

void ads1220_set_singleChannel(driver_t *drv,uint32_t ch)
{
    uint8_t reg=0;
 
    read_reg(drv,ADS1220_REG_0, 0x01, &reg);
        
    reg = (reg&0x0F) |((ch<<4)|0x80);
   
    write_reg(drv,ADS1220_REG_0,0x01,&reg);

}

void ads1220_set_diffChannel(driver_t *drv,uint32_t ch)
{
  uint8_t reg=0;
    
  read_reg(drv,ADS1220_REG_0, 0x01, &reg);

  switch(ch)
  {
    case 0:
       reg = (reg&0x0F) |ADS1220_MUX_AIN0_AIN1;
    break;
    case 1:
       reg = (reg&0x0F) |  ADS1220_MUX_AIN2_AIN3;
    break;
  }
    
    write_reg(drv,ADS1220_REG_0,0x01,&reg);

}

void ads1220_reset_sw(driver_t *drv)
{
    ads1220_cfg_t *cfg=(ads1220_cfg_t*)drv->cfg;

    driverex_spi_pend_sem(cfg->spi_io);

    driver_do_low(cfg->cs_io);

    driverex_spi_send_byte(cfg->spi_io,ADS1220_CMD_RESET);

    driver_do_high(cfg->cs_io);

    driverex_spi_post_sem(cfg->spi_io);
}

/**
 * @brief
 * 
 * 1LSB = (2*Vref/Gain)/s^24
 * 양의 최대 값 0x7FFFFF  8388607
 * 음의 최대 값 0x800000 -8388608
 * Vref = 5V
 * 1LSB = 0.000001192092896
 * 0.596 uV
 * 
 * Vref = 6V
 * 0.715 uV
 * 
 * 양의 최대 입력값은 Vref/Gain-1LSB
 * Vref= 5V이면        4.999999404V
 * 음의 최대 입력값은 -4.999999404V
 * ADC = (Vin/Vref)*2^23 
 *
 */

int32_t ads1220_read_adc(driver_t *drv,uint8_t *err)
{
    osStatus status;
    int32_t data=0;
    ads1220_cfg_t *cfg=(ads1220_cfg_t*)drv->cfg;
    
    *err = 1;

    osSemaphoreAcquire(g_dataReadySem,0);
    ads1220_start_conv(drv);
       
    status = osSemaphoreAcquire(g_dataReadySem, 60);//타임아웃 5ms 줌

    if(status == osErrorTimeout)
    {
        *err = 2;
        return 0;
    }

    driverex_spi_pend_sem(cfg->spi_io);
    driver_do_low(cfg->cs_io);
    //이 명령어 전송되면 drdy 핀 올라감
    driverex_spi_send_byte(cfg->spi_io,ADS1220_CMD_RDATA);

    data = driverex_spi_read_byte(cfg->spi_io);
    data = (data << 8) |driverex_spi_read_byte(cfg->spi_io);
    data = (data << 8) |driverex_spi_read_byte(cfg->spi_io);

    if (data & 0x00800000)
    {
        data |= 0xff000000;
    }

    driver_do_high( cfg->cs_io);
    *err = 0;
    driverex_spi_post_sem(cfg->spi_io);
    
    return data;
}

int32_t ads1220_read_single_ch(driver_t *drv,int32_t ch,uint8_t *err)
{
    int32_t data=0;
    


    data = ads1220_read_adc(drv,err);
     
    return data;
}

int32_t ads1220_read_diff_ch(driver_t *drv,int32_t ch,uint8_t *err)
{
    int32_t data;



    data = ads1220_read_adc(drv,err);
     
    return data;
}

void irq_dataReady(void *arg)
{
    osSemaphoreRelease(g_dataReadySem);
}

void ads1210_init(driver_t *drv)
{
    uint8_t reg;
    di_isr_set_cfg_t isr_cfg;
    ads1220_cfg_t *cfg = (ads1220_cfg_t*)drv->cfg;

    g_dataReadySem = osSemaphoreNew(1, 0, NULL);

    isr_cfg.call    = irq_dataReady;
    isr_cfg.name    = "ads1220_data_ready";
    isr_cfg.trigger = eDI_FALLING;
    isr_cfg.prio    = 5;

    driver_di_set(cfg->irq_io,DI_SET_INTERRUPT,&isr_cfg);

    ads1220_reset_sw(drv);
  
    if(drv->sem == NULL)
    {
        drv->sem = osSemaphoreNew(1, 1, NULL); 
    }

    /*
    gain 1,2,4는 PGA없이 사용가능해서 비활성 가능
    이때는 게인이 스위치드캐패시터구조로 얻어짐
    0 PGA_BYPASS:1b
    */

// ADS1220 Register 0 Configuration
// MUX[7:4] = 0000 - AIN0 - AIN1
// GAIN[3:1] = 000 - Gain 1
// PGA_BYPASS[0] = 0 - PGA Enabled (default)
    reg = 0x00;
 

    write_reg(drv,ADS1220_REG_0, 1, &reg);  

    /*
    7:5 DR   :000b 20sps      데이터 속도
    4:3 MODE :00b             동작 모드
      2 CM   :0b                단일 변환
      1 TS   :0b                온도센서 비활성
      0 BCS  :0b                10uA 전류 소스 비활성
    */
    reg = 0x00;
    reg |= (0x00)<<5;


    write_reg(drv,ADS1220_REG_1, 1, &reg);
    /*
     7:6 VREF   01b REFP0,REFN0
     5:4 50/60  01b 50Hz,60Hz 제거 
       3 PSW     1b 로우사이드 전원 스위치 닫힘
     2:0 IDAC  000b 끄기
    */
    reg =  (0x01)<<6;
    reg |= (0x01)<<3;
    reg |= (0x00)<<4;
    write_reg(drv,ADS1220_REG_2, 1, &reg);

    /*
    7:5 I1MUX 000b IDAC1 비활성화
    4:2 I2MUX 000b IDAC2 비활성화
      1 DRDYM   0b DRDY 핀만 사용
      0 미사용
    */
    reg = 0x00;
    write_reg(drv,ADS1220_REG_3, 1, &reg);

#if DEBUG_PRINTF_USE
    parse_ads1220_register(drv);
#endif
}







void ads1220_close(driver_t *handle);
int32_t ads1220_single_read(driver_t *handle,int channel,uint16_t avg,uint8_t *err);
void ads1220_set(driver_t *handle, adc_set_option_t option, void *value);
int32_t ads1220_diff_read(driver_t *handle,int channel,uint16_t avg,uint8_t *err);


const adc_api_t ads1220_api ={.close = ads1220_close,
                           .read_single = ads1220_single_read,
                           .read_diff = ads1220_diff_read,
                           .set = ads1220_set};

driver_t ads1220_driver;
ads1220_cfg_t ads1220_cfg;


driver_t *ads1220_open(uint32_t num,void *pot)
{
  if(ads1220_driver.opened)
  {
    return &ads1220_driver;
  }

  ads1220_driver.opened = true;

  ads1220_driver.name = "ADC_ADS1220";

  ads1220_cfg.spi_io = driver_spi_open(STM_SPI_2);
  ads1220_cfg.cs_io  = driver_do_open(DO_ADC_NCS,0);
  ads1220_cfg.irq_io = driver_di_open(DI_0_ADC_RDY,0);

  ads1220_driver.cfg = &ads1220_cfg;
  ads1220_driver.api = &ads1220_api;

  if(ads1220_driver.sem == NULL)
  {
    ads1220_driver.sem = osSemaphoreNew(1, 1, NULL); 
  }

  adc_mux_init();
  ads1210_init(&ads1220_driver);

  return &ads1220_driver;
}



void ads1220_close(driver_t *drv)
{
  osSemaphoreAcquire(drv->sem, osWaitForever);

  osSemaphoreRelease(drv->sem);  // 세마포어 해제
}


//논리 채널을 물리채널로 변환 해야 함
const uint8_t user_adc_single_channel[18]={0,1,4,5,8,9,12,13,16,17,20,21,24,25,28,29,2,6};

int32_t ads1220_single_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{
  int32_t adc;
  int32_t sum=0;
  int32_t valid_cnt = 0;

  osSemaphoreAcquire(drv->sem, osWaitForever);

  channel = user_adc_single_channel[channel];

  adc_single_mux_set(channel);

  ads1220_set_singleChannel(drv, channel % 4);

  osDelay(2);//채널 바꾸고 안정화 위해 

  for (int i = 0; i < avg; i++)
  {
    adc = ads1220_read_single_ch(drv,channel%4,err);
  
    if(*err == 0)
    {
      sum += adc;
      valid_cnt++;
    }
  }

  adc = sum/valid_cnt;
  
  osSemaphoreRelease(drv->sem);  // 세마포어 해제
  return adc;
}

int32_t ads1220_diff_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{

  int32_t adc;
  uint8_t valid_cnt=0;
  float average=0;

  osSemaphoreAcquire(drv->sem, osWaitForever);

  //차동 채널 0,1,2,3,4,5,6,7 은 ADS1220에서는 0채널로만 측정하며  MUX가 채널이 됨

  ads1220_set_diffChannel(drv, channel / 8);

  adc_diff_mux_set(channel);

  for(int i = 0 ; i< avg;i++)
  {
    adc = ads1220_read_diff_ch(drv,channel/8,err);
    if(*err ==0)
    {
      valid_cnt++;
      average = recursiveAvg(average,adc,valid_cnt);
    }
  }
 
  adc = (int32_t)average;

  osSemaphoreRelease(drv->sem);  // 세마포어 해제

  return adc;

}


void ads1220_set(driver_t *drv, adc_set_option_t option, void *value)
{


  OS_PEND_SEM(drv->sem,osWaitForever);


  OS_POST_SEM(drv->sem);


}
