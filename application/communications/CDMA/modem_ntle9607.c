
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "cmsis_os2.h"
#include "util_memory.h"

#include "modem_ntle9607.h"
#include "at_cmd.h"
#include "drv_rs232.h"

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

/// @brief at 명령어와 응답 목록
const atCmd_t cmd_ntle9607[] = {{AT_ASYNC_RECV_REBOOT, "^MODE: 9"},
                                {AT_ASYNC_RECV_TCP_DISCONNECTED, "*TCPDISCONNECTED"},
                                {AT_ASYNC_RECV_SMS, "+CMTI"},
                                {AT_ASYNC_RECV_RING, "+CLIP"},
                                {AT_ASYNC_RECV_TCP_DATA, "*TCPRD"},
                                {AT_ASYNC_RESP_VOICE_END, "*VOICE END"},
                                {AT_ASYNC_RECV_DTMF, "+RXDTMF"},
                                {AT_TCP_OPEN_PPP, "AT*NET*PPPOP\r\n"},
                                {AT_TCP_CLOSE_PPP, "AT*NET*PPPCL\r\n"},
                                {AT_TCP_OPEN_SOCKET, "AT*NET*SOCKOP\r\n"},
                                {AT_ASYNC_OFF_VOICE, "AT*VOICE*FLASH=0\r\n"},
                                {AT_ASYNC_OPEN_VOICE, "AT*VOICE*ANS\r\n"},
                                {AT_TCP_WRITE_IP_RESP, "*NET*SOCKPA"},
                                {AT_TCP_CLOSE_SOCKET, "*NET*SOCKCL"},
                                {AT_ASYNC_OPEN_VOICE_RESP, "*VOICE CONNECT"},
                                {AT_ASYNC_GET_RSSI, "*SKT*LEVEL"},
                                {AT_SYNC_GET_RSSI_RESP, "+CSQ"},
                                {AT_SYNC_SMS_READ_RESP_OK, "*SMS*MTREAD"},
                                {AT_ASYNC_SMS_READ_RESP_ERR, "+CMS ERROR"},
                                {AT_SMS_SEND_RESP, "*SMSACK"},
                                {AT_TCP_SEND_DATA_RESP, "*ANET*SOCKWR"},
                                {AT_TCP_OPEN_SOCKET_RESP_OK, "*TCPCONNECTED"},
                                {AT_TCP_OPEN_SOCKET_RESP_FAIL, "*TCPCONNECTFAIL"},
                                {AT_TCP_OPEN_SOCKET_RESP, "*ANET*SOCKOP"},
                                {AT_TCP_READ_NUM_RESP, "*SKT*DIAL"},
                                {AT_TCP_CONNECT_VPN_RESP, "*VPN*STATUS: Connected"},
                                {AT_TCP_OPEN_PPP_RESP, "*NET*PPPOP"},
                                {AT_TCP_CLOSE_PPP_RESP, "*NET*PPPCL"},
                                {AT_TCP_CLOSE_SOCKET_RESP, "*ANET*SOCKCL"},
                                {AT_RESET_SW_RESP, "*SET*RESET"},
                                {AT_ASYNC_DIAL_RESP, "+COLP"},
                                {AT_ASYNC_DIAL_OFF, "AT*VOICE*CEND\r\n"},
                                {AT_ASYNC_CONFIG_READ_RESP, "*VPN*CONFIG"},
                                {AT_TCP_NETWORK_SERVICE, "*ST*REGSTS:"}};

uint32_t ntle9607_get_count(void)
{
    return _countof(cmd_ntle9607);
}

const char *get_modem_string_ntle9607(eAT_COMMAND_t cmd)
{
  for (int i = 0; i < AT_MAX; i++)
  {
    if (cmd_ntle9607[i].cmd == cmd)
    {
      return cmd_ntle9607[i].cmdStr;
    }
  }

  return NULL;
}




static void ntle9607_modem_sends(const char *data)
{
    modem_sends(data);
}

static void ntle9607_modem_send(const char *data,uint16_t dataLen)
{
    modem_send(data,dataLen);
}

/*
async task에서 만 수신되도록한 응답을 확인
*/
static M_RET_t ntle9607_asyncRecv_response(char *pBuff,uint16_t buffSize)
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
static M_RET_t ntle9607_tcpRecv_response(char *pBuff,uint16_t buffSize)
{
    M_RET_t ret = RET_FAIL;
    uint32_t cmd;

    if(wait_tcpResp(&cmd,pBuff,buffSize) == 0)
    {
        ret = RET_OK;
    }

    return ret;
}


