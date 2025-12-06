

#include "app_logging.h"
#include "at_cmd.h"
#include "bsp.h"
#include "bsp_uart.h"
#include "config_app.h"
#include "debug_io.h"
#include "drv_do.h"
#include "drv_rs232.h"
#include "FreeRTOS.h"
#include "kma_protocol_handler.h"
#include "modem_if.h"
#include "modem_ntle9607.h"
#include "modem_sms.h"
#include "modem_tx700.h"
#include "os_user_def.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "system_err.h"
#include "task_cellular.h"
#include "task_logging.h"
#include "task_wdt.h"
#include "update_fw.h"
#include "util_time.h"

typedef enum{
	ePOWER_RESET,
	eCONNECT_TCP_WDT,
	eCONNECT_SW_RESET,
	eCONNECT_HW_RESET,
	eCONNECT_TX_FAIL,
	eCONNECT_MODEM_REBOOT,
	eCONNECT_SOCKET_CLOSED,
	eCONNECT_CHANG_IP_REQ,
	eCONNECT_CONNECTION_WDT
}eConnect_Type_t;

#define MODEM_PORT COM_CDMA
#define MODEMSMS_MAIL_SIZE   3
#define MODEM_RESP_MAIL_SIZE  3
#define CALLREQ_MAIL_SIZE   1
#define TCP_OK  (0U)

#define MODEM_DIAL_ON 1
#define MODEM_DIAL_OFF 0

#define STATUS_OK 0
#define STATUS_FAIL 1
#define WAIT_FOREVER 0

#define RTOSAL_WAIT_FOREVER osWaitForever
typedef uint32_t STATUS_t;

typedef struct
{
    uint8_t call_connected;//0 끊김,1 연결됨
    uint8_t server_closed;//1 끊김
    uint8_t modemPwr;//0 전원 꺼짐,1켜짐(AT 명령어 처리 가능 상태)
    uint8_t tcp_connected;//0 연결 안됨, 1 연결됨
    uint8_t networkStatus;//0 네트워크 등록안됨, 1 등록됨
    int16_t rssi;
    uint8_t modemReboot;//오직 Task A에서 1인지만 확인
    uint8_t init;//0 초기화 안됨, 1 초기화됨
    uint8_t smsRecvCnt;
    uint8_t ringReceived;
    uint8_t ringCnt;
    uint8_t voiceEnd;
    char ringNum[20];
    uint8_t tcpData[1];
    uint16_t tcpDataLen;
    uint8_t smsSendOk;
    uint8_t dial;//1 전화 연결됨,0 전화 끊김
    uint8_t phoneNumChecked;
}modemEx_t;

typedef struct 
{
  uint32_t cmd;
  char buff[400];
}resp_async_t;

typedef struct 
{
  uint32_t cmd;
  char buff[512];
}resp_tcp_t;

typedef struct 
{
  uint16_t len;
  uint8_t data[512];
}tcpData_t;

const osThreadAttr_t kAtTask_attributes = {
  .name = "cdma AT",
  .stack_size = TASK_STACK(TASK_CDMA_AT_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_CDMA_AT_DEF),
};

const osThreadAttr_t kTcpTask_attributes = {
  .name = "cdma TCP",
  .stack_size = TASK_STACK(TASK_CDMA_TCP_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_CDMA_TCP_DEF),
};


const osThreadAttr_t kAsyncTask_attributes = {
  .name = "cdma ASYNC",
  .stack_size = TASK_STACK(TASK_CDMA_ASYNC_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_CDMA_ASYNC_DEF),
};

static const atCmd_t *s_p_atCmd;


static osSemaphoreId_t s_modem_sem_id = NULL;
static osMessageQueueId_t s_resp_async_mail_id=NULL;
static osMessageQueueId_t s_resp_tcp_mail_id=NULL;
static osMessageQueueId_t s_tcp_data_mail_id=NULL;
static modemEx_t s_modem;
static uint8_t s_cdma_retarget_ip[4];
static uint16_t s_cdma_retarget_port=0;
static bool  s_cdma_retarget=false;

iCellular_t g_iCellular;
iCellular_t *_iCellular=NULL;
cdma_system_t g_cdma_system;
modem_config_t g_modem_config;


void set_cdma_retarget(bool target)
{
  s_cdma_retarget = target;
}

bool is_cdma_retarget(void)
{
  return s_cdma_retarget;
}

void set_cdma_retarget_ip(uint8_t ip[4],uint16_t port)
{
  s_cdma_retarget_ip[0] = ip[0];
  s_cdma_retarget_ip[1] = ip[1];
  s_cdma_retarget_ip[2] = ip[2];
  s_cdma_retarget_ip[3] = ip[3];
  s_cdma_retarget_port = port;
}

