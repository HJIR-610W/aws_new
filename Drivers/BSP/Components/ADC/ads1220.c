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


void os_pend(void *sem,uint32_t timeout_ms)
{
    if(sem)
    {
        osSemaphoreAcquire(sem, timeout_ms);
    }
}

void os_post(void *sem)
{
    if(sem)
    {
        osSemaphoreRelease(sem);
    }
}


void write_reg(ads1210_t *ads1210,uint8_t startAddress,uint8_t numRegs,uint8_t *pData)
{
    uint32_t i;
    uint8_t data;

    os_pend(((driver_spi_t *)ads1210->spi_io)->sem,osWaitForever);

    driver_digitalOut_low( (driver_digitalOut_t *)ads1210->cs_io);

    mcu_delay(50);

    data = ADS1220_CMD_WREG | (((startAddress<<2) & 0x0c) |((numRegs-1)&0x03));
 
    driver_spi_send_byte(ads1210->spi_io,data);

    for (i=0; i< numRegs; i++)
    {
        driver_spi_send_byte(ads1210->spi_io,*pData++);
    }
   
    driver_digitalOut_high( (driver_digitalOut_t *)ads1210->cs_io);

    os_post(((driver_spi_t *)ads1210->spi_io)->sem);
}

void read_reg(ads1210_t *ads1210,uint8_t startAddress,uint8_t numRegs, uint8_t *pBuff)
{
    uint32_t i;
    uint8_t val=0;
    uint8_t data;

    os_pend(((driver_spi_t *)ads1210->spi_io)->sem,osWaitForever);


    driver_digitalOut_low( (driver_digitalOut_t *)ads1210->cs_io);

mcu_delay(50);
    
    data = (ADS1220_CMD_RREG | (((startAddress<<2) & 0x0c) |((numRegs-1)&0x03)));

    driver_spi_send_byte(ads1210->spi_io,data);

    for (i=0; i< numRegs; i++)
    {
        val = driver_spi_read_byte(ads1210->spi_io);
        *pBuff++ = val;
    }
   
    driver_digitalOut_high( (driver_digitalOut_t *)ads1210->cs_io);
    
    os_post(((driver_spi_t *)ads1210->spi_io)->sem);
    

}

void ADS1210_start_conv(ads1210_t *ads1210)
{
    os_pend(((driver_spi_t *)ads1210->spi_io)->sem,osWaitForever);

    driver_digitalOut_low( (driver_digitalOut_t *)ads1210->cs_io);
    
mcu_delay(50);
    driver_spi_send_byte(ads1210->spi_io,ADS1220_CMD_SYNC);
   
    driver_digitalOut_high( (driver_digitalOut_t *)ads1210->cs_io);
    
    os_post(((driver_spi_t *)ads1210->spi_io)->sem);
    
}

void ADS1210_set_channel(ads1210_t *ads1210,uint32_t ch)
{
    uint8_t reg=0;
    
    read_reg(ads1210,ADS1220_REG_0, 0x01, &reg);
        
    reg = (reg&0x0F) |((ch<<4)|0x80);
   
    write_reg(ads1210,ADS1220_REG_0,0x01,&reg);




}

void ADS1210_reset_sw(ads1210_t *ads1210)
{

    os_pend(((driver_spi_t *)ads1210->spi_io)->sem,osWaitForever);
    driver_digitalOut_low( (driver_digitalOut_t *)ads1210->cs_io);


    driver_spi_send_byte(ads1210->spi_io,ADS1220_CMD_RESET);

    driver_digitalOut_high( (driver_digitalOut_t *)ads1210->cs_io);

    os_post(((driver_spi_t *)ads1210->spi_io)->sem);
}


void irq_dataReady(void *ads1210)
{

}


void read_adc(ads1210_t *ads1210,uint8_t *err)
{
        int32_t data=0;
            *err = 1;
    if(driver_digitalIn_read(ads1210->drdy_i)==0)
{
    os_pend(((driver_spi_t *)ads1210->spi_io)->sem,osWaitForever);
    driver_digitalOut_low( (driver_digitalOut_t *)ads1210->cs_io);
    osDelay(2);
    driver_spi_send_byte(ads1210->spi_io,ADS1220_CMD_RDATA);

    data = driver_spi_read_byte(ads1210->spi_io);
    data = (data << 8) |driver_spi_read_byte(ads1210->spi_io);
    data = (data << 8) |driver_spi_read_byte(ads1210->spi_io);

    if (data & 0x00800000)
    {
        data |= 0xff000000;
    }
//    data = data<<8;
//    data = data>>8; // 부호확장
    driver_digitalOut_high( (driver_digitalOut_t *)ads1210->cs_io);
    *err = 0;
        os_post(((driver_spi_t *)ads1210->spi_io)->sem);
 
    }
}

int32_t AD1220_read_data(ads1210_t *ads1210,int32_t ch,uint8_t *err)
{
    int32_t data=0;
    uint32_t startTime;


    
    *err = 1;

    os_pend(ads1210->sem,osWaitForever);

    ADS1210_set_channel(ads1210,ch);
   // osDelay(10);

    //if(!READY_DRDY_ADC())
    {
 
       ADS1210_start_conv(ads1210);
        
        startTime = osKernelSysTick();

        while(1)
        {
            if((osKernelSysTick()-startTime) > 100)
            {
                break;
            }
            if(driver_digitalIn_read(ads1210->drdy_i)==0)
            {
                os_pend(((driver_spi_t *)ads1210->spi_io)->sem,osWaitForever);
                driver_digitalOut_low( (driver_digitalOut_t *)ads1210->cs_io);
                osDelay(2);
                driver_spi_send_byte(ads1210->spi_io,ADS1220_CMD_RDATA);//이명령어 전송되면
                //drdy 핀 올라감

                data = driver_spi_read_byte(ads1210->spi_io);
                data = (data << 8) |driver_spi_read_byte(ads1210->spi_io);
                data = (data << 8) |driver_spi_read_byte(ads1210->spi_io);

                if (data & 0x00800000)
                {
                    data |= 0xff000000;
                }
            //    data = data<<8;
            //    data = data>>8; // 부호확장
                driver_digitalOut_high( (driver_digitalOut_t *)ads1210->cs_io);
                *err = 0;
                    os_post(((driver_spi_t *)ads1210->spi_io)->sem);
                break;
            }
        }
    }

    os_post(ads1210->sem);
    return data;
}

  uint8_t reg;
void ads1210_init(ads1210_t *ads1210)
{
  
  ADS1210_reset_sw(ads1210);
  

    if(ads1210->sem == NULL)
    {
        ads1210->sem = osSemaphoreNew(1, 1, NULL); 
    }

    reg = 0x01;//PGA disable  

    write_reg(ads1210,ADS1220_REG_0, 1, &reg);  

    reg = 0xc0;
    write_reg(ads1210,ADS1220_REG_1, 1, &reg);

    reg = (0x01)<<6;
    write_reg(ads1210,ADS1220_REG_2, 1, &reg);

    reg = 0x00;
    write_reg(ads1210,ADS1220_REG_3, 1, &reg);

}