static M_RET_t ntle9607_check_asyncResp(const char *const*pAckList,uint32_t ackListCnt,uint32_t *foundedIndex,char *pBuff,uint32_t buffSize,uint32_t timeout_ms)
{

    uint32_t startTime;
    uint32_t i;
    M_RET_t ret = RET_FAIL;

    startTime = osKernelGetTickCount();

    do{
        ret = ntle9607_asyncRecv_response(pBuff,buffSize);

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


static M_RET_t ntle9607_check_tcpResp(const char *const*pAckList,uint32_t ackListCnt,uint32_t *foundedIndex,char *pBuff,uint32_t buffSize,uint32_t timeout_ms)
{
    uint32_t startTime;
    uint32_t i;
    M_RET_t ret = RET_FAIL;

    startTime = osKernelGetTickCount();

    do{
        ret = ntle9607_tcpRecv_response(pBuff,buffSize);

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
수신된 문자수신 명령어 에서 전화번호화 문자내용을 추출
msg 문자수신 명령어
sms 문자구조체

*SMS*MTREAD: 2022032815044236,"01053730725","313374"

*/
static void parse_sms(char* msg,sms_t *pSms)
{
    char* argv[10] = { NULL };// 매개값 목록
    char* ptr;
    uint32_t cnt;
    uint32_t len;

    memset(pSms,0x00,sizeof(sms_t));

    cnt = parse_args(msg, argv,10);// [*SMS*MTREAD:],[2022032815044236],["01053730725"],["313374"]
 
    if(cnt != 4)
    {
      return ;
    }
    
    ptr = (char *)h_findnum((char *)argv[2]);//전화번호 문자열 리턴

    if(ptr)
    {
        strcpy_safe(pSms->num,sizeof(pSms->num),ptr);    
    }

    ptr = argv[3]+1;//문자내용 리턴
    
    if (ptr)
    {
        len = strlen(ptr)-1;

        if (len < sizeof(pSms->msg))
        {
            len = Convert_HexAscii2uchar(ptr,len,(uint8_t *)pSms->msg);
            pSms->msg[len] = '\0';
        }
    }
}


M_RET_t ntle9607_read_sms(sms_t *pSms)
{
    const char *cmd    = "AT*SMS*MTREAD=0\r\n";//최근 문자 1개 읽기
    const char *delCmd = "AT*SMS*ALLDEL=3\r\n";//전부 삭제
    const char *ackList[] = {"*SMS*MTREAD","+CMS ERROR"};  
    char buff[310];
    uint32_t idx=0;
    M_RET_t ret = RET_FAIL;

    ntle9607_modem_sends(cmd);

    ret = ntle9607_check_asyncResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),1000);

    if(ret == RET_OK)
    {
        switch(idx)
        {
            case 0:
                ret = RET_OK;
                parse_sms(buff,pSms);
                modem_sends(delCmd); // 읽은 메시지는 지운다
                osDelay(1000);
                break;
            case 1:
                ret = RET_FAIL_RESP;
                break;
        }
    }

    return ret;


}



M_RET_t ntle9607_send_sms(char *num,char *msg)
{
    char buff[200];
    const char *ackList[] = {"*SMSACK"};  
    int32_t len=0;
    uint32_t idx;
    M_RET_t ret = RET_FAIL;

    len = snprintf((char *)buff,sizeof(buff),"AT*SMS*MO=%s,"",",num);

    if( (sizeof(buff)-len-2) >= (strlen((char *)msg)*2) )
    {
        len += Convert_ucharHexAscii((uint8_t *)msg,strlen(msg),&buff[len]);

        buff[len++] ='\r';
        buff[len++] ='\n';

        ntle9607_modem_sends(buff);

        ret = ntle9607_check_asyncResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),200);

        if(ret == RET_OK)
        {
            ret = RET_OK;
        }
 
    }

    return ret;
}



