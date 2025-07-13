
#include "cmsis_os.h"
#include "driver_sdi.h"
#include "driver_uart.h"
#include "bsp_do.h"

typedef struct sdi_cfg_s
{
  driver_t *uart_io;
  int dir_do_num;
}sdi_cfg_t;

sdi_cfg_t g_sdi_cfg[1];
driver_t g_sdi_list[1]={{.cfg=&g_sdi_cfg[0]}};


driver_t *driver_sdi_open(uint32_t num,void *opt)
{
  if(g_sdi_list[num].opened == true)
  {
    return &g_sdi_list[num];
  }

  g_sdi_list[num].opened = true;


  switch (num)
  {
  case SDI_0:
      g_sdi_cfg[num].uart_io =  driver_uart_open(UART_9_SDI,opt);
      g_sdi_cfg[num].dir_do_num   =  BSP_DO_DIR_RS485_A ;
      bsp_do_low(g_sdi_cfg[num].dir_do_num);//수신 모드

    if( g_sdi_list[num].sem == NULL)
    {
       g_sdi_list[num].sem = osSemaphoreNew(1, 1, NULL); 
    }
    break;

  default:
    break;
  }

  return &g_sdi_list[num];
  
}

void driver_sdi_sends(driver_t *drv,uint8_t *pData,uint16_t dataLen)
{
  sdi_cfg_t *cfg;
  cfg = (sdi_cfg_t *)(drv->cfg);

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
  bsp_do_high(cfg->dir_do_num);
  osDelay(1);
  driver_uart_send(cfg->uart_io,pData,dataLen);
  bsp_do_low(cfg->dir_do_num);
  osDelay(1);
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
}


uint16_t driver_sdi_recv(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms)
{
  sdi_cfg_t *cfg;
  cfg = (sdi_cfg_t *)(drv->cfg);
uint16_t len;

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }

  len = driver_uart_recv(cfg->uart_io,pBuff,rLen, timeOutms);

  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
  
  return len;

}