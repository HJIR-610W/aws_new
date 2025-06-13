
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
#include "os_user_def.h"
#define PROTOCOL_TYPE_t uint8_t
#define P_TYPE_HJ 0
#define P_TYPE_WEATHERPIA_2 1
#define P_TYPE_WEATHERPIA_013456 2
#define WATCHDOG_BUFFER_MAX 12  // 32bit size
#define PASSNUMBER_MAX 5

typedef struct hjsnow_cfg_s
{
  driver_t *io;
  void *sem;
  int32_t channel;
} hjsnow_cfg_t;



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
  driver_t *p_driver;
  uint8_t frame[60];
  static uint16_t len;
  int16_t data = 0;
  uint8_t para[2];
  uint8_t paraCnt = 0;
  uint16_t offset = (uint16_t)(int)&((SYSTEM_TypeDef *)0)->CurSnowLevel;
  hjsnow_cfg_t *cfg;
  devIoTimeOutopt_t opt;
  *err = 1;


  p_driver = dev->driver;
  cfg = p_driver->cfg;


#if 0
    para[paraCnt++] = 0x00;//(uint8_t)&((SYSTEM_TypeDef *)0)->CurSnowLevel;
    para[paraCnt++] = 0x2C;//sizeof(((SYSTEM_TypeDef *)0)->CurSnowLevel);
#else

  para[paraCnt++] = 0x00;
  para[paraCnt++] = sizeof(SYSTEM_TypeDef);

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
  hjsnow_driver.name = "HJ_SNOW";


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
  OS_CREATE_BINARY_SEM(hjsnow_cfg.sem);

  return &hjsnow_driver;
}

int32_t read_hjsnow(driver_t *driver, uint8_t *err)
{
  hjsnow_cfg_t *pcfg = driver->cfg;
  int32_t snow = 0;
  dev_io_t dev_io;

  OS_PEND_SEM(pcfg->sem,osWaitForever);

  dev_io.driver = pcfg->io;

  if (pcfg->channel == 0)  // 485
  {
    dev_io.io = eRS485_IO;
  }
  else
  {
    dev_io.io = eRS232_IO;
  }

  snow =  read_hjSnowFall(&dev_io, err);

  OS_POST_SEM(pcfg->sem);

  return snow;
}


void hjsnow_read_config(driver_t *driver,uint8_t *p_out,uint16_t out_size,uint8_t *err)
{
  const uint8_t request[] = {0x02, 0x00, 0x28, 0x02, 0x00, 0x32, 0x5C, 0x03};
  uint8_t frame[100];
  static uint16_t len;
  int16_t data = 0;
  uint8_t para[2];
  uint8_t paraCnt = 0;
  uint16_t offset = (uint16_t)(int)&((SYSTEM_TypeDef *)0)->CurSnowLevel;
  devIoTimeOutopt_t opt;
  hjsnow_cfg_t *pcfg = driver->cfg;
  // int32_t snow = 0;
  dev_io_t dev_io;


  OS_PEND_SEM(pcfg->sem,osWaitForever);
  dev_io.driver = pcfg->io;

  if (pcfg->channel == 0)  // 485
  {
    dev_io.io = eRS485_IO;
  }
  else
  {
    dev_io.io = eRS232_IO;
  }

  *err = 1;

  dev_io_flush(&dev_io);
  dev_io_write(&dev_io, (uint8_t*)request, sizeof(request), 0);

  opt.waitTimeOutMs = 50;
  len = dev_io_read(&dev_io, frame, sizeof(frame), DEV_IO_CMD_DATA_TIMEOUT, (void *)&opt);

  if (len == 0)
  {
    *err = DRV_ERR_TIMEOUT;
  }

  if (len)
  {
    if (check_hjsnow(frame, len))
    {
      memcpy(p_out, &frame[10],out_size);
       *err = DRV_ERR_NONE;
    }
    else
    {
      *err = DRV_ERR_RECV_DATA;
      LOG_MEM(frame,len,0,16);
    }
  }

  OS_POST_SEM(pcfg->sem);
}