const char *get_ack(uint16_t cmd)
{
  int i;
  const char *ret =NULL;

  for(i =  0 ;i < _countof(cmd_ntle9607);i++)
  {
    if(cmd_ntle9607->cmd == cmd)
    {
      ret = cmd_ntle9607->cmdStr;
      break;
    }
  }
  return ret;
}
/*
2025-02-07 10:06:44.721 [COM11] - AT*NET*PPPOP<CR><LF>
2025-02-07 10:06:44.739 [COM10] - <CR><LF>
*PPPOPENED<CR><LF>
<CR><LF>
*NET*PPPOP:1<CR><LF>
<CR><LF>
OK<CR><LF>
<CR><LF>
*PPPOPENED<CR><LF>
*/
M_RET_t ntle9607_open_ppp(void)
{
  const char *ackList[]={"*NET*PPPOP:"};
  char buff[100];
  int32_t code;
  uint32_t idx;
  M_RET_t ret = RET_FAIL;

  osDelay(500);
  ntle9607_modem_sends(get_modem_string_ntle9607(AT_TCP_OPEN_PPP));
  ret = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),10000);

  if(ret == RET_OK)
  {
      sscanf(buff,"*NET*PPPOP:%d",&code);
      switch(code)
      {
          case 1:
              ret = RET_OK;
          break;
          default:
              ret = RET_FAIL_RESP;
          break;            
      }
  }
  
  return ret;
}


/*
2025-02-07 10:05:34.282 [COM11] - AT*NET*PPPCL<CR><LF>
2025-02-07 10:05:34.284 [COM10] - <CR><LF>
*PPPCLOSED<CR><LF>
<CR><LF>
*NET*PPPCL:1<CR><LF>
<CR><LF>
OK<CR><LF>
*/
M_RET_t ntle9607_close_ppp(void)
{

    const char *ackList[]= {"*NET*PPPCL:"};
    char buff[100];
    uint32_t idx;
    int32_t code;
    M_RET_t ret = RET_FAIL;

  osDelay(500);
  ntle9607_modem_sends(get_modem_string_ntle9607(AT_TCP_CLOSE_PPP));

  ret = ntle9607_check_tcpResp(ackList, CNT_OF(ackList), &idx, buff, sizeof(buff), 10000);

  if (ret == RET_OK)
  {
    sscanf(buff, "*NET*PPPCL:%d", &code);
    switch (code)
    {
      case 1:
        ret = RET_OK;
        break;
      default:
        ret = RET_FAIL_RESP;
        break;
    }
    }


    return ret;
}

M_RET_t ntle9607_open_socket(void)
{
    const char *const cmd= "AT*ANET*SOCKOP\r\n";
    const char *ackList[]= {"*TCPCONNECTED","*TCPCONNECTFAIL"};
    char buff[100];
    uint32_t idx;
    M_RET_t ret = RET_FAIL;

  osDelay(500);
    ntle9607_modem_sends(cmd);
    
    ret = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),10000);

    if(ret == RET_OK)
    {
        switch(idx)
        {
        case 0:
            ret = RET_OK;
            break;
        case 1:
            ret = RET_FAIL_RESP;
            break;
        }
    }
    return ret;
}

M_RET_t ntle9607_close_socket(void)
{
    const char *cmd= "AT*ANET*SOCKCL\r\n";
    const char *ackList[]= {"*TCPDISCONNECTED","*ANET*SOCKCL:0"};
    char buff[100];
    uint32_t idx;
    M_RET_t ret = RET_FAIL;

    osDelay(500);
    ntle9607_modem_sends(cmd);
    
    ret = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),10000);

    if(ret == RET_OK)
    {
        switch(idx)
        {
        case 1:
            ret = RET_OK;
            break;
        }
    }
    return ret;
}

const char *serviceCode1[]={"0 No Service",
                            "1 : Limited Service",
                            "2 : Service Available",
                            "3 : Limited Regional Service",
                            "4 : MS is in Power Save or Deep Sleep",
                            "5 : No Service",
                            "6 : Limited Service",
                            "7 : Limited Regional Service",
                            "8 : Power Save"};

const char *serviceCode2[]={"0 : Error None",
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



M_RET_t ntle9607_check_network_service(char *msgOut,uint16_t msgSize)
{
    const char *cmd = "AT*ST*REGSTS\r\n";
    const char *ackList[] = {"*ST*REGSTS:"}; 
    char  buff[50];
    int32_t code1,code2,code3;
    uint32_t idx;
    M_RET_t ret = RET_FAIL;

    ntle9607_modem_sends(cmd);
    ret = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),200);
    if(ret == RET_OK)
    {
      sscanf(buff,"*ST*REGSTS:%d,%d,%d",&code1,&code2,&code3);

      snprintf(msgOut,msgSize,"%s,%s,%d",serviceCode1[code1],serviceCode2[code2],code3);
      ret = RET_OK;
    }
  return ret;
}


