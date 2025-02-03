#include "cmsis_os.h"
#include "ads1220.h"
#include "driver_do.h"
#include "driver_spi.h"
#include "driver_di.h"
#include "mcu_delay.h"
#include "mcu_interrupt.h"

osSemaphoreId_t g_dataReadySem=NULL;

void write_reg(driver_t *drv,uint8_t startAddress,uint8_t numRegs,uint8_t *pData)
{
    uint32_t i;
    uint8_t data;
    ads1220_cfg_t *cfg=(ads1220_cfg_t*)drv->cfg;


    driverex_spi_pend_sem(cfg->spi_io);

    driver_do_low(cfg->cs_io);

    mcu_delay(50);

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

    mcu_delay(50);
    
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
    
    mcu_delay(50);
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

    if(status == osEventTimeout)
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
    ads1220_cfg_t *cfg=(ads1220_cfg_t*)drv->cfg;

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


driver_t ads1220;
ads1220_cfg_t ads1210_cfg;

driver_t *ads1220_open(void)
{
    if(ads1220.opened == false)
    {
        ads1220.opened = true;

        ads1220.cfg = &ads1210_cfg;
    }

    return &ads1220;
}
