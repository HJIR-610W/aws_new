
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "cmsis_os2.h"
#include "util_memory.h"
#include "modem_tx700.h"
#include "at_cmd.h"

#include "util_memory.h"

#include "bsp.h"
#include "drv_power.h"




extern void EwFree( void* aMemory );
extern void* EwAlloc( int aSize );
extern void modem_sends(const char *pData);
extern void modem_send(const char *pData,uint16_t dataLen);
extern uint32_t wait_asyncResp(uint32_t *cmd,char *pBuff,uint16_t buffSize);
extern uint32_t wait_tcpResp(uint32_t *cmd,char *pBuff,uint16_t buffSize);
extern uint32_t recv_tcp(uint8_t *pBuff,uint16_t buffSize,uint16_t *pLen,uint32_t timeOutMs);
extern bool is_modemBoot(void);
extern uint32_t is_serverErr(void);



#define CNT_OF(arr)   (sizeof(arr)/sizeof(arr[0]))

#define SPK_LEVEL_0 0 
#define SPK_LEVEL_1 1 
#define SPK_LEVEL_2 2 
#define SPK_LEVEL_3 3 
#define SPK_LEVEL_4 4 
#define SPK_LEVEL_5 5 
#define SPK_LEVEL_6 6 
#define SPK_LEVEL_7 7 
#define SPK_LEVEL_8 8 

#define MIC_LEVEL_0 0 
#define MIC_LEVEL_1 1 
#define MIC_LEVEL_2 2 
#define MIC_LEVEL_3 3 
#define MIC_LEVEL_4 4 
#define MIC_LEVEL_5 5 
#define MIC_LEVEL_6 6 
#define MIC_LEVEL_7 7 
#define MIC_LEVEL_8 8


