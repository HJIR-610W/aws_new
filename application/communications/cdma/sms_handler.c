
#include "sms_handler.h"

#include <string.h>

#include "app_version.h"
#include "boot_version.h"
#include "config_app.h"
#include "util_crc16_ccitt.h"
#include "util_memory.h"
#include "old_aws_sms.h"
#include "system_err.h"
#include "app_logging.h"
#include "task_logging.h"
#include "task_cellular.h"
#include "cellular_api.h"

#include "sms_core.h"




static void read_sms_info(sms_t *sms)
{
  uint8_t release;
  uint8_t add;
  uint8_t fix;
  uint8_t build;
  int32_t len = 0;
  uint32_t build_time;
  const char *mfg_name;
  DATE_TIME_BUF ct;

  get_app_build(&ct);

  build_time = time_cvt_timestamp(&ct);

  get_app_version(&release, &add, &fix, &build);

  len += snprintf(&sms->message[len], sizeof(sms->message) - len, "A/B(%d.%d.%d/", release, add, fix);

  get_boot_version(&release, &add, &fix, &build);

  len += snprintf(&sms->message[len], sizeof(sms->message) - len, "%d.%d.%d)", release, add, fix);

  mfg_name = get_alias_name();

  len += snprintf(&sms->message[len], sizeof(sms->message) - len, "PCB:%u,MFG:%s,AREA:%u,BUILD:%u,", get_boot_pcb_version(), mfg_name, get_app_area_code(), build_time);
  len += snprintf(&sms->message[len], sizeof(sms->message) - len, "ID:%d ", get_config_app()->device_id);

  cellular_send_sms(sms->number, sms->message);
}

static void reset_sms(sms_t *sms)
{
  int reset_delay_seconds = 15;
  
  log_printf(L_INFO, "sms reset");
  reset_system_delay(reset_delay_seconds);
  snprintf(&sms->message[0], sizeof(sms->message) ,"The device will reset in %d seconds.", reset_delay_seconds );

  cellular_send_sms(sms->number, sms->message);
}

extern void set_cdma_retarget_ip(uint8_t ip[4],uint16_t port);
extern void set_cdma_retarget(bool target);

static void reconnect_tcp_sms(sms_t *sms)
{
  uint8_t ip[4];
  uint16_t port;

  ip[0] = sms->message[3];
  ip[1] = sms->message[4];
  ip[2] = sms->message[5];
  ip[3] = sms->message[6];

  memcpy(&port, &sms->message[7], sizeof(port));

  set_cdma_retarget_ip(ip, port);

  set_cdma_retarget(true);

  sprintf(sms->message, "Reconnecting to %u.%u.%u.%u:%u.", ip[0], ip[1], ip[2], ip[3], port);

  cellular_send_sms(sms->number, sms->message);
}

static void set_vpn_sms(sms_t *sms)
{
  struct vpn_s
  {
    char id[24];
    char pass[24];
    uint16_t port;
    uint8_t ip[4];
  } vpn_info;

  memcpy(&vpn_info, &sms->message[3], sizeof(vpn_info));

cellular_set_vpn_config(vpn_info.id, vpn_info.pass, vpn_info.ip, vpn_info.port);

  memset(&vpn_info, 0, sizeof(vpn_info));

cellular_read_vpn_config((char *)&vpn_info, sizeof(vpn_info));

  vpn_info.ip[3] = 0;  

  snprintf(sms->message, sizeof(sms->message), "%s", (char *)&vpn_info);

  cellular_send_sms(sms->number, sms->message);
}

static void read_mem_sms(sms_t *sms)
{
  uint32_t start;
  uint16_t len;

  memcpy(&start, &sms->message[3], sizeof(start));
  memcpy(&len, &sms->message[7], sizeof(len));

  memset(sms->message, 0x00, sizeof(sms->message));
  if (len <= (sizeof(sms->message) / 2 - 1))
  {
    Convert_ucharHexAscii((uint8_t *)start, len, sms->message);
  }
  else
  {
    snprintf(sms->message, sizeof(sms->message), "Read length exceeded.");
  }

  cellular_send_sms(sms->number, sms->message);
}

static void read_config_sms(sms_t *sms)
{
  uint8_t *p_config = (uint8_t *)get_config_app();
  uint16_t start;
  uint16_t len;

  memcpy(&start, &sms->message[3], sizeof(start));
  memcpy(&len, &sms->message[5], sizeof(start));

  memset(sms->message, 0x00, sizeof(sms->message));
  if (len <= (sizeof(sms->message) / 2 - 1))
  {
    Convert_ucharHexAscii(((uint8_t *)p_config) + start, len, sms->message);
  }
  else
  {
    snprintf(sms->message, sizeof(sms->message), "Read length exceeded.");
  }

  cellular_send_sms(sms->number, sms->message);
}

static void write_config_sms(sms_t *sms)
{
  uint8_t *p_config = (uint8_t *)get_config_app();
  uint16_t start;
  uint16_t len;

  memcpy(&start, &sms->message[3], sizeof(start));
  memcpy(&len, &sms->message[5], sizeof(start));

  memcpy(((uint8_t *)p_config) + start, &sms->message[7], len);
  save_config_app();
}

void sms_cmd(sms_t *sms)
{
  uint8_t buff[100];
  uint16_t crc16;
  uint16_t cal_crc16;
  int32_t len;

  len = Convert_HexAscii2uchar((char*)sms->message, strlen(sms->message), buff);

  memcpy(sms->message, buff, len);

  len = sms->message[1];
  cal_crc16 = Cal_CRC16_xmodem((uint8_t *)&sms->message[1], len - 3);

  memcpy(&crc16, &sms->message[len - 2], sizeof(crc16));

  if(crc16 == cal_crc16)
  {
    switch(sms->message[2])
    {
      case eSMS_CMD_RESET:
        reset_sms(sms);
        break;
      case eSMS_CMD_Info:
        read_sms_info(sms);
        break;
      case eSMS_CMD_RECONNECT_TCP:
        reconnect_tcp_sms(sms);
        break;
      case eSMA_CMD_VPN_SET:
        set_vpn_sms(sms);
        break;
      case eSMS_CMD_READ_MEM:
        read_mem_sms(sms);
        break;
      case eSMS_CMD_READ_CONFIG:
        read_config_sms(sms);
        break;
      case eSMS_CMD_WRITE_CONFIG:
        write_config_sms(sms);
        break;
    }
  }
  else
  {
    CheckReadSMS(sms->message, sms->number);
  }
}