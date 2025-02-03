
#include "driver_485.h"
#include "driver_uart.h"
#include "driver_digitalOut.h"

typedef struct rs485_cfg_s
{
  driver_t *uart_io;
  driver_t *do_io;
}rs485_cfg_t;

rs485_cfg_t g_rs485_cfg[2];

driver_t g_rs485[2]={{.cfg=&g_rs485_cfg[0]},{.cfg=&g_rs485_cfg[1]}};


driver_t *driver_rs485_open(uint32_t num,void *opt)
{
  if(g_rs485[num].opened == true)
  {
    return &g_rs485[num];
  }

  g_rs485[num].opened = true;


  switch (num)
  {
  case RS485_A:
      g_rs485[num].name = "RS485_A";
      g_rs485_cfg[num].uart_io =  driver_uart_open(UART_4_RS485_A,opt);
      g_rs485_cfg[num].do_io   =  driver_do_open(DO_DIR_RS485_A); 
      driver_do_low(g_rs485_cfg[num].do_io);//수신 모드
      if(g_rs485[num].sem == NULL)
      {
        g_rs485[num].sem = osSemaphoreNew(1, 1, NULL); 
      }
      break;
  case RS485_B:
      g_rs485[num].name = "RS485_B";
      g_rs485_cfg[num].uart_io =  driver_uart_open(UART_5_RS485_B,opt);
      g_rs485_cfg[num].do_io   =  driver_do_open(DO_DIR_RS485_B); 
      driver_do_low(g_rs485_cfg[num].do_io);//수신 모드
     if( g_rs485[num].sem == NULL)
      {
        g_rs485[num].sem = osSemaphoreNew(1, 1, NULL); 
      }
  break;
  }

  return &g_rs485[num];
  
}

int32_t driver_rs485_send(driver_t *drv,uint8_t *pData,uint16_t dataLen)
{
  int32_t cnt=0;
  rs485_cfg_t *cfg;
  cfg = (rs485_cfg_t *)(drv->cfg);

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
  driver_do_high(cfg->do_io);
 cnt =  driver_uart_send(cfg->uart_io,pData,dataLen);
  osDelay(20);
  driver_do_low(cfg->do_io);

  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

  return cnt;
}


int32_t driver_rs485_recv(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms)
{
  rs485_cfg_t *cfg;
  cfg = (rs485_cfg_t *)(drv->cfg);
  int32_t len=0;

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }

  len = driver_uart_recv(cfg->uart_io,pBuff,rLen,timeOutms);

  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
  
  return len;

}


void driver_rs485_set(driver_t *drv,uint8_t cmd,void *option)
{
  rs485_cfg_t *cfg;
    uart_config_t uart_cfg;
    
  cfg = (rs485_cfg_t *)(drv->cfg);

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }

  switch (cmd)
  {
    case 0:

    uart_cfg.baud = (int)option;
    driver_uart_set(cfg->uart_io,UART_SET_BAUDRATE,&uart_cfg);
    break;
  }

  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

}