driver_t g_tx700_drv;

    /*

    서버가 실행중이 아닌상태에서 AT$$TCP_SCOP=0이것을 하면
    서버가 실행될때까지 계속 접속시도하는듯함
    수십초 지나서 서버가 실행이되면 그때 연결됨
    모뎀이 서버접속의 타임아웃을 얼마로 했는지는 알수 없음

    전환 연결됨
    $$TELL: 751, VOICE : CONNECT USER

    전화가 연결된 상태에서 상대가 끊을때
    NO CARRIER
    $$TELL: 754, VOICE : NETWORK RELEASE

    전화를 받지 않은 상태에서 상대가 전화를 끊을때
    $$TELL: 754, VOICE : NETWORK RELEASE
    */

    /// @brief at 명령어와 응답 목록
    const atCmd_t cmd_tx700[] = {
        {AT_ASYNC_RECV_TCP_DISCONNECTED, "$$TELL: 605"},  //$$TELL: 605, TCP : TCP ???? ????
        {AT_ASYNC_RECV_SMS, "+CMTI"},            //+CMTI: "ME",0
        {AT_ASYNC_RECV_RING, "+CLIP"},           //+CLIP: "01053730725",128,"",0,,0
        {AT_ASYNC_RECV_REBOOT, "$$TELL:34"},              //$$TELL:34,Modem Boot Up
        {AT_ASYNC_RECV_TCP_DATA, "$$BinRecv"},            //$$BinRecv
        {AT_ASYNC_RESP_VOICE_END,
         "$$TELL: 754, VOICE : NETWORK RELEASE"},     //$$TELL: 754, VOICE : NETWORK RELEASE
        {AT_ASYNC_RECV_DTMF, "$DTMF:"},               //$DTMF: 4
        {AT_TCP_WRITE_IP_RESP, "$$TCP_ADDR:"},        //$$TCP_ADDR: 0
        {AT_TCP_OPEN_PPP, "AT$$TCP_PPPOP\r\n"},       // AT$$TCP_PPPOP
        {AT_TCP_CLOSE_PPP, "AT$$TCP_PPPCL\r\n"},      // AT$$TCP_PPPCL
        {AT_TCP_OPEN_SOCKET, "AT$$TCP_SCOP=0\r\n"},   // AT$$TCP_SCOP=0
        {AT_TCP_CLOSE_SOCKET, "AT$$TCP_SCCL=0\r\n"},  // AT$$TCP_SCCL=0
        {AT_ASYNC_OPEN_VOICE, "ATA\r\n"},             // ATA
        {AT_ASYNC_OPEN_VOICE_RESP,
         "$$TELL: 751, VOICE : CONNECT USER"},  //$$TELL: 751, VOICE : CONNECT USER
        {AT_ASYNC_GET_RSSI, "AT+CSQ\r\n"},      // AT+CSQ
        {AT_SYNC_GET_RSSI_RESP, "+CSQ"},       //+CSQ: 28,99<
        {AT_SYNC_SMS_READ_RESP_OK,
         "+CMGR:"},  //+CMGR: "REC UNREAD","01053730725",,"25/05/25,10:07:34+36"<CR><LF>
                     //+CMGS: 59
        {AT_ASYNC_SMS_READ_RESP_ERR, "+CMS ERROR"},
        {AT_SMS_SEND_RESP, "$$TELL:45"},              //$$TELL:45,?????? ???? ???? ????<CR><LF>
        {AT_TCP_SEND_DATA_RESP, "$$TCP_SENDDATA:"},   //$$TCP_SENDDATA:1
        {AT_TCP_OPEN_SOCKET_RESP_OK, "$$TELL: 603"},  //$$TELL: 603, TCP : TCP ???? ????<CR><LF>
        {AT_TCP_OPEN_SOCKET_RESP_FAIL,
         "$$TELL: 602"},                           //$$TELL: 602, TCP : TCP ???? ???? ??<CR><LF>
        {AT_TCP_OPEN_SOCKET_RESP, "$$TELL: 603"},  //
        {AT_TCP_READ_NUM_RESP, "+CNUM:"},          //+CNUM: ,"01220891572",129
        {AT_ASYNC_OFF_VOICE, "$$TELL: 756"},       //$$TELL: 756, VOICE : USER RELEASE<
        {AT_TCP_CONNECT_VPN_RESP, " "},
        {AT_TCP_OPEN_PPP_RESP, "$$TELL: 600"},      //$$TELL: 600, TCP : PPP ???? ????<CR><LF>
        {AT_TCP_CLOSE_PPP_RESP, "$$TELL: 601"},     //$$TELL: 601, TCP : PPP ???? ????<CR><LF>
        {AT_TCP_CLOSE_SOCKET_RESP, "$$TELL: 605"},  //$$TELL: 605, TCP : TCP ???? ????<CR><LF>
        {AT_RESET_SW_RESP, "OK"},                   // OK
        {AT_ASYNC_DIAL_RESP, " "},
        {AT_ASYNC_DIAL_OFF, "AT+CHUP\r\n"},  // AT+CHUP
        {AT_ASYNC_CONFIG_READ_RESP, " "},
        {AT_TCP_NETWORK_SERVICE, " "},
        {AT_READ_NUM, "AT+CNUM\r\n"},
        {AT_RESET_SW, "AT$$RESET\r\n"}};



uint32_t tx700_get_count(void)
{
    return _countof(cmd_tx700);
}

const char *get_modem_string_tx700(eAT_COMMAND_t cmd)
{
  for (int i = 0; i < AT_MAX; i++)
  {
    if (cmd_tx700[i].cmd == cmd)
    {
      return cmd_tx700[i].cmdStr;
    }
  }

  return NULL;
}


static void tx700_modem_sends(const char *data)
{
    modem_sends(data);
}

static void tx700_modem_send(const char *data,uint16_t dataLen)
{
  modem_send(data,dataLen);
}

/*
async task에서 만 수신되도록한 응답을 확인
*/
static M_RET_t tx700_asyncRecv_response(char *pBuff,uint16_t buffSize)
{
    uint32_t cmd;
    M_RET_t ret = RET_FAIL;

    if(wait_asyncResp(&cmd,pBuff,buffSize) == 0)
    {
        ret = RET_OK;
    }

    return ret;
}

/*
tcp task에서만 수신되도록한 응답을 확인
*/
static M_RET_t tx700_tcpRecv_response(char *pBuff,uint16_t buffSize)
{
    M_RET_t ret = RET_FAIL;
    uint32_t cmd;

    if(wait_tcpResp(&cmd,pBuff,buffSize) == 0)
    {
      
        ret = RET_OK;
    }

    return ret;
}


