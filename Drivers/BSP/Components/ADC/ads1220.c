#include "cmsis_os2.h"
#include "ads1220.h"
#include "driver_digitalOut.h"
#include "driver_spi.h"
#include "driver_digitalIn.h"
#include "mcu_delay.h"

#if 0 


#include "driver_gpio.h"
#include "driver_misc.h"
#include "board_spi.h"

#include "cmsis_os.h"

#define LOW_ADS1220_CS()      LOW_SPI1_CS();
#define HIGH_ADS1220_CS()     HIGH_SPI1_CS();
#define ADS1220_SEND_BYTE(x)  board_spi_send_byte(H_SPI1,x)
#define ADS1220_READ_BYTE()   board_spi_read_byte(H_SPI1)

#define  ADS1220_PEND_SEM() board_spi1_pend_sem();
#define ADS1220_POST_SEM() board_spi1_post_sem();

void write_reg(uint8_t startAddress,uint8_t numRegs,uint8_t *pData)
{
    uint32_t i;

    ADS1220_PEND_SEM();

    LOW_ADS1220_CS();
    misc_delay_us(50);

    ADS1220_SEND_BYTE(ADS1220_CMD_WREG | (((startAddress<<2) & 0x0c) |((numRegs-1)&0x03)));
 
    for (i=0; i< numRegs; i++)
    {
        ADS1220_SEND_BYTE(*pData++);
    }
   
    HIGH_ADS1220_CS();

    ADS1220_POST_SEM();
}

void read_reg(uint8_t startAddress,uint8_t numRegs, uint8_t *pBuff)
{
    uint32_t i;
    uint8_t val=0;

    ADS1220_PEND_SEM();
    LOW_ADS1220_CS();
    misc_delay_us(50);
    ADS1220_SEND_BYTE(ADS1220_CMD_RREG | (((startAddress<<2) & 0x0c) |((numRegs-1)&0x03)));

    for (i=0; i< numRegs; i++)
    {
        val = ADS1220_READ_BYTE();
        *pBuff++ = val;
    }
   
    HIGH_ADS1220_CS();
    
    ADS1220_POST_SEM();
    

}




void ADS1210_set_channel(uint32_t mux)
{
    uint8_t reg=0;
    
    read_reg(ADS1220_REG_0, 0x01, &reg);
        
    reg = (reg&0x0F) |mux;
   
    write_reg(ADS1220_REG_0,0x01,&reg);
}




void ADS1210_reset_sw(void)
{
    ADS1220_PEND_SEM();
    LOW_ADS1220_CS();
   
    ADS1220_SEND_BYTE(ADS1220_CMD_RESET);
   
    HIGH_ADS1220_CS();

        ADS1220_POST_SEM();
}



void ADS1210_start_conv(void)
{
      ADS1220_PEND_SEM();
    LOW_ADS1220_CS();
    misc_delay_us(50);
  
    ADS1220_SEND_BYTE(ADS1220_CMD_SYNC);
   
    HIGH_ADS1220_CS();
            ADS1220_POST_SEM();
    return;
}

void ADS1210_powerDown(void)
{
          ADS1220_PEND_SEM();
    LOW_ADS1220_CS();
  
    ADS1220_SEND_BYTE(ADS1220_CMD_POWERDOWN);
   
    HIGH_ADS1220_CS();
                ADS1220_POST_SEM();
}

#define SINGLE_SHOT_TIMEOUT_MS 100

int32_t AD1220_read_data(int32_t ch,uint8_t *err)
{
    int32_t data=0;
    uint32_t startTime;
    uint8_t muxList[]={ADS1220_MUX_AIN0_AIN1,ADS1220_MUX_AIN2_AIN3};

    
    *err = 1;

      board_spi1_pend_phaseSem();
    ADS1210_set_channel(muxList[ch]);
    osDelay(1);

    //if(!READY_DRDY_ADC())
    {
 
        ADS1210_start_conv();
        
        startTime = osKernelSysTick();

        while(1)
        {
            if((osKernelSysTick()-startTime) > SINGLE_SHOT_TIMEOUT_MS)
            {
                break;
            }
            if(READY_DRDY_ADC())
            {

                ADS1220_PEND_SEM();
                LOW_ADS1220_CS();
                misc_delay_us(50);
                ADS1220_SEND_BYTE(ADS1220_CMD_RDATA);

                data = ADS1220_READ_BYTE();
                data = (data << 8) |ADS1220_READ_BYTE();
                data = (data << 8) |ADS1220_READ_BYTE();

                if (data & 0x00800000)
                {
                    data |= 0xff000000;
                }
            //    data = data<<8;
            //    data = data>>8; // 부호확장
                HIGH_ADS1220_CS();
                *err = 0;
                            ADS1220_POST_SEM();
                break;
            }
        }
    }
    board_spi1_post_phaseSem();
    return data;
}