M_RET_t ntle9607_init(void)
{
    M_RET_t ret = RET_OK;
 

    ntle9607_modem_sends("ATE0V1\r\n");//E0 에코 금지 V1 응답은 아스키 형태
    osDelay(500);
    ntle9607_modem_sends("AT*ST*REGSTS\r\n");//네트워크 서비스 상태 조회
    osDelay(500);
    /*
    *ST*REGSTS:2,0,0 
    2 서비스 가능능
    */
    return ret;
}


void ntle9607_write_ip(uint8_t ip[4],uint16_t port)
{
  const char *ackList[] = {"*ANET*SOCKPA:"}; 
  char  buff[50];
  uint32_t idx;
  int32_t code;
  M_RET_t ret = RET_FAIL;

    snprintf(buff, sizeof(buff),"AT*ANET*SOCKPA=%d.%d.%d.%d,%d\r\n", ip[0],ip[1],ip[2],ip[3],port);
    
    osDelay(500);
    ntle9607_modem_sends(buff);

    ret = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),1000);

    if(ret == RET_OK)
    {
      sscanf(buff,"*ANET*SOCKPA:%d",&code);
      switch(code)
      {
        case 1:// 전송 실패
          ret = RET_OK;
          break;
      }
    }
    


}

void ntle9607_resetSW(void)
{
    const char *cmd = "AT*SET*RESET\r\n";
    const char *ackList[] = {"*SET*RESET"}; 
    char  buff[50];
    uint32_t idx;
    M_RET_t ret = RET_FAIL;
   
    ntle9607_modem_sends(cmd);

    ret = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),200);

    if(ret == RET_OK)
    {
        ret = RET_OK;
    }


}
/*

2025-05-13 14:25:06.281 [COM15] - <CR><LF>
*BOOTALERT<CR><LF>

2025-05-13 14:25:06.382 [COM15] - <CR><LF>
^MODE: 9<CR><LF>

*/

void ntle9607_reset(uint8_t resetType,uint32_t delayMs)
{
    uint32_t cnt;
    
    switch(resetType)
    {
        case M_RESET_SW:
        ntle9607_resetSW();
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


M_RET_t ntle9607_recv_tcp(uint8_t *buff,uint16_t buffSize,uint16_t *recvLen,uint32_t timeOutMs)
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

    
M_RET_t ntle9607_send_tcp(uint8_t *data,uint16_t dataLen)
{
    const char *ackList[]={"*ANET*SOCKWR"};//*ANET*SOCKWR:1 성공,*ANET*SOCKWR:0 실패
    char buff[512+64];
    uint32_t findex;
    int32_t len;
    M_RET_t ret = RET_FAIL;
    uint8_t code;
    
    //AT*ANET*SOCKWR=10,1234567890\r\n
    len = snprintf((char *)buff,sizeof(buff),"AT*ANET*SOCKWR=%d,",dataLen);

    memcpy(&buff[len],data,dataLen);
    len += dataLen;

    buff[len++] = '\r';
    buff[len++] = '\n';


    ntle9607_modem_send(buff,len);


    ret = ntle9607_check_tcpResp(ackList,sizeof(ackList)/sizeof(ackList[0]),&findex,buff,sizeof(buff),10000);

    if(ret == RET_OK)
    {
      code = buff[13];//
      switch(code)
      {
        case '0':// 전송 실패
          ret = RET_FAIL_SEND;
          break;
	    case '1':
          ret = RET_OK;
          break;
      }
    }

    return ret;
}





M_RET_t ntle9607_read_num(char *prNum,uint16_t numSize)
{
	const char *cmd    = "AT*SKT*DIAL\r\n";
    const char *ackList[] = {"*SKT*DIAL:"};           //*SKT*DIAL:01227090440<CR><LF>
    char  buff[50];
    uint32_t idx;
    int32_t len;
    M_RET_t ret = RET_FAIL;

    prNum[0] = '\0';


    ntle9607_modem_sends(cmd);

    ret  = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),200);
    if(ret == RET_OK)
    {
        //*SKT*DIAL:01227090440<CR><LF>

        len = strlen(&buff[10]);

        memcpy_safe((uint8_t *)prNum,numSize,(uint8_t *)&buff[10],len);
        prNum[len] = '\0';
        ret = RET_OK;
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
M_RET_t ntle9607_read_rssi(int16_t *rssi)
{
    const char *cmd = "AT+CSQ\r\n";
    const char *ackList[] = {"+CSQ"}; 
    char  buff[50];
    uint32_t idx;
    char *endptr;
    M_RET_t ret = RET_FAIL;
    char *argv[10]={0};

    ntle9607_modem_sends(cmd);

    ret = ntle9607_check_asyncResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),200);

    if(ret == RET_OK)
    {
        parse_args(buff,argv,10);
        *rssi = strtol(argv[1],&endptr,10);
        ret = RET_OK;
    }

	return ret;	
}