static M_RET_t tx700_check_asyncResp(const char *const*pAckList,uint32_t ackListCnt,uint32_t *foundedIndex,char *pBuff,uint32_t buffSize,uint32_t timeout_ms)
{

    uint32_t startTime;
    uint32_t i;
    M_RET_t ret = RET_FAIL;

    startTime = osKernelGetTickCount();

    do{
        ret = tx700_asyncRecv_response(pBuff,buffSize);

        if(ret == RET_OK)
        {
            for(i = 0 ; i< ackListCnt;i++)
            {
                if(strncmp((char *)pBuff,(char *)pAckList[i],strlen((char *)pAckList[i])) == 0)
                {
                    *foundedIndex = i;
                    return RET_OK;
                }
            }
        }
 
    }while((osKernelGetTickCount()-startTime) < timeout_ms);

    return RET_TIME_OUT;
}


static M_RET_t tx700_check_tcpResp(const char *const*pAckList,uint32_t ackListCnt,uint32_t *foundedIndex,char *pBuff,uint32_t buffSize,uint32_t timeout_ms)
{
    uint32_t startTime;
    uint32_t i;
    M_RET_t ret = RET_FAIL;

    startTime = osKernelGetTickCount();

    do{
        ret = tx700_tcpRecv_response(pBuff,buffSize);

        if(ret == RET_OK)
        {
            for(i = 0 ; i< ackListCnt;i++)
            {
                if(strncmp((char *)pBuff,(char *)pAckList[i],strlen((char *)pAckList[i])) == 0)
                {
                    *foundedIndex = i;
                    return RET_OK;
                }
            }
        }
 
    }while((osKernelGetTickCount()-startTime) < timeout_ms);

    return RET_TIME_OUT;
}
/*

+CMGR: "REC READ","01053730725",,"25/06/09,13:39:56+36",02050550AF

*/
int extract_sms(char* sms, char* p_out_number, int number_size, char* p_out_msg, int msg_size)
{
    // 1. 입력 유효성 검사
    if (sms == NULL || p_out_number == NULL || p_out_msg == NULL || number_size <= 0 || msg_size <= 0)
    {
        return -1;
    }

    // 출력 버퍼를 안전하게 초기화
    *p_out_number = '\0';
    *p_out_msg = '\0';

    // 2. 전화번호 추출
    const char* first_comma = strchr(sms, ',');
    if (first_comma == NULL) return -1;

    const char* num_start = strchr(first_comma, '"');
    if (num_start == NULL) return -1;
    num_start++; // 따옴표(") 다음으로 이동

    const char* num_end = strchr(num_start, '"');
    if (num_end == NULL) return -1;

    size_t num_len = num_end - num_start;
    // 버퍼 크기를 넘지 않도록 복사할 길이 계산
    size_t num_to_copy = (num_len >= number_size) ? (number_size - 1) : num_len;
    
    strncpy(p_out_number, num_start, num_to_copy);
    p_out_number[num_to_copy] = '\0'; // NULL 종료 문자 추가

    // 3. 메시지 추출
    const char* msg_start = strrchr(sms, ',');
    if (msg_start == NULL) return -1;
    msg_start++; // 쉼표(,) 다음으로 이동

    size_t msg_len = strlen(msg_start);
    // 버퍼 크기를 넘지 않도록 복사할 길이 계산
    size_t msg_to_copy = (msg_len >= msg_size) ? (msg_size - 1) : msg_len;

    strncpy(p_out_msg, msg_start, msg_to_copy);
    p_out_msg[msg_to_copy] = '\0'; // NULL 종료 문자 추가

    return 0; // 성공
}

M_RET_t tx700_read_sms(sms_t *p_sms)
{
  const char *cmd = "AT+CMGR=0\r\n";           // 최근 문자 1개 읽기
  const char *delCmd = "AT+CMGD=,4\r\n";   // 전부 삭제
  const char *ackList[] = {"+CMGR"};
  char buff[310];
  uint32_t idx = 0;
  M_RET_t ret = RET_FAIL;
  int results=0;

  tx700_modem_sends(cmd);

  ret = tx700_check_asyncResp(ackList, CNT_OF(ackList), &idx, buff, sizeof(buff), 200);

  if (ret == RET_OK)
  {
    switch (idx)
    {
      case 0:
        ret = RET_OK;
        results = extract_sms(buff, p_sms->num, sizeof(p_sms->num), p_sms->msg, sizeof(p_sms->msg));
        modem_sends(delCmd);  // 읽은 메시지는 지운다
        break;
      case 1:
        ret = RET_FAIL_RESP;
        break;
      }
    }
    if (results !=0)
    {
      ret = RET_FAIL;
    }


      return ret;


}