void ADS1210_init(void)
{
    uint8_t reg;

    reg = 0x00;  
    reg |= (ADS1220_MUX_AIN0_AIN1|ADS1220_PGA_DISABLE|ADS1220_GAIN_1); 
    write_reg(ADS1220_REG_0, 1, &reg);  

    reg |=REG1_DR_1000 | REG1_MODE_TURBO;
    write_reg(ADS1220_REG_1, 1, &reg);


    reg = 0x00;
    reg |= (  REG2_VREF_EX_DED|ADS1220_REJECT_OFF | REG2_PSW_AUTO);
    write_reg(ADS1220_REG_2, 1, &reg);

    reg = 0;
    read_reg(ADS1220_REG_2, 1, &reg);


    reg = 0x00;
    write_reg(ADS1220_REG_3, 1, &reg);

}

#endif



void write_reg(driver_t *drv,uint8_t startAddress,uint8_t numRegs,uint8_t *pData)
{
    uint32_t i;
    uint8_t data;
    ads1210_t *cfg=(ads1210_t*)drv->cfg;


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
    ads1210_t *cfg=(ads1210_t*)drv->cfg;

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

void ADS1210_start_conv(driver_t *drv)
{
    ads1210_t *cfg=(ads1210_t*)drv->cfg;

    driverex_spi_pend_sem(cfg->spi_io);

    driver_do_low( cfg->cs_io);
    
    mcu_delay(50);
    driverex_spi_send_byte(cfg->spi_io,ADS1220_CMD_SYNC);
   
    driver_do_high( cfg->cs_io);
    
   driverex_spi_post_sem(cfg->spi_io);
    
}

void ADS1210_set_channel(driver_t *drv,uint32_t ch)
{
    uint8_t reg=0;
 

    read_reg(drv,ADS1220_REG_0, 0x01, &reg);
        
    reg = (reg&0x0F) |((ch<<4)|0x80);
   
    write_reg(drv,ADS1220_REG_0,0x01,&reg);




}

void ADS1210_reset_sw(driver_t *drv)
{
    ads1210_t *cfg=(ads1210_t*)drv->cfg;

    driverex_spi_pend_sem(cfg->spi_io);

    driver_do_low(cfg->cs_io);

    driverex_spi_send_byte(cfg->spi_io,ADS1220_CMD_RESET);

    driver_do_high(cfg->cs_io);

    driverex_spi_post_sem(cfg->spi_io);
}


void irq_dataReady(void *ads1210)
{

}


void read_adc(driver_t *drv,uint8_t *err)
{
        ads1210_t *cfg=(ads1210_t*)drv->cfg;

        int32_t data=0;
            *err = 1;
    if(driver_di_read(cfg->drdy_i)==0)
{
    driverex_spi_pend_sem(cfg->spi_io);


    driver_do_low( cfg->cs_io);
    osDelay(2);
    driverex_spi_send_byte(cfg->spi_io,ADS1220_CMD_RDATA);

    data = driver_spi_read_byte(cfg->spi_io);
    data = (data << 8) |driver_spi_read_byte(cfg->spi_io);
    data = (data << 8) |driver_spi_read_byte(cfg->spi_io);

    if (data & 0x00800000)
    {
        data |= 0xff000000;
    }
//    data = data<<8;
//    data = data>>8; // 부호확장
    driver_do_high(cfg->cs_io);
    *err = 0;
    driverex_spi_post_sem(cfg->spi_io);
 
    }
}

int32_t AD1220_read_data(driver_t *drv,int32_t ch,uint8_t *err)
{
    int32_t data=0;
    uint32_t startTime;
        ads1210_t *cfg=(ads1210_t*)drv->cfg;

    
    *err = 1;

    driverex_spi_pend_sem(cfg->spi_io);

    ADS1210_set_channel(drv,ch);
   // osDelay(10);

    //if(!READY_DRDY_ADC())
    {
 
       ADS1210_start_conv(drv);
        
        startTime = osKernelSysTick();

        while(1)
        {
            if((osKernelSysTick()-startTime) > 100)
            {
                break;
            }
            if(driver_di_read(cfg->drdy_i)==0)
            {
                    driverex_spi_pend_sem(cfg->spi_io);
                driver_do_low(cfg->cs_io);
                osDelay(2);
                driverex_spi_send_byte(cfg->spi_io,ADS1220_CMD_RDATA);//이명령어 전송되면
                //drdy 핀 올라감

                data = driverex_spi_read_byte(cfg->spi_io);
                data = (data << 8) |driverex_spi_read_byte(cfg->spi_io);
                data = (data << 8) |driverex_spi_read_byte(cfg->spi_io);

                if (data & 0x00800000)
                {
                    data |= 0xff000000;
                }
            //    data = data<<8;
            //    data = data>>8; // 부호확장
                driver_do_high( cfg->cs_io);
                *err = 0;
                driverex_spi_post_sem(cfg->spi_io);
                break;
            }
        }
    }

    driverex_spi_post_sem(cfg->spi_io);
    return data;
}

  uint8_t reg;
void ads1210_init(driver_t *drv)
{
  
  ADS1210_reset_sw(drv);
  

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


static driver_t ads1220;
ads1210_t ads1210_cfg;

driver_t *ads1220_open(void)
{
    if(ads1220.opened == false)
    {
        ads1220.opened = true;
        ads1220.cfg = &ads1210_cfg;
    }


    return &ads1220;
}