void get_cdma_retarget_ip(uint8_t ip[4],uint16_t *port)
{
  ip[0] = s_cdma_retarget_ip[0];
  ip[1] = s_cdma_retarget_ip[1];
  ip[2]  = s_cdma_retarget_ip[2];
  ip[3]  = s_cdma_retarget_ip[3];

  *port = s_cdma_retarget_port;

}

void modem_send(uint8_t *pData,uint16_t dataLen);
void modem_sends(const char *pData);
void get_ip(uint8_t pIp[4],uint16_t *pPort);
bool is_ipChanged(void);
void modem_set_dial(uint8_t status);
void flush_reqCall(void);

void modem_send(uint8_t *pData,uint16_t dataLen)
{
  bsp_uart_send(_iCellular->io_uart, pData, dataLen);
}

void modem_sends(const char *pData)
{
  bsp_uart_send(_iCellular->io_uart,  (uint8_t *)pData, strlen(pData));
}


/*텔라딘 기준으로 변환
0 113 dBm or less
1 111 dBm
2~30 109~52dBm (109~108dBm= rssi 2, 107~106dBm= rssi 3 ,,, 55~54dBm= rssi 29 53~52dBm= rssi 30)
31 51dBm or greater
99 unknown or not detectable
*/
int32_t convert_rssi_nt9607totx700(int32_t rssi)
{
	int i;
	int minRssi, maxRssi;
	int32_t tx700rssi = 0;

	if (rssi <= -113)
	{
		tx700rssi = 0;
	}
	else if (rssi == -111)
	{
		tx700rssi = 1;
	}
	else if (rssi >= -51)
	{
		tx700rssi = 31;
	}
	else
	{
		for (i = 2; i <= 30; i++)
		{
			minRssi = 109 - (i - 2) * 2;
			maxRssi = minRssi - 1;

			minRssi = -minRssi;
			maxRssi = -maxRssi;

			if (rssi >= minRssi && rssi <= maxRssi)
			{
				tx700rssi = i;
				break;
			}
		}

		if (tx700rssi == 0)//위의 for에서 찾지 못함
		{
			tx700rssi = 99;
		}
	}


	return tx700rssi;

}

void modem_send_sms(char *number,char *msg)
{
  _iCellular->send_sms(number, msg);
}

void modem_init(void)
{
    char num[20];

    modem_set_dial(MODEM_DIAL_OFF);   
// 모뎀 부팅하고 명령어 시도시 인식 안되는 경우가 존재

    for(uint32_t i = 0 ; i < 3;i++)
    {
        if(_iCellular->read_num(num,sizeof(num))==RET_OK)
        {
             strcpy_safe(g_cdma_system.num,sizeof(g_cdma_system.num),num);
            s_modem.phoneNumChecked = 1;
            break;
        }
        osDelay(5000);
    }
}