/*
2025-06-09 14:57:24.873 [COM23] - AT+CMGS="01053730725"<CR><LF>
AT$$TCP_NULLPERMISSION=1<CR><LF>

2025-06-09 14:57:24.896 [COM24] - <CR><LF>
>
2025-06-09 14:57:24.927 [COM23] - App/Boot
Ver:(0.1.0/0.1.0)PCB:0,MFG:UNKNOWN,AREA:0,BUILD:1748256491,ID:335 <SUB>2025-06-09 14:57:24.944
[COM24] - <CR><LF>

2025-06-09 14:57:24.986 [COM23] - AT$$TCP_SCCL=0<CR><LF>

2025-06-09 14:57:25.070 [COM24] - <CR><LF>
+CMGS: 63<CR><LF>
<CR><LF>
OK<CR><LF>
<CR><LF>
$$TELL:45,?????? ???? ???? ????<CR><LF>

*/

#define TX700_SEND_SMS_RESP_CNT 1
M_RET_t tx700_send_sms(char *num,char *msg)
{
    char buff[200];
    const char *ack_list[TX700_SEND_SMS_RESP_CNT];
    int32_t len=0;
    uint32_t idx;
    M_RET_t ret = RET_FAIL;

    ack_list[0] = get_modem_string_tx700(AT_SMS_SEND_RESP);

    len = snprintf((char *)buff, sizeof(buff), "AT+CMGS=\"%s\"\r\n", num);
    tx700_modem_sends(buff);
    osDelay(500);

    len = snprintf((char *)buff, sizeof(buff), "%s", msg);

    buff[len++] = 0x1A;
    buff[len] = 0;

    tx700_modem_sends(buff);

    ret = tx700_check_asyncResp(ack_list, TX700_SEND_SMS_RESP_CNT, &idx, buff, sizeof(buff), 200);

    if (ret == RET_OK)
    {
      ret = RET_OK;
    }


    return ret;
}




#define TX700_OPEN_PPP_RESP_CNT 1
M_RET_t tx700_open_ppp(void)
{
  const char *ack_list[TX700_OPEN_PPP_RESP_CNT];
  char buff[100];
  uint32_t matched_index;


  tx700_modem_sends(get_modem_string_tx700(AT_TCP_OPEN_PPP));

  ack_list[0] = get_modem_string_tx700(AT_TCP_OPEN_PPP_RESP);

  tx700_check_tcpResp(ack_list, TX700_OPEN_PPP_RESP_CNT, &matched_index, buff, sizeof(buff),
                            1000);

  return RET_OK;
}

#define TX700_CLOSE_PPP_RESP_CNT 1
M_RET_t tx700_close_ppp(void)
{
  const char *ack_list[TX700_CLOSE_PPP_RESP_CNT];
  char buff[100];
  uint32_t matched_index;


  tx700_modem_sends(get_modem_string_tx700(AT_TCP_CLOSE_PPP));

  ack_list[0] = get_modem_string_tx700(AT_TCP_CLOSE_PPP_RESP);

 tx700_check_tcpResp(ack_list, TX700_CLOSE_PPP_RESP_CNT, &matched_index, buff, sizeof(buff),
                            1000);

  return RET_OK;
}

#define TX700_OPEN_SOCKET_RESP_CNT 1
M_RET_t tx700_open_socket(void)
{
  const char *ack_list[TX700_OPEN_SOCKET_RESP_CNT];
  char buff[100];
  uint32_t matched_index;
  M_RET_t ret = RET_FAIL;

  
  tx700_modem_sends(get_modem_string_tx700(AT_TCP_OPEN_SOCKET));

  ack_list[0] = get_modem_string_tx700(AT_TCP_OPEN_SOCKET_RESP_OK);

  ret = tx700_check_tcpResp(ack_list, TX700_OPEN_SOCKET_RESP_CNT, &matched_index, buff,
                                sizeof(buff), 10000);

  if(ret == RET_OK)
  {
    switch(matched_index)
    {
    case 0:
      
      break;
    default:
      ret = RET_FAIL;
      break;
    }
  }
  return ret;
}

#define TX700_CLOSE_SOCKET_RESP_CNT 1

