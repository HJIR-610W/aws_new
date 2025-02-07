
#include <string.h>
#include "modem_sms.h"

#include "utile.h"

#include "crc16_ccitt.h"
#include "task_cellular.h"
#include "config.h"
#include "app_version.h"
#include "boot_version.h"
typedef enum
{
    eSMS_CMD_RESET=1,
    eSMS_CMD_READ_SYSTEM,
    eSMS_CMD_WRITE_CONFIG,
    eSMS_CMD_READ_CONFIG,
	eSMS_CMD_Info,
	eSMS_CMD_RECONNECT_TCP,
	eSMS_CMD_RESET_MODEM
}eSMS_CMD_t;



void SMS_Read_Info(sms_t *sms)
{

	uint32_t ver;
	uint8_t release;
	uint8_t add;
	uint8_t fix;
	int32_t len=0;
  uint8_t a,b,c,d;

  get_appVer(&a,&b,&c,&d);


	len    += snprintf(&sms->msg[len],sizeof(sms->msg)-len,"Ver:(%d.%d.%d.%d)", a,b,c,d);



	_iCellular->send_sms(sms->num,sms->msg);
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

			case eSMS_CMD_Info:
				SMS_Read_Info(sms);
				break;
    }
  }
}