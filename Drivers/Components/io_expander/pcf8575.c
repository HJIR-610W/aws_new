
#include <string.h>



#include "driver_stm32_i2c.h"
#include "driver_di_def.h"
#include "driver_gpio_def.h"
#include "driver_do_define.h"
#include "pcf8575.h"
#include "system_err.h"
#include "os_user_def.h"

#define PCF8575_0X20 0
#define PCF8575_MAX 1

typedef struct pcf8575_instance_s
{
  bool opened;
  void *i2c_io;
  void *irq_io;
  uint16_t address;
  uint16_t port_data;
  uint16_t dir;  // 핀 방향: 1=INPUT(읽기), 0=OUTPUT(쓰기)
  uint8_t pin[16];  // 핀 번호 매핑 배열: pin[물리핀번호] = 논리핀번호
  void *sem;
} pcf8575_inst_t;

pcf8575_inst_t pcf8575_inst[PCF8575_MAX] = {[PCF8575_0X20]={.opened = 0,.dir = 0x00FF,.address = 0x20}};

int pcf8575_write(int number, uint16_t port_data)
{
  uint8_t data[2];

  OS_PEND_SEM(pcf8575_inst[number].sem,osWaitForever);
  pcf8575_inst[number].port_data = port_data; 

  memcpy(data,&port_data,2);

  stm32_i2c_send_byte(pcf8575_inst[number].i2c_io, pcf8575_inst[number].address, data, 2);

  OS_POST_SEM(pcf8575_inst[number].sem);
  return 0;
}


int pcf8575_read(int number,uint16_t *port_data)
{
  uint8_t data[2];

  OS_PEND_SEM(pcf8575_inst[number].sem, osWaitForever);


  stm32_i2c_recv_byte(pcf8575_inst[number].i2c_io, pcf8575_inst[number].address, data, 2);

  *port_data = ((uint16_t)data[0]) | ((uint16_t)data[1]<<8)&0xFF00;
  OS_POST_SEM(pcf8575_inst[number].sem);
  return 0; 
}




int find_pin(uint8_t *pin,int num)
{
  for(int i = 0 ; i<16;i++)
  {
    if(pin[i]==num)
    {
      return i;
    }
  }

  return -1;
}

int32_t pcf8575_w_pin(int number, int pin, int high)
{
  uint16_t port_data=0;
  uint16_t w_pin=0;
  pcf8575_read(number, &port_data);

  if(high)
  {
    w_pin = 1<<pin;
    port_data |= w_pin;
    pcf8575_write(number,port_data);
  }
  else
  {
    w_pin = ~(1 << pin);
    port_data &= w_pin;
    pcf8575_write(number, port_data);
  }

  return 0;
}

int32_t pcf8575_write_pin(int number, int high)
{
  int pin;
  
  for(int inst = 0; inst < PCF8575_MAX; inst++)
  {
    pin = find_pin(pcf8575_inst[inst].pin, number);
    if(pin != -1)
    {
      if((pcf8575_inst[inst].dir & (1 << pin)) == 0)
      {
        pcf8575_w_pin(inst, pin, high);
        return 0;
      }
    }
  }
  
  return -1;
}


/**
 * @brief pin 읽기
 * @retval 0 low, 1high
 */
int32_t pcf8575_read_pin(int number)
{
  int pin;
  uint16_t port_data = 0;
  
  for(int inst = 0; inst < PCF8575_MAX; inst++)
  {
    pin = find_pin(pcf8575_inst[inst].pin, number);
    if(pin != -1)
    {
      if((pcf8575_inst[inst].dir & (1 << pin)) != 0)
      {
        pcf8575_read(inst, &port_data);
        return ((port_data & (1 << pin)) > 0);
      }
    }
  }
  
  return 0;
}




void pcf8575_init(void)
{
  i2c_open_opt_t i2c_open_opt;
  uint16_t dir = 0;
  int number;
  i2c_open_opt.freq = 400000;
  
  for(int i = 0 ; i< sizeof(pcf8575_inst)/sizeof(pcf8575_inst_t);i++)
  {
    switch(i)
    {
      case PCF8575_0X20:
        if (pcf8575_inst[PCF8575_0X20].opened)
        break;
        
        pcf8575_inst[PCF8575_0X20].opened = true;
        pcf8575_inst[PCF8575_0X20].i2c_io = driver_stm32_i2c_open(STM32_I2C_2, &i2c_open_opt);
        pcf8575_inst[PCF8575_0X20].pin[0] = DI_PCF8575_0;
        pcf8575_inst[PCF8575_0X20].pin[1] = DI_PCF8575_1;
        pcf8575_inst[PCF8575_0X20].pin[2] = DI_PCF8575_2;
        pcf8575_inst[PCF8575_0X20].pin[3] = DI_PCF8575_3;
        pcf8575_inst[PCF8575_0X20].pin[4] = DI_PCF8575_4;
        pcf8575_inst[PCF8575_0X20].pin[5] = DI_PCF8575_5;
        pcf8575_inst[PCF8575_0X20].pin[6] = DI_PCF8575_6;
        pcf8575_inst[PCF8575_0X20].pin[7] = DI_PCF8575_7;

        pcf8575_inst[PCF8575_0X20].pin[8]  = DO_PCF8575_0;
        pcf8575_inst[PCF8575_0X20].pin[9]  = DO_PCF8575_1;
        pcf8575_inst[PCF8575_0X20].pin[10] = DO_PCF8575_2;
        pcf8575_inst[PCF8575_0X20].pin[11] = DO_PCF8575_3;
        pcf8575_inst[PCF8575_0X20].pin[12] = DO_PCF8575_4;
        pcf8575_inst[PCF8575_0X20].pin[13] = DO_PCF8575_5;
        pcf8575_inst[PCF8575_0X20].pin[14] = DO_PCF8575_6;
        pcf8575_inst[PCF8575_0X20].pin[15] = DO_PCF8575_7;

        dir = pcf8575_inst[PCF8575_0X20].dir;

        OS_CREATE_BINARY_SEM(pcf8575_inst[PCF8575_0X20].sem);

        pcf8575_write(PCF8575_0X20,(uint16_t)dir);  // 방향을 설정한다.
        for (int i = 8; i < 16; i++)
        {
          pcf8575_write_pin(pcf8575_inst[PCF8575_0X20].pin[i],  1);  // DO 핀들 전부 High 출력
        }
        break;
    }
  }
}