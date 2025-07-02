
#include "driver_uart.h"

#include <stdbool.h>
#include <stdint.h>

#include "TL16C554.h"
#include "cmsis_os.h"
#include "driver_stm32_cdc.h"
#include "driver_stm32_uart.h"

driver_t *driver_uart_open(int32_t num, void *opt)
{
  driver_t *driver = NULL;

  switch (num)
  {
    case UART_0_D_SUB_0:
      driver = tls16c554_open(TL16C554_UART_1_D_SUB, opt);
      break;
    case UART_1_TTL:
      driver = tls16c554_open(TL16C554_UART_2_TTL_TTL, opt);
      break;
    case UART_2_EXT_A:
      driver = tls16c554_open(TL16C554_UART_3_RS232_A, opt);
      break;
    case UART_3_EXT_B:
      driver = tls16c554_open(TL16C554_UART_4_RS232_B, opt);
      break;
    case UART_6_RS485_A:
      driver = tls16c554_open(TL16C554_UART_5_RS485_A, opt);
      break;
    case UART_7_RS485_B:
      driver = tls16c554_open(TL16C554_UART_6_RS485_B, opt);
      break;
    case UART_4_EXT_C:
      driver = tls16c554_open(TL16C554_UART_7_RS232_C, opt);
      break;
    case UART_5_EXT_D:
      driver = tls16c554_open(TL16C554_UART_8_RS232_D, opt);
      break;
    case UART_8_CDMA:
      driver = stm32_uart_open(STM32_UART_0_CDMA, opt);
      break;
    case UART_9_SDI:
      driver = stm32_uart_open(STM32_UART_1_SDI, opt);
      break;
    case UART_10_CDC:
      driver = stm32_cdc_open(STM32_CDC, opt);
      break;
  }

  return driver;
}

void driver_uart_close(driver_t *drv)
{
  
    if(drv == NULL)
  {
    return  ;
  }
  
  uart_api_t *api = (uart_api_t *)drv->api;

  api->close(drv);
}

int32_t driver_uart_send(driver_t *drv, const uint8_t *pData, uint16_t dataLen)
{
  if(drv == NULL)
  {
    return  -1;
  }
  uart_api_t *api = (uart_api_t *)drv->api;

  return api->send(drv, pData, dataLen);
}

int32_t driver_uart_recv(driver_t *drv, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutMs)
{
    if(drv == NULL)
  {
    return  -1;
  }
  
  
  uart_api_t *api = (uart_api_t *)drv->api;

  return api->recv(drv, pBuff, rLen, timeOutMs);
}


/**
 * @brief 대기 없이 1바이트 수신
 */
int32_t driver_uart_get_charNonBlocking(driver_t *drv, uint8_t *pBuff)
{
    if(drv == NULL)
  {
    return  -1;
  }
  
  uart_api_t *api = (uart_api_t *)drv->api;

  return api->recv(drv, pBuff, 1, 0);
}

/**
 * @brief 1바이트 입력있을때 까지 대기
 */
int32_t driver_uart_get_char(driver_t *drv, uint8_t *pBuff, uint16_t rLen)
{
  
    if(drv == NULL)
  {
    return  -1;
  }
  uart_api_t *api = (uart_api_t *)drv->api;

  return api->recv(drv, pBuff, 1, osWaitForever);
}

void driver_uart_set(driver_t *drv, uart_set_option_t cmd, void *para)
{
    if(drv == NULL)
  {
    return  ;
  }
  
  uart_api_t *api = (uart_api_t *)drv->api;

  api->set(drv, cmd, para);
}

void driver_uart_get(driver_t *drv, uart_get_option_t cmd, void *para)
{
    if(drv == NULL)
  {
    return  ;
  }
  
  uart_api_t *api = (uart_api_t *)drv->api;

  api->get(drv, cmd, para);
}


int32_t driver_uart_recv_crlf(driver_t *drv, char *pBuff, uint16_t bSize, uint32_t tout_ms)
{
  uint8_t data;
  uint16_t cnt = 0;
  uint32_t startTime, startTick, stopTick, elapseTick;
  uint32_t timeout;
  uint32_t len;

  startTime = osKernelGetTickCount();
  timeout = tout_ms;

  do
  {
    startTick = osKernelGetTickCount();
    len = driver_uart_recv(drv, &data, 1, tout_ms);

    if (len)
    {
      pBuff[cnt++] = data;
      
      if((cnt==1)&&((data == '\r') || (data == '\n')))
      {
        cnt = 0;
        continue;
      }
         
         
      if ((data == '\r') || (data == '\n'))
      {
        pBuff[cnt - 1] = 0;
        return (cnt - 1); /* \r 또는 \n 를 제외한 문자열 길이 리턴*/
      }

      if (cnt == bSize)
      {
        return UART_ERR_SIZE;
      }
    }

    stopTick = xTaskGetTickCount();
    elapseTick = stopTick - startTick;

    if ((tout_ms == 0) || ((stopTick - startTime) >= tout_ms))
    {
      break;
    }
    if (tout_ms != osWaitForever)
    {
      timeout = timeout - elapseTick;
    }
  } while (1);

  return UART_ERR_TIMEOUT;
}

void driver_uart_flush_rx(driver_t *drv)
{
  uart_api_t *api = (uart_api_t *)drv->api;

  api->flush_rx(drv);
}

int32_t driver_uart_recv_opt(driver_t *drv, uint8_t *buffer, uint16_t buffer_size,
                              uint32_t timeout1_ms, uint32_t timeout2_ms)
{
  uart_api_t *api = (uart_api_t *)drv->api;

  return api->recv_opt(drv, buffer, buffer_size, timeout1_ms, timeout2_ms);
}

int32_t driver_uart_recv_ll(driver_t *drv, uint8_t *pBuff, uint16_t rLen, uint32_t timeOutMs)
{
  uart_api_t *api = (uart_api_t *)drv->api;

  return api->recv_ll(drv, pBuff, rLen, timeOutMs);
}

int32_t driver_uart_inject(driver_t *drv, const uint8_t *pData, uint16_t dataLen)
{
  uart_api_t *api = (uart_api_t *)drv->api;

  return api->inject(drv, pData, dataLen);
}