/**
 * @brief dtmf 코드 추출 
 * @retval dtmf 코드
 */
char ntle9607_get_dtmf(char *data)
{
    char dtmfCode;

 // "+RXDTMF: 1"에서 dtmf 코드만 추출
    dtmfCode = data[9];

    return dtmfCode;
}


void ntle9607_vpn_init(void)
{
  const char *connect_cmd =  "AT*VPN*CONNECT=1\r\n";
  const char *satus_cmt= "AT*VPN*STATUS\r\n";
  const char *disconnect_cmd = "AT*VPN*CONNECT=0\r\n";
  const char *ackList[]={cmd_ntle9607[AT_TCP_CONNECT_VPN_RESP].cmdStr};
  uint8_t j;
  uint32_t i;
  uint32_t idx;
  char buff[512];
    M_RET_t ret = RET_FAIL;
  /*
  전원이 투입되면 vpn 자동연결이 시도되는듯한, 모뎀 전원 리셋후 vpn 상태읽기 하면 
  connected 가 되는 경우 존재
  어떤경우에는 아무리 상태확인해도 연결이 안됨,이상태에서 껐다켜고 상태만 확인하면 연결이 되어있음
  그래서 일단 부팅되면 연결이 되었는지 확인하고 안되어있으면 연결 명령어를 시도
  */

  for(j = 0 ; j < 5; j++)//약 15초 동안 vpn 로그인 상태 확인
  {
    /* vpn 연결 되었는지 확인 */
    ntle9607_modem_sends(satus_cmt);

    ret = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),200);
    
    if(ret == RET_OK)
    {
      //연결이 완료되었으면 빠져나옴
      goto LOOP_EXIT;
    }
  }

  ntle9607_modem_sends(disconnect_cmd);// 
  osDelay(2000);// 정해진 지연 시간은 없음, 적당히 지연 
  ntle9607_modem_sends(connect_cmd);
  osDelay(2000);// 정해진 지연 시간은 없음, 적당히 지연

  for(j = 0 ; j< 2; j++)
  {
    for(i = 0 ; i < 5;i++)
    {
      /*vpn 연결 되었는지 확인*/
      ntle9607_modem_sends(satus_cmt);

      ret = ntle9607_check_tcpResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),200);
      if(ret == RET_OK)
      {
        //연결이 완료되었으면 빠져나옴
        goto LOOP_EXIT;
      }
                  
      osDelay(1000);
    }
    
    if(j == 0)// j==0일때 연결이 안되면 다시 연결종료 명령어 전송하고 다시 연결시도
    {
      ntle9607_modem_sends(disconnect_cmd);// 잘못된 연결  해제 
      osDelay(5000);
      ntle9607_modem_sends(connect_cmd);
    }
  }

LOOP_EXIT:
  (void)(0);//warning 때문에 넣음

}


/**
 * @brief 안정적인 전원 차단을 위해서 아래와 같이 AT CMD 실행이 필요 합니다.
 */
void ntle9607_off_powerSafe(void)
{
    const char *cmd = "AT+APMODE=0\r\n";

    ntle9607_modem_sends(cmd);

    osDelay(11000);//엔티모아에서 권장한 지연시간(email)
}


M_RET_t ntle9607_read_ringNum(char *pData,char *prNum,uint16_t numSize)
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


M_RET_t ntle_9607_recv_call(void)
{
    M_RET_t ret = RET_FAIL;
    const char *ackList[] = {cmd_ntle9607[AT_ASYNC_OPEN_VOICE_RESP].cmdStr}; 

    uint32_t idx;
    char  buff[50];

    for(uint32_t i =  0 ; i < 2; i++)
    {
        ntle9607_modem_sends( cmd_ntle9607[AT_ASYNC_OPEN_VOICE].cmdStr);

        ret = ntle9607_check_asyncResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),200);

        if(ret == RET_OK)
        {
            ret = RET_OK;
            break;
        }
        osDelay(1000);
    }

    return ret;
}

