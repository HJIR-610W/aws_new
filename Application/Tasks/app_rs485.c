
#include "driver_485.h"
#include "app_rs485.h"
#include "utile.h"


const uint8_t driver_portList[]={[eAPP_RS485_A]=RS485_A,
                                 [eAPP_RS485_B]=RS485_B};

const char *rs485PortNameList[eAPP_RS485_MAX]={"EX1 RS485 A",
                                               "EX2 RS485 B"};
bool app_rs485Open[eAPP_RS485_MAX];

driver_t *app_rs485[eAPP_RS485_MAX];

void rs485_open(eRS485_PORT_t port)
{
  app_rs485[(int)port] = driver_rs485_open(driver_portList[(int)port]);
  app_rs485Open[(int)port] = true;
}




void rs485_close(eRS485_PORT_t port)
{

  app_rs485Open[(int)port] = 0;
}

void rs485_sends(eRS485_PORT_t port,uint8_t *pData,uint16_t dataLen)
{
  driver_rs485_sends(app_rs485[(int)port],pData,dataLen);
}

uint16_t rs485_recv(eRS485_PORT_t port,uint8_t *pBuff,uint16_t rLen,uint32_t timeOutms)
{
  return driver_rs485_recv(app_rs485[(int)port],pBuff,rLen,timeOutms);
}



uint16_t rs485_get_portList(const char ***list)
{
  *list = rs485PortNameList;
  return _countof(rs485PortNameList);
}

bool rs485_opened(eRS485_PORT_t port)
{
  return   app_rs485Open[(int)port];
}