M_RET_t tx700_close_socket(void)
{
  const char *ack_list[TX700_CLOSE_SOCKET_RESP_CNT];
  char buff[100];
  uint32_t matched_index;


  ack_list[0] = get_modem_string_tx700(AT_TCP_CLOSE_SOCKET_RESP);

  tx700_modem_sends(get_modem_string_tx700(AT_TCP_CLOSE_SOCKET));

   tx700_check_tcpResp(ack_list, TX700_CLOSE_SOCKET_RESP_CNT, &matched_index, buff,
                            sizeof(buff), 1000);

  return RET_OK;
}

const char *serviceCode1_[]={"0 No Service",
                            "1 : Limited Service",
                            "2 : Service Available",
                            "3 : Limited Regional Service",
                            "4 : MS is in Power Save or Deep Sleep",
                            "5 : No Service",
                            "6 : Limited Service",
                            "7 : Limited Regional Service",
                            "8 : Power Save"};

const char *serviceCode2_[]={"0 : Error None",
                            "1 : Related USIM",
                            "2 : Cell Restrict",
                            "3 : Access Control",
                            "4 : Out Of Service",
                            "5 : Attach Reject (NW->UE)",
                            "6 : Location Update Reject",
                            "7 : Detach Request",
                            "8 : Active Reject (NW->MS)",
                            "9 : Deactivate Request (NW->MS)",
                            "10 : Tracking Area Update Reject (NW->MS)"};



M_RET_t tx700_check_network_service(char *msgOut,uint16_t msgSize)
{
  msgOut[0] = 0;
  return RET_OK;
}


M_RET_t tx700_init(void)
{
    M_RET_t ret = RET_OK;

    tx700_modem_sends("ATE0V1\r\n");//E0 에코 금지 V1 응답은 아스키 형태
    osDelay(100);
    tx700_modem_sends("AT$$TCP_NULLPERMISSION=1\r\n"); 
    osDelay(100);
    return ret;
}

#define TX700_WRITE_IP_REST_CNT 1
void tx700_write_ip(uint8_t ip[4],uint16_t port)
{
  const char *ack_list[1];
  char  buff[50];
  uint32_t matched_index;


  ack_list[0] = get_modem_string_tx700(AT_TCP_WRITE_IP_RESP);

  snprintf(buff, sizeof(buff), "AT$$TCP_ADDR=0,%d,%d,%d,%d,%d\r\n", ip[0], ip[1], ip[2], ip[3],
             port);

  tx700_modem_sends(buff);

   tx700_check_tcpResp(ack_list, TX700_WRITE_IP_REST_CNT, &matched_index, buff, sizeof(buff),
                            1000);
}

#define TX700_RESET_SW_CNT_RESP 1
void tx700_resetSW(void)
{
  const char *ack_list[1]; 
  char  buff[50];
  uint32_t matched_index;


  ack_list[0] = get_modem_string_tx700(AT_RESET_SW_RESP);

  tx700_modem_sends(get_modem_string_tx700(AT_RESET_SW));

   tx700_check_tcpResp(ack_list, TX700_RESET_SW_CNT_RESP, &matched_index, buff, sizeof(buff),
                            200);
}
/*

2025-05-13 14:25:06.281 [COM15] - <CR><LF>
*BOOTALERT<CR><LF>

2025-05-13 14:25:06.382 [COM15] - <CR><LF>
^MODE: 9<CR><LF>

*/

void tx700_reset(uint8_t resetType,uint32_t delayMs)
{
    uint32_t cnt;
    
    switch(resetType)
    {
        case M_RESET_SW:
        tx700_resetSW();
        break;
        case M_RESET_HW:
          drv_power_off(DRV_POWER_CDMA);
          osDelay(2000);
          drv_power_on(DRV_POWER_CDMA);

          break;
    }

#if 1 
    if(delayMs)
    {
        cnt = delayMs/1000;
        for(uint32_t i = 0 ; i < cnt; i++ )
        {
            if(is_modemBoot())
            {
                osDelay(5000);// 부팅후 안정화 
                break;;
            }
            osDelay(1000);
        }
    }
#endif

}


M_RET_t tx700_recv_tcp(uint8_t *buff,uint16_t buffSize,uint16_t *recvLen,uint32_t timeOutMs)
{

    M_RET_t ret = RET_FAIL;


    if(recv_tcp(buff,buffSize,recvLen,timeOutMs)==0)
    {
        ret =  RET_OK;
    }

    if(is_modemBoot())
    {
        ret = RET_MODEM_ERR;
    }

    if(is_serverErr())
    {
        ret = RET_SERVER_ERR;
    }

    return ret;
}

