
#include "Sensors\snow\hj_snow.h"

#include <string.h>

#include "app_rs232.h"
#include "app_rs485.h"
#include "app_sensor.h"
#include "dev_io.h"
#include "driver_485.h"
#include "driver_uart.h"
#include "snow_define.h"
#include "util_memory.h"
#include "config_sensor.h"
#define PROTOCOL_TYPE_t uint8_t
#define P_TYPE_HJ 0
#define P_TYPE_WEATHERPIA_2 1
#define P_TYPE_WEATHERPIA_013456 2
#define WATCHDOG_BUFFER_MAX 12  // 32bit size
#define PASSNUMBER_MAX 5
typedef struct
{
  // Snowfall Config
  uint8_t xModel[3][6];  // bluetooth laser model
                         //		uint8_t  temp_heaton;
  //// value + 40 		uint8_t  temp_heatoff;
  //// value + 40 		uint8_t  heat_automan;
  //// 히터 사용유무(여름에 온도에으한 오동작방지) : 0 : manual , 1 : auto
  uint16_t DeviceID;      //
  uint8_t snow_scantime;  // 0:real , 1:1분 , 5분 , 10분 , 30분, 60분
#define SNOW_HEAT_AUTO 1
#define SNOW_HEAT_MANUAL 0
  uint32_t snow_stddistance;  // 기준 높이 지면으로 부터 장치의 높이

  uint32_t snow_refdistance[3];  // calibration point distance
  uint8_t snow_runflag;          // 0: Not scan , 1: scan

  uint8_t snow_scantemp;      // value + 40 : snow scan 가동 온도 : off온도 :
                              // snow_scantemp - 2도 낮은점
  uint8_t snow_scantempauto;  // 0: manual 1: auto mode,
#define SNOW_SCAN_AUTO 1
#define SNOW_SCAN_MANUAL 0

  uint16_t snow_filterlevel;
  uint8_t snow_nofiltermode;  // 1: run mode(filter) , 0:test mode(non filter)
  // 디바이스별 설정값들..
  uint16_t FactorySet;

  uint8_t Com1PingTime;  // 0:off, 1m~60m
  uint8_t Com2PingTime;  // 0:off, 1m~60m
  uint8_t Com3PingTime;  // 0:off, 1m~60m
  uint8_t Com4PingTime;  // TBD

  ///////////////////////////////
  char Password[PASSNUMBER_MAX];             // DTMF 및 SMS TCP/IP 통신시 비밀번호
  uint8_t WatchDogSec[WATCHDOG_BUFFER_MAX];  // 0 ~ 9:off, 10 ~ 240sec,
  PROTOCOL_TYPE_t protocolType;  // 통신 프로토콜, 0 화진(요청 응답) 1 웨더피아(일방전송)
  uint32_t txPeriodSec;          //  웨더피아 전송 주기 sec
  uint8_t snowScanCnt;
} CONFIG_TypeDef;  // Config		LOGMSG_BUFFER_MAX

typedef struct
{
  uint16_t CurDistance[3];  // channel별 실제 측정거리값
  int16_t CurLevel[3];      // channel별 높이
  int16_t CurSnowLevel;     // 3점의 평균한 높이
  uint32_t LastScanTime;    // 마지막으로 스캔한시간

  uint8_t CurConnStat[3];  // 블루투스 Connection 상태
  uint8_t HeaterStat;
#define SNOW_STAT_HEAT_ON 1
#define SNOW_STAT_HEAT_OFF 0
  uint8_t snow_debugprint;  // 0: not print , 1: print
#define SNOW_STAT_DEBUGPRINT_ON 1
#define SNOW_STAT_DEBUGPRINT_OFF 0
  // eTempSens_t 순서로 배치
  int16_t innerTemp;    // 25 	-> 25`C
  int16_t Humidity;     // 30		-> 30%
  int16_t snow_vitemp;  // virtual temp (temp + 40)
                        // 평상시 filter 적용하고 Test시에만 뺄수있다
  uint8_t ComPingCnt[4];
  uint32_t SysResetTime;  // 시스템 파워 온 시간
  int8_t MstMcuInit;      // TFT Mcu가 초기화한 상태

} SYSTEM_TypeDef;  // System

// SNOWFALL Command
typedef enum
{
  //	CMD_SNOW_START	= 0xC0,
  CMD_SNOW_START = 0x20,  // 00
  CMD_SNOW_READ_STAT,     // 01

  CMD_SNOW_SET_RTC,  // 02
  CMD_SNOW_GET_RTC,  // 03

  CMD_SNOW_SCAN_CTRL,    // 04
  CMD_SNOW_HEATER_CTRL,  // 05

  CMD_SNOW_FOR_LOOP,   // 06
  CMD_SNOW_PING_TEST,  // 07

  CMD_SNOW_CFG_READ,   // 08
  CMD_SNOW_CFG_WRITE,  // 09

  CMD_SNOW_QUECNT_RD,   // 0A
  CMD_SNOW_QUECNT_CLR,  // 0B
  CMD_SNOW_QUEDATA_RD,  // 0C

  CMD_SNOW_SYS_RESET,  // 0D

  CMD_SNOW_FW_DOWN1,
  CMD_SNOW_FW_CHECK1,
  CMD_SNOW_FW_DOWN2,
  CMD_SNOW_FW_CHECK2,

} eCMD_SNOW_t;
uint8_t make_snowFrame(uint8_t *pFrame, uint8_t Cmd, uint8_t DataLen)
{
  uint8_t i;
  uint8_t sum = 0;

  // header
  pFrame[0] = 0x02;  // STX
  pFrame[1] = 0x00;  // ID
  pFrame[2] = Cmd;
  pFrame[3] = DataLen;

  // checksum : ID ~ DATA[N]
  for (i = 1; i <= DataLen + 3; i++) sum += pFrame[i];

  // tail
  pFrame[i++] = sum;
  pFrame[i++] = 0x03;  // ETX

  return i;
}