void hjsnow_read_system(driver_t *driver, uint8_t *p_out, uint16_t out_size, uint8_t *err)
{
  const uint8_t request[] = {0x02, 0x00, 0x21, 0x02, 0x00, 0x32, 0x55, 0x03};
  hjsnow_cfg_t *pcfg = driver->cfg;
  uint8_t frame[100];
  static uint16_t len;
  int16_t data = 0;
  uint8_t para[2];
  uint8_t paraCnt = 0;
  devIoTimeOutopt_t opt;
  dev_io_t dev_io;

  OS_PEND_SEM(pcfg->sem, osWaitForever);
  
  dev_io.driver = pcfg->io;

  if (pcfg->channel == 0)  // 485
  {
    dev_io.io = eRS485_IO;
  }
  else
  {
    dev_io.io = eRS232_IO;
  }

  *err = 1;


  dev_io_flush(&dev_io);
  dev_io_write(&dev_io, (uint8_t *)request, sizeof(request), 0);

  opt.waitTimeOutMs = 50;
  len = dev_io_read(&dev_io, frame, sizeof(frame), DEV_IO_CMD_DATA_TIMEOUT, (void *)&opt);

  if (len == 0)
  {
    *err = DRV_ERR_TIMEOUT;
  }

  if (len)
  {
    if (check_hjsnow(frame, len))
    {
      memcpy(p_out, &frame[4], out_size);
      *err = DRV_ERR_NONE;
    }
    else
    {
      *err = DRV_ERR_RECV_DATA;
      LOG_MEM(frame, len, 0, 16);
    }
  }

  OS_POST_SEM(pcfg->sem);
}


void hjsnow_run_zero(driver_t *driver,uint8_t *err)
{
  const uint8_t request[] = {0x02, 0x00, 0x24, 0x02, 0x04, 0x01, 0x2B, 0x03};
  hjsnow_cfg_t *pcfg = driver->cfg;
  dev_io_t dev_io;
  uint16_t len;
    uint8_t frame[50];
    devIoTimeOutopt_t opt;
    dev_io.driver = pcfg->io;

    if (pcfg->channel == 0)  // 485
    {
      dev_io.io = eRS485_IO;
    }
  else
  {
    dev_io.io = eRS232_IO;
  }
  opt.waitTimeOutMs = 50;
  dev_io_write(&dev_io, (uint8_t*)request, sizeof(request), 0);
  len = dev_io_read(&dev_io, frame, sizeof(frame), DEV_IO_CMD_DATA_TIMEOUT, (void *)&opt);

  if (len == 0)
  {
    *err = DRV_ERR_TIMEOUT;
  }

  if (len)
  {
    if (check_hjsnow(frame, len))
    {
      *err = DRV_ERR_NONE;
    }
    else
    {
      *err = DRV_ERR_RECV_DATA;
    }
  }
}

void hjsnow_ctrl(driver_t *driver, eHJSNOW_CTRL_t ctrl, void *w_opt,void *r_opt,uint8_t *err)
{
  switch (ctrl)
  {
    case eHJSNOW_RUN_ZERO:
      hjsnow_run_zero(driver,err);
      break;
    case eHJSNOW_SET_DISTANCE:
      break;
    case eHJSNOW_GET_CONFIG:
      hjsnow_read_config(driver, (uint8_t *)&((hjsnow_read_config_t*)r_opt)->config, sizeof(((hjsnow_read_config_t *)r_opt)->config),
                         err);
      break;
    case eHJSNOW_GET_SYSTEM:
      hjsnow_read_system(driver, (uint8_t *)&((hjsnow_read_system_t *)r_opt)->system,
                         sizeof(((hjsnow_read_system_t *)r_opt)->system), err);
      break;
  }
}

driver_t * hjsnow_opened(void)
{
  if(hjsnow_driver.opened)
  {
    return &hjsnow_driver;
   
  }

  return NULL;
  
}