
#include "cmsis_os2.h"
#include "config.h"
#include "dev_io.h"
#include "driver_uart.h"
#include "driver_di.h"
#include "driver_do.h"


#define HART_TX_ON()  driver_do_low(g_hart_rts)
#define HART_TX_OFF() driver_do_high(g_hart_rts)
#define IS_HART_CD()    driver_di_read(g_hart_cd)
#define HART_SEND(data,len) driver_uart_send(g_hart_uart,data,len)
#define HART_RECV(buff,buffSize,timeout) driver_uart_recv(g_hart_uart,buff,buffSize,timeout)

#define HART_POWER_ON() driver_do_high(g_power_24)
#define HART_POWER_OFF() driver_do_low(g_power_24)

#define HART_RESET_L() 
#define HART_RESET_H()

driver_t *g_hart_uart;
driver_t *g_hart_rts;
driver_t *g_hart_cd;
driver_t *g_hart_sel;
driver_t *g_power_24;
driver_t *g_hart_reset;



const osThreadAttr_t hardTask_attributes = {
  .name = "hartTask",
  .stack_size = 1024,
  .priority = (osPriority_t) osPriorityNormal1,
};


int32_t hart_send(uint8_t *cmd,uint16_t dataLen)
{

  HART_TX_ON();
  osDelay(2);
  HART_SEND(cmd,dataLen);
  osDelay(2);
  HART_TX_OFF();


  
}
/*
송신
FF FF FF FF FF 02 80 00 00 82

수신
FF FF FF FF FF 06 80 00 18 00 40 FE 62 DC 05 07 05 05 10 00 1C 9C EF 05 05 01 3C 00 00 62 00 62 01 DF
*/
void hardTask(void *arg)
{
    
  uint8_t buff[50];
  uint8_t cmd[]={0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x02 ,0x80 ,0x00 ,0x00 ,0x82};//레코더 
  //uint8_t cmd[]={0x00,0xff,0x00};
  int32_t len;
  


  
  while(1)
  {
      hart_send(cmd,sizeof(cmd));
      len = HART_RECV(buff,sizeof(buff),1000);
      if(len)
      {
        LOG_MEM(buff,len,0,16);
      }
      
  }
}

  
  
void hartTask_init(void)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};
  do_config_t do_config;

  uart_config.baud = 1200;
  uart_config.parityIdx = PARITY_ODD;
  uart_config.stop_bit = 0;
  uart_config.dataLen = UART_DATA_LEN_8;
  
  g_hart_uart = driver_uart_open(UART_5_EXT_D,&uart_config);
  g_hart_cd   = driver_di_open(DI_HART_CD,0);

  do_config.mode = DO_OUT_PP;
  do_config.pullup = DO_NO_PULL;

  g_hart_rts   = driver_do_open(DO_HART_RTS,&do_config);
  g_hart_sel   = driver_do_open(DO_HART_SEL,&do_config);
  g_power_24   = driver_do_open(DO_POWER_24V_ACTIVE_H,&do_config);
  
  driver_do_high(g_power_24);//HART 24V를 공급
    
    
  g_hart_reset = driver_do_open(DO_HART_RESET,&do_config);


  driver_do_high(g_hart_sel);
  driver_do_low(g_hart_reset);//HART 리셋
  osDelay(10);
  driver_do_high(g_hart_reset);

  osThreadNew(hardTask, NULL, &hardTask_attributes);

}