uint16_t make_hjSnowFrame(uint8_t *out, uint16_t outSize, uint8_t Cmd, uint8_t *data,
                          uint16_t dataLen)
{
  uint16_t i;
  uint8_t sum = 0;
  uint16_t cnt = 0;
  // header
  out[cnt++] = 0x02;  // STX
  out[cnt++] = 0x00;  // ID
  out[cnt++] = Cmd;
  out[cnt++] = dataLen;

  // checksum : ID ~ DATA[N]
  for (i = 0; i <= dataLen; i++)
  {
    out[cnt + i] = data[i];
  }
  cnt += dataLen;

  for (int i = 1; i < cnt; i++)
  {
    sum += out[i];
  }

  // tail
  out[cnt++] = sum;
  out[cnt++] = 0x03;  // ETX

  return cnt;
}

bool check_hjsnow(uint8_t *pdata, uint16_t datalen)
{
  uint8_t len;
  uint8_t sum = 0;

  if (pdata[0] != 0x02)
  {
    return false;
  }

  if (pdata[1] != 0x00)  // id
  {
    return false;
  }

  len = pdata[3];

  sum = make_sum(&pdata[1], len + 3);

  if (sum != pdata[datalen - 2])
  {
    return false;
  }

  return true;
}


//요청  02 00 21 02 00 2C 4F 03 
//응답 02 00 21 2D A2 0A 19 01 6A 0C D2 D3 5E 02 B3 CB A1 E0 00 00 A9 44 6D 38 01 01 01 00 00 00 19 00 2E 00 00 00 00 00 00 00 80 43 6D 38 00 00 00 00 06 D8 03 
int32_t read_hjSnowFall(dev_io_t *dev, uint8_t *err)
{
  uint8_t frame[60];
  static uint16_t len;
  int16_t data = 0;
  uint8_t para[2];
  uint8_t paraCnt = 0;
  uint16_t offset = (uint16_t)(int)&((SYSTEM_TypeDef *)0)->CurSnowLevel;
  uint16_t req_bytes;
  devIoTimeOutopt_t opt;
  *err = 1;

#if 0
    para[paraCnt++] = 0x00;//(uint8_t)&((SYSTEM_TypeDef *)0)->CurSnowLevel;
    para[paraCnt++] = 0x2C;//sizeof(((SYSTEM_TypeDef *)0)->CurSnowLevel);
#else
  para[paraCnt++] = 0x00;
  para[paraCnt++] = sizeof(SYSTEM_TypeDef);
  req_bytes = 37;
#endif
  len = make_hjSnowFrame(frame, sizeof(frame), CMD_SNOW_READ_STAT, para, paraCnt);

  dev_io_write(dev, frame, len, 0);

  opt.waitTimeOutMs = 50;
  len = dev_io_read(dev, frame, sizeof(frame), DEV_IO_CMD_DATA_TIMEOUT, (void *)&opt);


  if(len == 0)
  {
    *err = DRV_ERR_TIMEOUT;
  }

  if (len)
  {
    if (check_hjsnow(frame, len))
    {
      memcpy(&data, &frame[4 + offset], sizeof(data));
      *err = DRV_ERR_NONE;
    }
    else
    {
      *err = DRV_ERR_RECV_DATA;
    }
  }

  return data;
}

typedef struct hjsnow_cfg_s
{
  driver_t *io;
  int32_t channel;
} hjsnow_cfg_t;



driver_t hjsnow_driver;
hjsnow_cfg_t hjsnow_cfg;

int32_t read_hjsnow(driver_t *driver, uint8_t *err);

snow_api_t snow_api = {.read = read_hjsnow};

driver_t *hjsnow_open(int32_t num, void *opt)
{
 int32_t port;
 hjsnow_config_t *hjsnow = opt;


 if (hjsnow_driver.opened)
 {
   return &hjsnow_driver;
 }

  hjsnow_driver.opened = true;

  switch(hjsnow->physical_layer)
  {
    case ePHYSICAL_RS232:
    {
      hjsnow_config_t *rs232_config = opt;
      uart_config_t uart_config;

      uart_config.baud = 19200;
      uart_config.dataLen = 8;
      uart_config.parityIdx = 0;
      uart_config.stop_bit = 1;

      hjsnow_cfg.channel = 1;

      port = uart_num_to_driver_num(rs232_config->port);
      hjsnow_cfg.io = driver_uart_open(port, &uart_config);

      hjsnow_driver.cfg = &hjsnow_cfg;
      hjsnow_driver.api = &snow_api;
    }
    break;
    case ePHYSICAL_RS485:
    {
      uart_config_t uart_config;

      uart_config.baud = 19200;
      uart_config.dataLen = 8;
      uart_config.parityIdx = 0;
      uart_config.stop_bit = 1;
      hjsnow_cfg.io = driver_rs485_open(hjsnow->port, &uart_config);
      hjsnow_cfg.channel = 0;
      hjsnow_driver.cfg = &hjsnow_cfg;
      hjsnow_driver.api = &snow_api;
    }
    break;
  }
  


  return &hjsnow_driver;
}

int32_t read_hjsnow(driver_t *driver, uint8_t *err)
{
  hjsnow_cfg_t *pcfg = driver->cfg;
  int32_t snow = 0;
  dev_io_t dev_io;

  dev_io.driver = pcfg->io;

  if (pcfg->channel == 0)  // 485
  {
    dev_io.io = eRS485_IO;
  }
  else
  {
    dev_io.io = eRS232_IO;
  }

  return read_hjSnowFall(&dev_io, err);
}