void modem_voice_init(void)
{
    (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    s_modem.modemReboot = 0;
    s_modem.voiceEnd = 1;
    s_modem.ringReceived = 0;
    s_modem.ringCnt = 0;
    s_modem.dial = 0;
    (void)osSemaphoreRelease(s_modem_sem_id);

    flush_reqCall();
}

void modem_socket_init(void)
{
    (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    s_modem.server_closed = 0;

    (void)osSemaphoreRelease(s_modem_sem_id);
}


void modem_set_dial(uint8_t status)
{
    (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    s_modem.dial = status;

    (void)osSemaphoreRelease(s_modem_sem_id);
}


bool is_vpn_enabled(void)
{

  if(get_config_app()->cdma_model == eCDMA_NTLE9607)
  {
    return get_config_app()->cdma_vpn_active;
  }

  return false;
}

#define CONNECT_TIMEOUT_MS 43200000
/**
 * @brief 모뎀 초기화 
 * tcp ip 통신을 위해 서버 접속
 * @return 서버 접속이 되어야 리턴함
*/
STATUS_t connect_tcp(eConnect_Type_t type)
{
    uint8_t connectionCnt = 2;// 초기hW 리셋됨
    uint8_t ip[4];
    uint16_t port;
    STATUS_t connection = STATUS_FAIL;
    uint32_t start_time;
    M_RET_t ret;

    start_time = osKernelGetTickCount();
    
    do
    {
        if(type == eCONNECT_TCP_WDT)
        {
            if(connectionCnt == 1)  // 서버 연결 1회 시도 실패면 sw 리셋
            {
                log_printf(L_INFO,"MODEM SW RESET");
                _iCellular->reset(M_RESET_SW,20000);
                type = eCONNECT_MODEM_REBOOT;
            }
            else if(connectionCnt == 2)// 2회 시도 실패면 hw 리셋
            {
                log_printf(L_INFO, "MODEM HW RESET");
                _iCellular->off_powerSafe();
                _iCellular->reset(M_RESET_HW,20000);
                type = eCONNECT_MODEM_REBOOT;
                start_time = osKernelGetTickCount();
            }
            else if(connectionCnt == 3) 
            {
                connectionCnt = 2;
                if((osKernelGetTickCount()-start_time)>CONNECT_TIMEOUT_MS)//12시간
                {
                  log_printf(L_INFO, "MODEM RESET TIMEOUT");    
                  connectionCnt = 0;
                }
                modem_socket_init();
            }
        }


        switch(type)
        {
            case ePOWER_RESET:
                 _iCellular->reset(M_RESET_HW,_iCellular->resetDelay);
            case eCONNECT_MODEM_REBOOT:
                modem_init();
                _iCellular->check_network_service(g_cdma_system.network_service_msg,sizeof(g_cdma_system.network_service_msg));
                modem_voice_init();
                modem_socket_init();
                if (is_vpn_enabled())
                {
                    _iCellular->vpn_init();
                }
            break;
            case eCONNECT_SOCKET_CLOSED:
            case eCONNECT_CONNECTION_WDT:

                break;

        }
    
    _iCellular->init();
    _iCellular->close_tcp();
    _iCellular->close_ppp();

    modem_socket_init();
    get_ip(ip,&port);
    
    if(is_cdma_retarget())
    {
      get_cdma_retarget_ip(ip,&port);
      set_cdma_retarget(false);
    }
    
    _iCellular->write_ip(ip,port);

    _iCellular->open_ppp();


    ret = _iCellular->open_tcp();
    connectionCnt++;

    if(ret == RET_OK)
    {
        connection = STATUS_OK;
        break;
    }

    type = eCONNECT_CONNECTION_WDT;
    osDelay(1000);

    }while(connection != STATUS_OK);

    log_printf( L_INFO,"SERVER Connected");
    return connection;
}

/**
 * @brief tcp 데이터 수신
 * @param pBuff 수신처리 버퍼
 * @param buffSize pBuff의 사이즈
 * @param pLen 수신된 데이터 길이
 * @param timeOutMs 수신 대기 시간
 * @retval 0 수신됨,1 수신없음
 */
uint32_t recv_tcp(uint8_t *pBuff,uint16_t buffSize,uint16_t *pLen,uint32_t timeOutMs)
{
#if 0
    uint32_t ret = 1;
    osEvent event;
    tcpData_t *tcpData;
    *pLen = 0;
    event = osMailGet(s_tcp_data_mail_id,10);
    if(event.status == osEventMail)
    {
        tcpData = (tcpData_t *)event.value.p;

        memcpy_safe(pBuff,buffSize,tcpData->data,tcpData->len);
        *pLen = tcpData->len;

        osMailFree(s_tcp_data_mail_id, tcpData);
        ret = 0;
    }
        return ret;
#else
    uint32_t ret = 1;
    tcpData_t tcpData;
    if(osMessageQueueGet(s_tcp_data_mail_id, &tcpData, NULL, 1000) == osOK)
    {
      memcpy_safe(pBuff,buffSize,tcpData.data,tcpData.len);
      *pLen = tcpData.len;
      ret = 0;
    }
    return ret;
#endif

}

void get_ip(uint8_t *prIp,uint16_t *pPort)
{

  prIp[0] = config.cdma_server_ip[0];
  prIp[1] = config.cdma_server_ip[1];
  prIp[2] = config.cdma_server_ip[2];
  prIp[3] = config.cdma_server_ip[3];

  *pPort = config.cdma_port;

}

#define PING_TIMEOUT_MS 120000

/**
 * @brief 모뎀이 리셋되었는지 체크
 * @retval true 리셋됨, false 리셋안됨
 */
bool is_modemBoot(void)
{
    bool err = false;

    (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    if(s_modem.modemReboot)
    {
        s_modem.modemReboot = 0;
        err = true;
    }

    (void)osSemaphoreRelease(s_modem_sem_id);

    return err;
}

/**
 * @brief 서버가 소켓을 닫았는지 체크
 * @retval true 소켓 닫힘, false 이상없음
 */
bool is_serverErr(void)
{
    bool err=false;

    (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    if(s_modem.server_closed)
    {
        err = true;
    }

    (void)osSemaphoreRelease(s_modem_sem_id);

    return err;
}




/**
 * @brief ip가 변경되었는지 판단
 * @retval true ip 변경요청,false 요청없음
 */ 
bool is_ipChanged(void)
{
    bool is = false;



    return is;
}





/**
 * @brief async 관련 at 응답또는 데이터 수신
 * @param cmd 수신된 cmd index
 * @param pBuff 수신 버퍼
 * @param buffSize pBuff 사이즈
 * @retval 0 수신,1수신없음
 */
uint32_t wait_asyncResp(uint32_t *cmd,char *pBuff,uint16_t buffSize)
{
  #if 0 
    resp_async_t *resp = NULL;
    osEvent event;
    uint32_t ret = 1;

    event = osMailGet(s_resp_async_mail_id,10);
    if(event.status == osEventMail)
    {
            resp = (resp_async_t *)event.value.p;

            *cmd = resp->cmd;

            strcpy_safe(pBuff,buffSize,resp->buff);
        
            osMailFree(s_resp_async_mail_id, resp);
            ret = 0;
        }
  
    return ret;
    #else
    resp_async_t resp;
    uint32_t ret = 1;
    if(osMessageQueueGet(s_resp_async_mail_id ,&resp, NULL, 10) == osOK)
      {
            *cmd = resp.cmd;

            strcpy_safe(pBuff,buffSize,resp.buff);
            ret = 0;
      }

      return ret;
    #endif
}

/**
 * @brief async관련 데이터 메시지큐 저장
 * @param cmd 수신된 명령어 index
 * @param pData 수신된 명령어 응답 또는 데이터
 * @param dataLen pData의 길이
 */
void put_asyncResp(uint32_t cmd,char *pData,uint16_t dataLen)
{
  #if 0 
    resp_async_t *resp = NULL;

    resp = (resp_async_t *)osMailAlloc(s_resp_async_mail_id,osWaitForever);

    if(resp)
    {
        resp->cmd = cmd;
        memcpy_safe((uint8_t *)resp->buff,sizeof(resp->buff),(uint8_t *)pData,dataLen);
        resp->buff[dataLen] = '\0';
        osMailPut(s_resp_async_mail_id,resp);
    }
    #else
    resp_async_t resp;

         resp.cmd = cmd;
        memcpy_safe((uint8_t *)resp.buff,sizeof(resp.buff),(uint8_t *)pData,dataLen);
        resp.buff[dataLen] = '\0';

    if (osMessageQueuePut(s_resp_async_mail_id, &resp, 0, osWaitForever) == osOK)
    {
 
    }
    #endif
}

/**
 * @brief tcp 응답수신
 * @param cmd 수신된 명령어 cmd index
 * @param pBuff 수신된 AT 응답
 * @param buffSize pBuff 버퍼 크기
 * @retval 0 정상,1 수신된 응답 없음
 */ 
uint32_t wait_tcpResp(uint32_t *cmd,char *pBuff,uint16_t buffSize)
{
  resp_tcp_t resp ;

  uint32_t err = 1;
  if(osMessageQueueGet(s_resp_tcp_mail_id, &resp, NULL, 10) == osOK)
  {
    *cmd = resp.cmd;
    strcpy_safe(pBuff,buffSize,resp.buff);
    err = 0;
  }

  return err;


}

/**
 * @brief tcp 응답 메시지 큐 전송
 * @param cmd 명령어 index
 * @param pData 수신된 tcp관련 at 명령어 응답또는 데이타
 * @param dataLen pData의 길이
 */
void put_tcpResp(uint32_t cmd,uint8_t *pData,uint16_t dataLen)
{
  resp_tcp_t resp;

  resp.cmd = cmd;

  memcpy((uint8_t *)resp.buff,(uint8_t *)pData,dataLen);
  resp.buff[dataLen] = '\0';

  if (osMessageQueuePut(s_resp_tcp_mail_id, &resp, 0, 100) != osOK)
  {
    dbg_printf("put_tcpResp error\r\n");
  }


}


/**
 * @brief 전화를 끊기
 */ 
void off_call(void)
{
    modem_sends(s_p_atCmd[AT_ASYNC_OFF_VOICE].cmdStr);
}
/**
 * @brief 발신 중지
 */
void dial_off(void)
{
    modem_sends(s_p_atCmd[AT_ASYNC_DIAL_OFF].cmdStr);
}

/**
 * @brief 수신한 SMS 있는지 확인
 * @retval true 수신한 sms 있음
 *         false 수신한 sms 없음
 */ 
bool is_smsRx(void)
{
     bool is = false;

    (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    if(s_modem.smsRecvCnt)
    {
        s_modem.smsRecvCnt--;// 읽은 후 지워야 하는지 추후 
        is = true;
    }

    (void)osSemaphoreRelease(s_modem_sem_id);

    return is;
}

/**
 * @brief 전화가 수신되었는지 확인
 * @param prNum 수신된 전화번호
 * @param numSize num 버퍼의 크기
 * @param prCnt 벨리 울린 횟수
 * @retval ture 수신됨, false 수신없음
*/
bool is_ringReceived(char *prNum,uint16_t numSize,uint16_t *prCnt)
{
    bool is = false;

    (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    if(s_modem.ringReceived)
    {
        strcpy_safe(prNum,numSize,s_modem.ringNum);
        *prCnt = s_modem.ringCnt;
        is = true;
    }

    (void)osSemaphoreRelease(s_modem_sem_id);

    return is;
}


/**
 * @brief 수신차단 번호인지 확인
 * @retval true  차단 번호
 *         false 차단 번호 아님
 */
bool is_rejectCallNum(char *callNum)
{
#ifdef REJECT_CALL_USE
    bool isRejectNum = false;

    uint32_t i;


    for(i = 0 ; i < REJECT_CALL_NUM_MAX;i++)
    {
        if(strncmp(callNum,(char *)config.rejectCallNum[i],strlen(callNum))==0)
        {
            isRejectNum = true;
            break;
        }
    }


    return isRejectNum;
#else

return false;
#endif

}
/**
 * @brief 수신된 전화 정보 초기화
 */
void modem_clear_ring(void)
{
   (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    s_modem.ringReceived = 0;;
    s_modem.ringCnt = 0;
    memset(s_modem.ringNum,0x00,sizeof(s_modem.ringNum));

  (void)osSemaphoreRelease(s_modem_sem_id);

}


bool modem_is_dialOk(void)
{
    bool is= false;

   (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);
    if(s_modem.dial)
    {
        is = true;
    }
    
    memset(s_modem.ringNum,0x00,sizeof(s_modem.ringNum));

  (void)osSemaphoreRelease(s_modem_sem_id);

  return is;
}

/**
 * @brief 발신전화 목록 비움
 */
void flush_reqCall(void)
{


}

/**
 * @brief 전화발신 요청
 * @param num 수신자 전화번호
 * @param waitTimeOutMs 전화 대기 시간,이시간동안 안받으면 자동 종료
 */
void dial_call(char *num,uint32_t waitTimeOutMs)
{
#if 0 
    callReq_t *call = NULL;

    call = (callReq_t *)osMailAlloc(_callReqMailId,osWaitForever);

    if(call)
    {
        strcpy_safe(call->num,sizeof(call->num),num);
        call->waitTimeOutMs = waitTimeOutMs;
        osMailPut(_callReqMailId,call);
    }
#endif
}



void proc_sms(void)
{
    sms_t sms;

    if(is_smsRx())
    {
        if(_iCellular->read_sms(&sms) == RET_OK)
        {

		        log_printf(L_INFO, "SMS: %s", sms.num);
            sms_cmd(&sms);
        }
    }
}

#define READ_RSSI_SCAN_TIME_MS 60000

void modemAsyncTask(void  *argument)
{
    int16_t rssi;
    uint32_t start_time;
    uint8_t once = 1;
    uint8_t rssiRead =0;
    start_time = osKernelGetTickCount();

  DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"modemAsync task start\r\n");
    while(1)
    {
        osDelay(1000);
        proc_sms();
        if(once)
        {
           if(s_modem.phoneNumChecked == 1)
           {
            once = 0;
            rssiRead = 1;
           }
        }
        if(((osKernelGetTickCount() - start_time)>READ_RSSI_SCAN_TIME_MS) || rssiRead)
        {
          if(rssiRead)
          {
            rssiRead = 0;
          }
          start_time = osKernelGetTickCount();
          if(_iCellular->read_rssi(&rssi) == RET_OK)
          {
              g_cdma_system.rssi = rssi;
          }
        }
    }
}






void at_reboot(uint32_t cmd,char *pData,uint16_t dataLen)
{
  (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    //if(_modem.init)
    {
        s_modem.modemReboot = 1;;
    }
  (void)osSemaphoreRelease(s_modem_sem_id);
}

void at_sms_received(uint32_t cmd,char *pData,uint16_t dataLen)
{
  (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    s_modem.smsRecvCnt++;

  (void)osSemaphoreRelease(s_modem_sem_id);
    
}

void at_async_tcp_disconnected(uint32_t cmd,char *pData,uint16_t dataLen)
{
  (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    s_modem.server_closed =1;

  (void)osSemaphoreRelease(s_modem_sem_id);
  
}

void at_ring_received(uint32_t cmd,char *pData,uint16_t dataLen)
{
    char num[20];
    M_RET_t ret;

    ret = _iCellular->read_ringNum(pData,num,sizeof(num));

  (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    if(ret == RET_OK)
    {
        if(s_modem.ringReceived == 0)
        {
            s_modem.ringReceived = 1;
            strcpy_safe(s_modem.ringNum,sizeof(s_modem.ringNum),num);
        }
    }

    s_modem.ringCnt++;

  (void)osSemaphoreRelease(s_modem_sem_id);

}

/**
 * @brief 시리얼 dtmf 수신 처리
 * @param cmd 현재 미사용
 * @param pData at 
 * @param dataLen at 길이
 */
void at_dtmf(uint32_t cmd,char *pData,uint16_t dataLen)
{
   // char dtmf;

  (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    //dtmf = _iCellular->get_dtmf(pData);//
    
    //DTMF_put_dtmf(dtmf);

  (void)osSemaphoreRelease(s_modem_sem_id);
}

/**
 * @brief 전화끊김 인식후 처리
 * @param cmd 현재 미사용
 * @param pData 수신된 at 
 * @param dataLen 수시된 at 길이
 */
void at_voice_end(uint32_t cmd,char *pData,uint16_t dataLen)
{
  (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);
    s_modem.voiceEnd = 1;
    s_modem.ringReceived = 0;
    s_modem.ringCnt = 0;
    s_modem.dial = 0;
  (void)osSemaphoreRelease(s_modem_sem_id);
}




/**
 * @brief 서버에서 수신된 tcp 데이터를 tcp task 전달
 * @param data tcp 데이터
 * @param dataLen tcp 데이터 길이
 */
void put_tcpData(uint8_t *data, uint16_t dataLen)
{
  #if 0 
    tcpData_t *tcpData = NULL;

    tcpData = (tcpData_t *)osMailAlloc(s_tcp_data_mail_id,1000);

    if(tcpData)
    {
        tcpData->len = dataLen;
        memcpy_safe((uint8_t *)tcpData->data,sizeof(tcpData->data),(uint8_t *)data,dataLen);
        
        osMailPut(s_tcp_data_mail_id,tcpData);
    }
    #else
  tcpData_t tcpData;
  tcpData.len = dataLen;

  memcpy_safe((uint8_t *)tcpData.data,sizeof(tcpData.data),(uint8_t *)data,dataLen);

  if(osMessageQueuePut(s_tcp_data_mail_id, &tcpData, 0, 1000) != osOK)
  {
    dbg_printf("put_tcpData timeout\r\n");
  }

    #endif
}



void put_smsResp(uint32_t cmd,char *pData,uint16_t dataLen)
{

  (void)osSemaphoreAcquire(s_modem_sem_id, RTOSAL_WAIT_FOREVER);

    s_modem.smsSendOk = 1;

  (void)osSemaphoreRelease(s_modem_sem_id);
}




/**
 * @brief tcp 데이터 수신 처리
 * TCP데이터가 수신되면 *TCPRD=4<CR><LF> 형태의 메시지가 수신됨
 * 4는 처리해야할 데이터 길이임
 * @param cmd 명령어 index,현재 미사용
 * @param pData 수신된 AT 데이터
 * @param dataLen 수신된 AT 데이터 길이
 */
void at_async_tcp_recv(uint8_t *p_data,uint16_t data_len)
{
  _iCellular->recv_bin(_iCellular->io_uart, p_data,data_len);
}



/**
 * @brief 모뎀에서 수신되는 AT명령어 처리
 * @param argument task 생성시 매개변수
 */
void modemAtTask(void  *argument)
{
  char buff[512+32];
  int32_t len;
  eAT_COMMAND_t at_cmd;
  uint32_t cmd_count = _iCellular->get_count();

  //wdt_number =  wdt_task_register(kAtTask_attributes.name,600000);
  DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"modemAt task start\r\n");
  while (1)
  {
    ERROR_PRINTF("modem recv start\r\n");
    len = _iCellular->recv_handler(_iCellular->io_uart, (uint8_t *)buff, sizeof(buff));

    if(len<=0)
    {
      continue;
    }

    ERROR_PRINTF("modem recved\r\n");
    //  wdt_task_feed(wdt_number);
    
    for (uint32_t idx = 0; idx < cmd_count; idx++)
    {
      if(strncmp((char *)buff,s_p_atCmd[idx].cmdStr,strlen(s_p_atCmd[idx].cmdStr))==0)
      {
        at_cmd = s_p_atCmd[idx].cmd;

        ERROR_PRINTF("%d %s\r\n", at_cmd, s_p_atCmd[idx].cmdStr);
        switch (at_cmd)
        {
          case AT_ASYNC_RECV_REBOOT:
              at_reboot(idx, buff, len);  // 모뎀이 리셋되었다는 부팅 메시지를 받음
            break;
            case AT_ASYNC_RECV_SMS:
                at_sms_received(idx,buff,len);//SMS가 수시되었다는 알림을 받음
            break;
            case AT_ASYNC_RECV_RING:
                at_ring_received(idx,buff,len);//전화 수신되었다는 메시지를 받음
            break;
            case AT_ASYNC_RESP_VOICE_END:
                  at_voice_end(idx,buff,len);// 전화가 끊겼다는 메시지를 받음
            break;
            case AT_ASYNC_RECV_DTMF://DTMF를 받음
                at_dtmf(idx,buff,len);
            break;
            case AT_ASYNC_RECV_TCP_DATA://tcp data를 받음
                at_async_tcp_recv((uint8_t *)buff,len);
            break;
            case AT_ASYNC_RECV_TCP_DISCONNECTED://tcp 가 끊겼다는 메시지를 받음
                at_async_tcp_disconnected(idx,buff,len);
            break;
            case AT_SYNC_SMS_READ_RESP_OK:  // SMS 읽기에 대한 응답
              if (_iCellular->sms_handler)
              {
                _iCellular->sms_handler(_iCellular->io_uart, buff, len);
                break;
              }
            case AT_ASYNC_OPEN_VOICE_RESP:  // 전화가 연결되었는지 응답
            case AT_SYNC_GET_RSSI_RESP:    // 수신감도 명령어에 대한 응답

            case AT_ASYNC_SMS_READ_RESP_ERR:// SMS 읽기 에러에대한 응답
            case AT_ASYNC_DIAL_RESP:
                put_asyncResp(idx,buff,len);
                break;
            case AT_SMS_SEND_RESP:        // SMS 전송에대한 응답
                put_smsResp(idx,buff,len);
                break;
            case AT_TCP_WRITE_IP_RESP:
            case AT_TCP_SEND_DATA_RESP:
            case AT_TCP_OPEN_SOCKET_RESP_OK:
            case AT_TCP_OPEN_SOCKET_RESP_FAIL:
            case AT_TCP_OPEN_SOCKET_RESP:
            case AT_TCP_READ_NUM_RESP:
            case AT_TCP_CONNECT_VPN_RESP:
            case AT_TCP_OPEN_PPP_RESP:
            case AT_TCP_CLOSE_PPP_RESP:
            case AT_TCP_CLOSE_SOCKET_RESP:
            case AT_RESET_SW_RESP:
            case AT_TCP_NETWORK_SERVICE:
                put_tcpResp(idx,(uint8_t*)buff,len);
                break;
            
          }
        break;
     }
    }
  }
    
}




/**
* @brief 셀룰러 인터페이스 초기화
*/
void iCellular_init(void)
{
  uart_config_t uart_config;

  int cdma_uart;

 
  
  uart_config.dataLen = UART_DATA_LEN_8;
  uart_config.baud = 57600;
  uart_config.parity_index = 0;
  uart_config.stop_bit = 0;

  cdma_uart = BSP_UART_8_CDMA;
  
  bsp_uart_init(BSP_UART_8_CDMA, &uart_config);

  _iCellular = &g_iCellular;


  _iCellular->io_uart = cdma_uart;


  switch (get_config_app()->cdma_model)
  {
    case eCDMA_NTLE9607:

    s_p_atCmd = cmd_ntle9607;
    _iCellular->resetDelay = 20000;
  _iCellular->init       = ntle9607_init;
  _iCellular->read_sms   = ntle9607_read_sms;
  _iCellular->send_sms   = ntle9607_send_sms;
  _iCellular->write_ip   = ntle9607_write_ip;
  _iCellular->open_ppp   = ntle9607_open_ppp;
  _iCellular->close_ppp  = ntle9607_close_ppp;
  _iCellular->open_tcp   = ntle9607_open_socket;
  _iCellular->close_tcp  = ntle9607_close_socket;
  _iCellular->reset      = ntle9607_reset;
  _iCellular->recv_tcp   = ntle9607_recv_tcp;
  _iCellular->send_tcp   = ntle9607_send_tcp;
  _iCellular->read_num   = ntle9607_read_num;
  _iCellular->read_rssi  = ntle9607_read_rssi;
  _iCellular->get_dtmf   = ntle9607_get_dtmf;
  _iCellular->vpn_init   =  ntle9607_vpn_init;
  _iCellular->off_powerSafe   = ntle9607_off_powerSafe;
  _iCellular->read_ringNum    = ntle9607_read_ringNum;
  _iCellular->recv_call       = ntle_9607_recv_call;
  _iCellular->dial            = ntle_9607_dial;
  _iCellular->set_vpn_config  = ntle_9607_set_vpn;
  _iCellular->read_vpn_config = ntle_9607_read_vpn;
  _iCellular->at_direct       = ntle_9607_at_direct;
  _iCellular->check_network_service = ntle9607_check_network_service;
  _iCellular->recv_bin = ntle9607_recv_bin;
  _iCellular->get_count = ntle9607_get_count;
  _iCellular->recv_handler= ntle9607_recv_handler;
  _iCellular->sms_handler = NULL;
  break;
  case eCDMA_TX700:
   s_p_atCmd = cmd_tx700;
    _iCellular->resetDelay = 20000;
    _iCellular->init = tx700_init;
    _iCellular->read_sms = tx700_read_sms;
    _iCellular->send_sms = tx700_send_sms;
    _iCellular->write_ip = tx700_write_ip;
    _iCellular->open_ppp = tx700_open_ppp;
    _iCellular->close_ppp = tx700_close_ppp;
    _iCellular->open_tcp = tx700_open_socket;
    _iCellular->close_tcp = tx700_close_socket;
    _iCellular->reset = tx700_reset;
    _iCellular->recv_tcp = tx700_recv_tcp;
    _iCellular->send_tcp = tx700_send_tcp;
    _iCellular->read_num = tx700_read_num;
    _iCellular->read_rssi = tx700_read_rssi;
    _iCellular->get_dtmf = tx700_get_dtmf;
    _iCellular->vpn_init = NULL;
    _iCellular->off_powerSafe = tx700_off_powerSafe;
    _iCellular->read_ringNum = NULL;
    _iCellular->recv_call = NULL;
    _iCellular->dial = tx700_dial;
    _iCellular->set_vpn_config = NULL;
    _iCellular->read_vpn_config = NULL;
    _iCellular->at_direct = tx700_at_direct;
    _iCellular->check_network_service = tx700_check_network_service;
    _iCellular->recv_bin = tx700_recv_bin;
    _iCellular->get_count = tx700_get_count;
    _iCellular->recv_handler = tx700_recv_handler;
    _iCellular->sms_handler = tx700_sms_handler;
    
    break;
  }
  
}



/**
 * @brief tcp 통신
 * @param argument task 매개변수
*/
void modemTcpTask(void  *argument)
{
    uint8_t buff[512+32];//tcp data 512 + 기타
    uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
    uint8_t err=0;
    uint16_t len;
    uint16_t rtu_id;
    uint32_t start_time=0;
    M_RET_t ret;
    eConnect_Type_t type = ePOWER_RESET;//초기에는 전원리셋이 발생하였다고넘겨줌

    g_cdma_system.link_status = eCDMA_LINK_IDLE;
    g_modem_config.connection_timeoutms = 3600000;

  DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"modemTcp task start\r\n");
    while (1)
    {
      g_cdma_system.link_status = eCDMA_LINK_DOWN;

      if (connect_tcp(type) == STATUS_OK)
      {
        rtu_id = swap_uint16(get_config_app()->id);
        _iCellular->send_tcp((uint8_t *)&rtu_id, 2);

        start_time = osKernelGetTickCount();
        g_cdma_system.link_status = eCDMA_LINK_UP;

        err = 0;

        while (1)
        {
          ret = _iCellular->recv_tcp(buff, sizeof(buff), &len, 0);

          switch (ret)  // 통신 이상 없음
          {
            case RET_OK:
              if (len)  // 수신된 데이터가 있음
              {
                start_time = osKernelGetTickCount();
                g_cdma_system.last_recv_time = time_timestamp();
                UPDATE_CNT(g_cdma_system.rx_cnt, 99);
                len = kma_cmd_handler(buff, len, tx_buffer, eREQ_SOURCE_CDMA);

                if (len)  // 전송할 데이터있다면
                {
                  g_cdma_system.last_send_time = time_timestamp();
                  UPDATE_CNT(g_cdma_system.tx_cnt, 99);
                  if (_iCellular->send_tcp(tx_buffer, len) == RET_FAIL_SEND)
                  {
                    err = 1;
                    type = eCONNECT_TX_FAIL;
                  }

                  if (get_firmware_update())
                  {
                    _iCellular->close_tcp();
                    osDelay(1000);
                    reset_system( "CDMA update");
                  }
                }
              }
              break;
            case RET_SERVER_ERR:
              type = eCONNECT_SOCKET_CLOSED;
              err = 1;
              break;
            case RET_MODEM_ERR:
              type = eCONNECT_MODEM_REBOOT;
              err = 1;
              break;
          }

          if (err)
          {
            break;
          }

          if ((osKernelGetTickCount() - start_time) >
              g_modem_config.connection_timeoutms) /*일정 기간동안 ping이 한번이라도 수신 안되면*/
          {
            type = eCONNECT_TCP_WDT;
            break;
          }

          if (is_cdma_retarget())
          {
            type = eCONNECT_CHANG_IP_REQ;
            break;
          }
        }
        }
    }
}

cdma_system_t *get_cdma_system(void)
{
  return &g_cdma_system;
}


void mdoem_status_init(void)
{
  g_cdma_system.rssi = -1;
  g_cdma_system.link_status = eCDMA_LINK_IDLE;
 
}


void cellularTask_init(void)
{
  iCellular_init();

  mdoem_status_init();
 
  s_modem_sem_id = osSemaphoreNew(1, 1, NULL); 

  s_resp_async_mail_id = osMessageQueueNew(MODEM_RESP_MAIL_SIZE, sizeof(resp_async_t), NULL);
  s_resp_tcp_mail_id = osMessageQueueNew(1, sizeof(resp_tcp_t), NULL);
  s_tcp_data_mail_id = osMessageQueueNew(1, sizeof(tcpData_t), NULL);

  osThreadNew(modemAtTask   ,NULL,&kAtTask_attributes);
  osThreadNew(modemTcpTask  ,NULL,&kTcpTask_attributes);
  osThreadNew(modemAsyncTask,NULL,&kAsyncTask_attributes);

}