#define NTLE9607_DIAL_ACK_CNT 2
M_RET_t ntle_9607_dial(char *num,uint32_t waitTimeOutMs)
{
    M_RET_t ret = RET_FAIL;
    const char *ack_list[NTLE9607_DIAL_ACK_CNT];
    uint32_t matched_index=0;
    char  buff[50];

    ack_list[0] = get_modem_string_ntle9607(AT_ASYNC_DIAL_RESP);
    ack_list[1] = "OS_DIAL_OFF";


        snprintf(buff, sizeof(buff), "AT*VOICE*ORI=%s\r\n", num);

    ntle9607_modem_sends(buff);

    ret = ntle9607_check_asyncResp(ack_list, NTLE9607_DIAL_ACK_CNT, &matched_index, buff,
                                   sizeof(buff), waitTimeOutMs);

    if(ret == RET_OK)
    {
      switch (matched_index)
      {
        case 0:
          ret = RET_OK;
          break;
        case 1:
          ret = RET_FAIL;
          break;
      }

    }

    return ret;
}


M_RET_t ntle_9607_set_vpn(char *id,char *pw,uint8_t ip[4],uint16_t port)
{
  M_RET_t ret = RET_FAIL;
  char  buff[100];
  const char *ackList[] = {"OK"}; 
  uint32_t idx;

  snprintf(buff,sizeof(buff),"AT*VPN*CONFIG=%s,%s,%d.%d.%d.%d,%d\r\n",id,pw,
  ip[0],ip[1],ip[2],ip[3],port);

  ntle9607_modem_sends(buff);

  ret = ntle9607_check_asyncResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),1000);

    if(ret == RET_OK)
    {
        switch(idx)
        {
            case 0:
            ret = RET_OK;
            break;
        }
    }
  return ret;
}

M_RET_t ntle_9607_read_vpn(char *outBuffer,uint16_t outSize)
{
  M_RET_t ret = RET_FAIL;
  char  buff[100];
  const char *ackList[] = {"*VPN*CONFIG"}; 
  uint32_t idx;

//*VPN*CONFIG:test10,test135!@,112.221.177.172,4430<CR>

  snprintf(buff,sizeof(buff),"AT*VPN*CONFIG?\r\n");

  ntle9607_modem_sends(buff);

  ret = ntle9607_check_asyncResp(ackList,CNT_OF(ackList),&idx,buff,sizeof(buff),1000);

  if(ret == RET_OK)
  {
    switch(idx)
    {
        case 0:
        ret = RET_OK;
        break;
    }
  }
  return ret;
}

M_RET_t ntle_9607_at_direct(char *at,char *outBuffer,uint16_t outSize)
{
  M_RET_t ret = RET_FAIL;
  uint32_t startTime = osKernelGetTickCount();

  ntle9607_modem_sends(at);

  do{

      ret = ntle9607_asyncRecv_response(outBuffer,outSize);

      if(ret == RET_OK)
      {
        return RET_OK;
      }
  }while((osKernelGetTickCount()-startTime) < 1000);
  
  return ret;
}

extern void put_tcpData(uint8_t *data, uint16_t dataLen);
void ntle9607_recv_bin(int32_t uart, uint8_t *p_data, uint16_t data_len)
{
  uint16_t cnt;
  uint8_t temp[512 + 32];
  int32_t readCnt;
  int32_t len;

  cnt = data_len - 7;  //*TCPRD=4<CR><LF>에서 숫자의 자리수

  if (cnt < sizeof(temp))
  {
    memset(temp, 0x00, sizeof(temp));
    memcpy(temp, &p_data[7], cnt);
    readCnt = atoi((char *)temp);  // 수신 처리해야할 tcp data 길이를 계산

    len = drv_uart_recv(uart, (uint8_t *)temp, 1, 1000);  // 최종 tcp data 버퍼에서 가져옴
    len = drv_uart_recv(uart, (uint8_t *)temp, readCnt,
                           1000);  // 최종 tcp data 버퍼에서 가져옴

    if (len)
    {
      put_tcpData(temp, len);
    }
  }
}

int32_t ntle9607_recv_handler(int32_t uart, uint8_t *buffer, uint16_t buffer_size)
{

  int32_t len = 0;

  len = drv_uart_recv_crlf(uart,(char *)buffer, buffer_size, osWaitForever);


  return len;

}