#define TX700_SEND_TCP_RESP_CNT 1
M_RET_t tx700_send_tcp(uint8_t *data,uint16_t dataLen)
{
  const char *ack_list[TX700_SEND_TCP_RESP_CNT];
  char buff[512 + 64];
  uint32_t matched_index;
  M_RET_t ret;
  int32_t code=0;
  ack_list[0] = get_modem_string_tx700(AT_TCP_SEND_DATA_RESP);

  strcpy(buff, "AT$$TCP_SENDBIN=00");

  buff[16] = (0xFF & ((dataLen) >> 8));
  buff[17] = (0xFF & (dataLen));

  memcpy(&buff[18],(void *)data, dataLen);

  dataLen += 18;
  buff[dataLen++] = '\r';
  buff[dataLen++] = '\n';

  tx700_modem_send(buff, dataLen);

   ret = tx700_check_tcpResp(ack_list, TX700_SEND_TCP_RESP_CNT, &matched_index, buff, sizeof(buff),
                            10000);
  if(ret ==RET_OK)
  {
    if(matched_index ==0)
    {
      if (sscanf(buff, "$$TCP_SENDDATA:%d",&code) == 1)
      {
        if(code !=1)
        {
          ret = RET_FAIL;
        }
      }
    }
  }

  return ret;
}

#define TX700_READ_NUM_RESP_CNT 1
M_RET_t tx700_read_num(char *p_number,uint16_t number_size)
{
  const char *response_lst[TX700_READ_NUM_RESP_CNT];
  char buff[50];
  char number[20];
  uint32_t matched_index;
  M_RET_t ret = RET_FAIL;

  p_number[0] = '\0';

  tx700_modem_sends(get_modem_string_tx700(AT_READ_NUM));

  response_lst[0] = get_modem_string_tx700(AT_TCP_READ_NUM_RESP);
  
  ret = tx700_check_tcpResp(response_lst, TX700_READ_NUM_RESP_CNT, &matched_index, buff,
                            sizeof(buff), 200);
  if (ret == RET_OK)
  {
    //+CNUM: ,"01220891572",129
    if (sscanf(buff, "+CNUM: ,\"%19[^\"]", number) == 1)
    {
      strcpy_safe(p_number, number_size, number);
    }

  }

	return ret;	
}


/*
2025-02-07 10:22:25.384 [COM11] - AT+CSQ<CR><LF>

2025-02-07 10:22:25.400 [COM10] - <CR><LF>
+CSQ: 30,99<CR><LF>
<CR><LF>
OK<CR><LF>
*/
#define TX700_RSSI_RESP_CNT 1
M_RET_t tx700_read_rssi(int16_t *rssi)
{
  M_RET_t ret = RET_FAIL;
  char buff[50];
  const char *response_lst[TX700_RSSI_RESP_CNT];
  uint32_t matched_index;
  int32_t data=0;

  tx700_modem_sends(get_modem_string_tx700(AT_ASYNC_GET_RSSI));

  response_lst[0] = get_modem_string_tx700(AT_SYNC_GET_RSSI_RESP);

  ret = tx700_check_asyncResp(response_lst, TX700_RSSI_RESP_CNT, &matched_index, buff, sizeof(buff),
                              200);

  if(ret == RET_OK)
  {
    if(sscanf(buff,"+CSQ:%d",&data)==1)
    {
      *rssi = data;
      ret = RET_OK;
    }
    else
    {
      ret =RET_FAIL;
    }

  }

	return ret;	
}

/**
 * @brief dtmf 코드 추출 
 * @retval dtmf 코드
 */
char tx700_get_dtmf(char *data)
{
  char dtmf_code;

  //$DTMF: 4
  dtmf_code = data[7];

  return dtmf_code;
}


void tx700_vpn_init(void)
{
 


}


/**
 * @brief 안정적인 전원 차단을 위해서 아래와 같이 AT CMD 실행이 필요 합니다.
 */
void tx700_off_powerSafe(void)
{

}


