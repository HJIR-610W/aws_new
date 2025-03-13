

#include "pcb_define.h"

#include "driver_485.h"
#include "driver_uart.h"
#include "driver_do.h"

typedef struct rs485_cfg_s
{
  driver_t *uart_io;
  driver_t *do_io;
}rs485_cfg_t;

rs485_cfg_t g_rs485_cfg[2];

driver_t g_rs485[2];

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
      g_rs485[num].cfg = &g_rs485_cfg[num];
      g_rs485_cfg[num].uart_io =  driver_uart_open(UART_6_RS485_A,opt);
      g_rs485_cfg[num].do_io   =  driver_do_open(DO_DIR_RS485_A,0); 

      driver_do_low(g_rs485_cfg[num].do_io);//수신 모드
#if FREE_RTOS_USE
      if(g_rs485[num].sem == NULL)
      {
        g_rs485[num].sem = osSemaphoreNew(1, 1, NULL); 
      }
#endif
      break;
  case RS485_B:
      g_rs485[num].name = "RS485_B";
      g_rs485[num].cfg = &g_rs485_cfg[num];
      g_rs485_cfg[num].uart_io =  driver_uart_open(UART_7_RS485_B,opt);
      g_rs485_cfg[num].do_io   =  driver_do_open(DO_DIR_RS485_B,0); 
      driver_do_low(g_rs485_cfg[num].do_io);//수신 모드
#if FREE_RTOS_USE
     if(g_rs485[num].sem == NULL)
      {
        g_rs485[num].sem = osSemaphoreNew(1, 1, NULL); 
      }
#endif
  break;
  }

  return &g_rs485[num];
  
}

/**
 * @brief RS485 데이터 전송
 * @param drv
 * @param pData
 * @param dataLen
 * @retval -1 전송 오류, 0>= 전송된 데이터 수
 */
int32_t driver_rs485_send(driver_t *drv,uint8_t *pData,uint16_t dataLen)
{
  int32_t cnt=0;
  rs485_cfg_t *cfg;

  cfg = (rs485_cfg_t *)(drv->cfg);

#if FREE_RTOS_USE
  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
#endif

  driver_do_high(cfg->do_io);//출력으로 설정
    osDelay(1);
  cnt =  driver_uart_send(cfg->uart_io,pData,dataLen);
    osDelay(1);
  driver_do_low(cfg->do_io);//입력으로 설정

#if FREE_RTOS_USE
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
#endif

  return cnt;
}

/**
 * @brief RS485 수신 
 * @param timeOutms 타임아웃시간동안 데이터 수신
 * @retval -1에러,0 수신없음, 1이상 수신된 데이터 길이
 */
int32_t driver_rs485_recv(driver_t *drv,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms)
{
  rs485_cfg_t *cfg;
  cfg = (rs485_cfg_t *)(drv->cfg);
  int32_t len=0;

#if FREE_RTOS_USE
  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
#endif

  len = driver_uart_recv(cfg->uart_io,pBuff,rLen,timeOutms);

#if FREE_RTOS_USE
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
#endif

  return len;
}

int32_t driver_rs485_recv_opt(driver_t *drv,
                              uint8_t *pBuff,
                              uint16_t rLen,
                              eUART_RECV_OPT_t cmd,
                              void *opt) 
{
  rs485_cfg_t *cfg;
  cfg = (rs485_cfg_t *)(drv->cfg);
  int32_t len=0;

#if FREE_RTOS_USE
  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
#endif

  len = driver_uart_recv_opt(cfg->uart_io,pBuff,rLen,cmd,opt);

#if FREE_RTOS_USE
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
#endif

  return len;
}
void driver_rs485_set(driver_t *drv,uart_set_option_t cmd,void *option)
{
  rs485_cfg_t *cfg;
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  cfg = drv->cfg;

#if FREE_RTOS_USE
  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
#endif

  switch (cmd)
  {
    case UART_SET_BAUDRATE:
    driver_uart_set(cfg->uart_io,cmd,option);
    break;
  }

#if FREE_RTOS_USE
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
#endif
}

void driver_rs485_close(driver_t *drv)
{

}