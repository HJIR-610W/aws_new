
#include "modem_sms.h"

#include <string.h>

#include "app_version.h"
#include "boot_version.h"
#include "config_app.h"
#include "util_crc16_ccitt.h"
#include "task_cellular.h"
#include "util_memory.h"
#include "old_aws_sms.h"
#include "system_err.h"
typedef enum
{
  eSMS_CMD_RESET = 1,
  eSMS_CMD_READ_SYSTEM,
  eSMS_CMD_WRITE_CONFIG,
  eSMS_CMD_READ_CONFIG,
  eSMS_CMD_Info,
  eSMS_CMD_RECONNECT_TCP,
  eSMS_CMD_RESET_MODEM,
  eSMS_CMD_READ_MEM,
  eSMS_CMD_READ_SMALLSTREAM_RING,
  eSMS_CMD_VHF_LOOP_TEST,
  eSMA_CMD_VPN_SET,
  eSMA_CMD_AT_DIRECT,
  eSMA_CMD_ENTRY_NOT_ALLOWED,
  eSMA_CMD_ENTRY_ALLOWED
} eSMS_CMD_t;

void SMS_Read_Info(sms_t *sms)
{

  uint8_t release;
  uint8_t add;
  uint8_t fix;
	uint8_t build;
  int32_t len = 0;
	uint32_t bufild_time;
	DATE_TIME_BUF ct;

	
  const char *mfgName;

	get_app_build(&ct);

	bufild_time = time_cvt_timestamp(&ct);
	
  get_app_version(&release, &add, &fix, &build);


  len += snprintf(&sms->msg[len], sizeof(sms->msg) - len, "App/Boot Ver:(%d.%d.%d/", release, add, fix);

	get_boot_version(&release, &add, &fix, &build);

  len += snprintf(&sms->msg[len], sizeof(sms->msg) - len, "%d.%d.%d)", release, add, fix);

  mfgName = get_mfg_name();

  len += snprintf(&sms->msg[len], sizeof(sms->msg) - len, "PCB:%u,MFG:%s,AREA:%u,BUILD:%u,", get_bootPCB(), mfgName, get_appAREA(), bufild_time);
  len += snprintf(&sms->msg[len], sizeof(sms->msg) - len, "ID:%d ", get_config_app()->id);

  _iCellular->send_sms(sms->num, sms->msg);
}

void SMS_Reset(sms_t *sms)
{

  reset_system_delay(5);
  snprintf(&sms->msg[0], sizeof(sms->msg) ,"%s","장비가 리셋됩니다");

  _iCellular->send_sms(sms->num, sms->msg);
}

void SMS_Reconnect_TCP(sms_t *sms)
{
  uint16_t port;
	uint8_t ip[4];

	ip[0] = sms->msg[3];
	ip[1] = sms->msg[4];
	ip[2] = sms->msg[5];
	ip[3] = sms->msg[6];

	memcpy(&port, &sms->msg[7], sizeof(port));

  set_cdma_retarget_ip(ip,port);

  set_cdma_retarget(true);
	
  sprintf(sms->msg, "TCP IP/PORT를 변경합니다.");

  _iCellular->send_sms(sms->num, sms->msg);
}

void SMS_SET_VPN(sms_t *sms)
{
  struct vpn_s
  {
    char id[24];
    char pass[24];
    uint16_t port;
    uint8_t ip[4];
  } vpn_info;

  memcpy(&vpn_info, &sms->msg[3], sizeof(vpn_info));

  _iCellular->set_vpn_config(vpn_info.id, vpn_info.pass, vpn_info.ip, vpn_info.port);

  memset(&vpn_info, 0, sizeof(vpn_info));

  _iCellular->read_vpn_config((char *)&vpn_info, sizeof(vpn_info));

  vpn_info.ip[3] = 0;  // 문자열 null;

  snprintf(sms->msg, sizeof(sms->msg), "%s", (char *)&vpn_info);

  _iCellular->send_sms(sms->num, sms->msg);
}

void SMS_Read_Mem(sms_t *sms)
{
  uint32_t start;
  uint16_t len;

  memcpy(&start, &sms->msg[3], sizeof(start));
  memcpy(&len, &sms->msg[7], sizeof(len));

  memset(sms->msg, 0x00, sizeof(sms->msg));
  if (len <= (sizeof(sms->msg) / 2 - 1))
  {
    Convert_ucharHexAscii((uint8_t *)start, len, sms->msg);
  }
  else
  {
    snprintf(sms->msg, sizeof(sms->msg), "파라미터 오류");
  }

  _iCellular->send_sms(sms->num, sms->msg);
}

void SMS_Read_Config(sms_t *sms)
{
  uint16_t start;
  uint16_t len;
	uint8_t *p_config = (uint8_t *)get_config_app();
  memcpy(&start, &sms->msg[3], sizeof(start));
  memcpy(&len, &sms->msg[5], sizeof(start));

  memset(sms->msg, 0x00, sizeof(sms->msg));
  if (len <= (sizeof(sms->msg) / 2 - 1))
  {
    Convert_ucharHexAscii(((uint8_t *)p_config) + start, len, sms->msg);
  }
  else
  {
    snprintf(sms->msg, sizeof(sms->msg), "파라미터 오류");
  }

  _iCellular->send_sms(sms->num, sms->msg);
}

void SMS_Write_Config(sms_t *sms)
{
  uint16_t start;
  uint16_t len;
	uint8_t *p_config = (uint8_t *)get_config_app();

  memcpy(&start, &sms->msg[3], sizeof(start));
  memcpy(&len, &sms->msg[5], sizeof(start));

  memcpy(((uint8_t *)p_config) + start, &sms->msg[7], len);
  save_config_app();
}

void sms_cmd(sms_t *sms)
{
  uint8_t buff[100];
  int32_t len;
  uint16_t crc16,calCrc16;

  len = Convert_HexAscii2uchar((char*)sms->msg,strlen(sms->msg), buff);

  memcpy(sms->msg,buff,len);

	len = sms->msg[1];
	calCrc16 = Cal_CRC16_xmodem((uint8_t *)&sms->msg[1],len-3);

	memcpy(&crc16,&sms->msg[len-2],sizeof(crc16));

	if(crc16 == calCrc16)
	{
		switch(sms->msg[2])
		{
      case eSMS_CMD_RESET:
        SMS_Reset(sms);
        break;
      case eSMS_CMD_Info:
				SMS_Read_Info(sms);
				break;
      case eSMS_CMD_RECONNECT_TCP:
				SMS_Reconnect_TCP(sms);
				break;
			case eSMA_CMD_VPN_SET:
				SMS_SET_VPN(sms);
			break;
			case eSMS_CMD_READ_MEM:
				SMS_Read_Mem(sms);
				break;
			case eSMS_CMD_READ_CONFIG:
			SMS_Read_Config(sms);
			break;
			case eSMS_CMD_WRITE_CONFIG:
			SMS_Write_Config(sms);
			break;
    }
  }
	else
	{
	//AWS(구) SMS방식 처리
		CheckReadSMS(sms->msg,sms->num);
	}
}