M_RET_t tx700_read_ring_number(char *pData,char *prNum,uint16_t numSize)
{
    char *argv[MAX_ARGV];
    char *ptr = NULL;
    M_RET_t ret = RET_FAIL;

    parse_args(pData, argv,10);

    ptr = (char *)h_findnum((char *)argv[1]);

    if(ptr)
    {
        strcpy_safe(prNum,numSize,ptr);
        ret = RET_OK;
    }

    return ret;
}


#define CONNECT_CALL_RESPONSE_CNT 1
#define COONECT_CALL_RETRY 2
M_RET_t tx700_recv_call(void)
{
  const char *response_list[CONNECT_CALL_RESPONSE_CNT];
  char  buff[50];
  uint32_t matched_index = -1;
  M_RET_t ret = RET_FAIL;

  response_list[0] = get_modem_string_tx700(AT_ASYNC_OPEN_VOICE_RESP);

  for (uint32_t i = 0; i < COONECT_CALL_RETRY; i++)
  {
    tx700_modem_sends(get_modem_string_tx700(AT_ASYNC_OPEN_VOICE));

    ret = tx700_check_asyncResp(response_list, CONNECT_CALL_RESPONSE_CNT, &matched_index, buff,
                                sizeof(buff), 200);
    if (ret == RET_OK)
    {
      ret = RET_OK;
      break;
    }
    osDelay(1000);
  }

  return ret;
}

M_RET_t tx700_dial(char *num,uint32_t waitTimeOutMs)
{
  return RET_OK;
}


M_RET_t tx700_set_vpn(char *id,char *pw,uint8_t ip[4],uint16_t port)
{

    return RET_OK;
}

M_RET_t tx700_read_vpn(char *outBuffer,uint16_t outSize)
{

  return RET_OK;
}

M_RET_t tx700_at_direct(char *at,char *outBuffer,uint16_t outSize)
{
  M_RET_t ret = RET_FAIL;
  uint32_t startTime = osKernelGetTickCount();

  tx700_modem_sends(at);

  do{

      ret = tx700_asyncRecv_response(outBuffer,outSize);

      if(ret == RET_OK)
      {
        return RET_OK;
      }
  }while((osKernelGetTickCount()-startTime) < 1000);
  
  return ret;
}

/*
$$BinRecv:<NUL><SOH>2<CR><LF>
24 24 42 69 6E 52 65 63 76 3A 00 01 32 0D 0A
*/

extern void put_tcpData(uint8_t *data, uint16_t dataLen);
void tx700_recv_bin(int32_t port, uint8_t *p_data, uint16_t data_len)
{
  uint16_t len;

  (void)port;

  len = (p_data[10]&0x03)*256 + p_data[11];
  
  if (len)
  {
    put_tcpData((uint8_t *)&p_data[12], len);
  }

}

int32_t tx700_recv_handler(int32_t uart,uint8_t *buffer, uint16_t buffer_size)
{
  uint32_t startTime = osKernelGetTickCount();
  uint16_t cnt = 0;
  uint8_t ch;
  uint8_t bin_mode = 0;
  uint8_t first = 1;
  uint16_t len = 0;

  while (1)
  {
    if (drv_uart_recv(uart, &ch, 1, osWaitForever) == 1)
    {
      buffer[cnt++] = ch;

      if (cnt == 12 && first)
      {
        first = 0;
        if (strncmp((char *)buffer, "$$BinRecv:", 10) == 0)
        {
          bin_mode = 1;
          len = (buffer[10] & 0x0F) * 256 + buffer[11];
        }
      }
      else
      {
        if (bin_mode)
        {
          if (cnt >= (12 + len))
          {
            return cnt;
          }
        }
        else
        {
          if ((ch == '\r') || (ch == '\n'))
          {
            buffer[cnt - 1] = 0;
            return (cnt - 1);
          }
        }
      }

      if (cnt >= buffer_size)
      {
        return 0;
      }
    }
  }


}

extern void put_asyncResp(uint32_t cmd,char *pData,uint16_t dataLen);

void tx700_sms_handler(int32_t uart,char *data,uint16_t data_len)
{
  char buff[200];
  int32_t len=data_len;
  int32_t recv_len;
  int32_t total_len;
  memcpy(buff,data,data_len);

  buff[len++] = ',';

  recv_len = drv_uart_recv_crlf(uart, &buff[len], sizeof(buff) - len, 2000);

  total_len = recv_len +len;
  buff[total_len] = 0;

  put_asyncResp(AT_SYNC_SMS_READ_RESP_OK, buff, total_len);
}