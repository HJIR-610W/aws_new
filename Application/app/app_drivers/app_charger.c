
#include <stdio.h>

#include "driver_interface.h"
#include "driver_charger.h"
#include "app_charger.h"

driver_t *g_charger = NULL;
charger_data_t charger_data;
uint8_t charger_err = 99;//음수 아직 값이 업데이트 안됨, 0정상, 1 에러


void read_chargerStatus(char *pBuff,uint16_t buffSize)
{
  switch (charger_err)
  {
  case 99:
    snprintf(pBuff,buffSize,"--");
    break;
  case CHARGER_ERR_RECV_TIMEOUT:
  snprintf(pBuff,buffSize,"ERR COM");
  break;
  case CHARGER_ERR_RECV_PACKET:
  snprintf(pBuff,buffSize,"ERR PRO.");
  break;
  default:
  snprintf(pBuff,buffSize,"NORMAL");
    break;
  }
}

bool is_chargerValid(void)
{
  if(charger_err != 99)
  {
    return true;
  }

  return false;
}

void charger_init(uint32_t type)
{
  switch (type)
  {
  case APP_CHARGER_HJ:
  g_charger =   driver_charger_open(CHARGER_HJ_SMART,0);
    break;
  case APP_CHARGER_LS:
  g_charger =   driver_charger_open(CHARGER_LS1024,0);
    break;
  }
}

void update_charger(void)
{
  driver_charger_read(g_charger,&charger_data,&charger_err);
  
}

float read_solarVoltage1(uint8_t *err)
{
  *err = charger_err;
  return charger_data.solar1Volt;
}

float read_solarVoltage2(uint8_t *err)
{
  *err = charger_err;
  return charger_data.solar2Volt;
}
float read_solarCurrrent1(uint8_t *err)
{
  *err = charger_err;
  return charger_data.solar1Current;
}
float read_solarCurrent2(uint8_t *err)
{
  *err = charger_err;
  return charger_data.solar2Current; 
}
float read_batteryVoltage1(uint8_t *err)
{
  *err = charger_err;
  return charger_data.battery1;
}

float read_batteryVoltage2(uint8_t *err)
{
  *err = charger_err;
  return charger_data.battery2;
}

float read_loadCurrent1(uint8_t *err)
{
  *err = charger_err;
  return charger_data.load1Current;
}
float read_loadCurrent2(uint8_t *err)
{
  *err = charger_err;
  return charger_data.load2Current;
}
float read_loadCurrent3(uint8_t *err)
{
  *err = charger_err;
  return charger_data.load3Current;
}