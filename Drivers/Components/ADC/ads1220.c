
#include "cmsis_os2.h"

#include "ads1220_reg.h"
#include "ads1220.h"
#include "driver_adc_define.h"
#include "driver_adc.h"
#include "driver_do.h"
#include "driver_spi.h"
#include "driver_di.h"
#include "driver_mux.h"
#include "usDelay.h"
#include "mcu_interrupt.h"
#include "utile.h"

typedef struct ads1220_cfg_s
{
  void *spi_io;
  void *cs_io;
  void *irq_io;
  void *sem;
  uint8_t diffChCnt;
  uint8_t singleChCnt;
}ads1220_cfg_t;

osSemaphoreId_t g_dataReadySem=NULL;

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
 * 
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
       
    status = osSemaphoreAcquire(g_dataReadySem, 5);//타임아웃 5ms 줌

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
    
    ads1220_set_singleChannel(drv,ch);
    osDelay(2);
    data = ads1220_read_adc(drv,err);
     
    return data;
}

int32_t ads1220_read_diff_ch(driver_t *drv,int32_t ch,uint8_t *err)
{
    int32_t data;

    ads1220_set_diffChannel(drv,ch);

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

    reg = 0x01;//PGA disable  

    write_reg(drv,ADS1220_REG_0, 1, &reg);  

    reg = 0xc0;
    write_reg(drv,ADS1220_REG_1, 1, &reg);

    reg = (0x01)<<6;
    write_reg(drv,ADS1220_REG_2, 1, &reg);

    reg = 0x00;
    write_reg(drv,ADS1220_REG_3, 1, &reg);

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

const uint8_t user_adc_single_channel[18]={0,1,4,5,8,9,12,13,16,17,20,21,24,25,28,29,2,6};
int32_t ads1220_single_read(driver_t *drv,int channel,uint16_t avg,uint8_t *err)
{

  uint32_t diff_ch;
  int32_t adc;
  int32_t sum=0;
  uint8_t valid_cnt=0;

  osSemaphoreAcquire(drv->sem, osWaitForever);

  channel = user_adc_single_channel[channel];

  adc_single_mux_set(channel);
  
  for(int i = 0 ; i< avg; i++)
  {
    adc = ads1220_read_single_ch(drv,channel%4,err);
  
    if(*err ==0)
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
  uint32_t diff_ch;
  int32_t adc;
  uint8_t valid_cnt=0;
  float average=0;

  osSemaphoreAcquire(drv->sem, osWaitForever);

  //차동 채널 0,1,2,3,4,5,6,7 은 ADS1220에서는 0채널로만 측정하며  MUX가 채널이 됨
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
 
  adc = average;

  osSemaphoreRelease(drv->sem);  // 세마포어 해제

  return adc;

}


void ads1220_set(driver_t *drv, adc_set_option_t option, void *value)
{

  osSemaphoreAcquire(drv->sem, osWaitForever);

    osSemaphoreRelease(drv->sem);  // 세마포어 해제
}
