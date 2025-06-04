#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config_app.h"
#include "task_cellular.h"
#include "util_time.h"

typedef int ts_t;


uint16_t String2Ushort(char *szStr, uint32_t nLen)
{
  char szTemp[10];
  uint16_t usTmp;

  strncpy(szTemp, szStr, nLen);
  szTemp[nLen] = 0x00;
  usTmp = atoi(szTemp);

  return usTmp;
}


time_t ConvertDate_TMX400(char * szDate)  // String을 날짜 시간 데이터로
{
  time_t tmRet;
  char szTemp[5];
  int nYear;
  int nMonth;
  int nDay;
  int nHour;
  int nMinute;
  int nSecond;

  if (strlen(szDate) != 14)
    return (time_t)-1;

  strncpy(szTemp, szDate, 2);
  nYear = atoi(szTemp) + 2000;

  strncpy(szTemp, szDate + 3, 2);
  nMonth = atoi(szTemp);

  strncpy(szTemp, szDate + 5, 2);
  nDay = atoi(szTemp);

  strncpy(szTemp, szDate + 8, 2);
  nHour = atoi(szTemp);

  strncpy(szTemp, szDate + 11, 2);
  nMinute = atoi(szTemp);

  strncpy(szTemp, szDate + 14, 2);
  nSecond = atoi(szTemp);

  tmRet = SetTime(nYear, nMonth, nDay, nHour, nMinute, nSecond);
  return tmRet;
}
bool CheckReadSMS(char *sms_msg,char *sms_number)
{
  char szPassword[6];
  uint8_t cCmd;
  char szPort[16];
  char szIndex[12];
  char szTemp[32];
  uint8_t cIp[4];
  int nIndex;
  time_t tmSMSSend;
  time_t tmCurrent;
  ts_t tmsSpan;
  uint16_t port;
  int nRet;
  char password[10];
  uint8_t ip[4];


  // ========================================================================= //
  //  1  2  3      4    5     6          7        8      9
  //  1. Start Code: HR
  //  2. Frame Size: 43
  //  3. PASSWORD  : 0123
  //  4. Command   : 13

  //  4. IP Address: 192.168.123.231
  //  5. TCP Port  : 09000
  //  6. TCP Connection 유지시간 : 00020 초
  //  6. 일련  번호: 00001
  //  7. 전송일자  : 20150406
  //  8. 전일시간  : 153645
  //  9. BCC       : b

  //  HR36012313192.168.123.231090000000000001201504061827003
  //    ^ ^   ^ ^              ^    ^    ^    ^             ^
  // ========================================================================= //

  if (strncmp(sms_msg, "HR", 2) != 0)
  {
    return false;  // START CODE 맞지않음
  }

  snprintf(password,sizeof(password),"%04d",get_config_app()->password);

  if (strncmp(&sms_msg[4], password, 4) != 0)
  {
    return false;
  }

  strncpy(szTemp, sms_msg + 40, 14);
  szTemp[14] = 0x00;
  tmSMSSend = ConvertDate_TMX400(szTemp);
  tmCurrent = SetTime(Date_Time.Year, Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min,
                      Date_Time.Sec);
  tmsSpan = tmCurrent - tmSMSSend;
  nRet = GetTotalSeconds(tmsSpan);



  cCmd = String2Ushort(sms_msg + 8, 2);
  switch (cCmd)
  {
    case 12:  // TimeSync 명령 수행하면 단말기 Reset후 System시간을 단말기와 맞춘다
      //시간동기화
      break;
    case 13:  // 임의(SMS 전송)의 IP와 Port로 접속 일정시간동안

      ip[0] = (uint8_t)String2Ushort(sms_msg + 10, 3);
      ip[1] = (uint8_t)String2Ushort(sms_msg + 14, 3);
      ip[2] = (uint8_t)String2Ushort(sms_msg + 18, 3);
      ip[3] = (uint8_t)String2Ushort(sms_msg + 22, 3);

      memset(szPort, 0x00, sizeof(szPort));
      memcpy(szPort, sms_msg + 25, 5);
      port = (uint16_t)atoi(szPort);

      set_cdma_retarget_ip(ip,port);
      set_cdma_retarget(true);

      break;
    case 14:  // 접속할 Server IP & Port를 바꾼다
      ip[0] = (uint8_t)String2Ushort(sms_msg + 10, 3);
      ip[1] = (uint8_t)String2Ushort(sms_msg + 14, 3);
      ip[2] = (uint8_t)String2Ushort(sms_msg + 18, 3);
      ip[3] = (uint8_t)String2Ushort(sms_msg + 22, 3);

      port = String2Ushort(sms_msg + 25, 5);

      set_config_app_cdma_ip(ip);
      set_config_app_cdma_port(port);

      break;
  }  // SMS 수신 내용인지를 체크한다.
  return true;
}