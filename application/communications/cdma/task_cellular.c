
#include "task_cellular.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <string.h>

#include "util_memory.h"
#include "cellular_api.h"
#include "at_parser.h"
#include "system_err.h"

#include "config_app.h"
#include "protocols\aws\kma_protocol_handler.h"
#include "sms_handler.h"
#include "util_safe.h"

/* FSM 상태 */
typedef enum {
    APP_STATE_HW_RESET,
    APP_STATE_SW_RESET,      // 모뎀 리셋    
    APP_STATE_WAIT_FOR_BOOT, // 부팅감지   
    APP_STATE_INIT_MODEM,    //모뎀 초기화,에코끄기 등 
    APP_STATE_VPN_LOGIN,     // VPN 로그인
    APP_STATE_CONNECT_TCP,     // TCP 연결
    APP_STATE_COMMUNICATE,     // 데이터 송수신
    eAPP_STATE_ERROR            
} eAPP_STATE_t;


static osEventFlagsId_t s_app_event_flags;
static osTimerId_t s_connect_wdt_timer;
static osTimerId_t s_network_wdt_timer;
static osTimerId_t s_rssi_timer;

cdma_system_t g_cdma_system;
static uint8_t s_cdma_retarget_ip[4];
static uint16_t s_cdma_retarget_port=0;
static bool  s_cdma_retarget=false;



#define EVT_FLAG_MODEM_REBOOTED (1 << 0)
#define EVT_FLAG_TCP_DISCONNECTED (1 << 1) 
#define EVT_FLAG_CONNECTION_TIMEOUT (1 << 2) 
#define EVT_FLAG_CONNECT_TIMEOUT (1 << 3) 





static void dispatcher_urc_reboot_cb(const uint8_t* data, size_t len, void* ctx);
static void dispatcher_urc_tcp_disconnected_cb(const uint8_t* data, size_t len, void* ctx);
static void connect_wdt_callback(void *arg);
static void network_wdt_callback(void *arg);
static void ring_callback_task(void *arg);
static void sms_callback_task(void *arg);
static void dispatcher_urc_dtmf_cb(const uint8_t* data, size_t len, void* ctx);
static int32_t connect_tcp(eAPP_STATE_t state);

static void ring_callback_task(void* arg)
{
  const char* payload = (const char*)arg;
   DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[RING] %s\r\n", payload ? payload : "(null)");
  cellular_recv_call();
}


static void sms_callback_task(void* arg)
{
  sms_t sms;
  const char* payload = (const char*)arg; 
   DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[SMS] %s\r\n", payload ? payload : "(null)");
  if (cellular_read_sms(&sms) == 0)
  {
    sms_cmd(&sms);
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  From: %s, Message: %s\r\n", sms.number, sms.message);
  }
}




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

void get_ip(uint8_t *ip,uint16_t *port)
{
 ip[0] = config.cdma_server_ip[0];
 ip[1] = config.cdma_server_ip[1];
 ip[2] = config.cdma_server_ip[2];
 ip[3] = config.cdma_server_ip[3];

 *port = config.cdma_port;

}

int32_t connect_tcp(eAPP_STATE_t state)
{
  uint32_t flags;
  uint32_t clear_mask = 0;
  eAPP_STATE_t local_state = state;
  uint32_t connect_wtd_starttime = 0;
  int32_t ret;
  bool hw_reset_done = false;
  bool sw_reset_done = false;
  uint8_t ip[4];
  uint16_t port;

  while (1)
  {
    flags = osEventFlagsGet(s_app_event_flags);
    if (flags & EVT_FLAG_MODEM_REBOOTED)
    {
       DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"모뎀 리부팅됨\r\n");
      local_state = APP_STATE_INIT_MODEM;
      clear_mask |= EVT_FLAG_MODEM_REBOOTED;
    }
    if (flags & EVT_FLAG_TCP_DISCONNECTED)
    {
       DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"소켓 닫힘\r\n");
      local_state = APP_STATE_CONNECT_TCP;
      clear_mask |= EVT_FLAG_TCP_DISCONNECTED;
    }
    if (clear_mask != 0)
    {
      clear_mask = 0;
      osEventFlagsClear(s_app_event_flags, clear_mask);
    }

  switch (local_state)
  {
  case APP_STATE_HW_RESET:
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] State: APP_STATE_HW_RESET\r\n");
    cellular_reset_hw();
    local_state = APP_STATE_WAIT_FOR_BOOT;
    break;
  case APP_STATE_SW_RESET:
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] State: APP_STATE_SW_RESET\r\n");
    cellular_reset_sw();
    local_state = APP_STATE_WAIT_FOR_BOOT;
    break;
  case APP_STATE_WAIT_FOR_BOOT:
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] State: APP_STATE_WAIT_FOR_BOOT\r\n");
    if (osErrorTimeout != (int32_t)osEventFlagsWait(s_app_event_flags, EVT_FLAG_MODEM_REBOOTED |
      EVT_FLAG_TCP_DISCONNECTED, osFlagsWaitAny, 30000))
    {
       DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] 부팅 감지 신호 수신완료\r\n");
    }
    else
    {
       DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] 부팅 감지 타임 아웃\r\n");
    }

    osDelay(5000);//tx700의 경우 부팅감지되고 약간의 시간이 더 지나야 전화 번호 읽기 가능
    local_state = APP_STATE_INIT_MODEM;
    
    break;
  case APP_STATE_INIT_MODEM:
  {
    char buffer[20];
    int16_t rssi;
    cellular_init();
    for (int i = 0; i < 3; i++)
    {
      if (cellular_read_num(buffer, sizeof(buffer)) == 0)
      {
         strcpy_safe(g_cdma_system.num,sizeof(g_cdma_system.num),buffer);
         DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"전화번호:%s\r\n", buffer);
        break;
      }
      osDelay(5000);
    }

    if (cellular_read_rssi(&rssi) == 0)
    {
       DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"RSSI:%d \r\n", rssi);
       g_cdma_system.rssi = rssi;
    }

    local_state = APP_STATE_VPN_LOGIN;
  }
  break;
  case APP_STATE_VPN_LOGIN:
    local_state = APP_STATE_CONNECT_TCP;
    connect_wtd_starttime = osKernelGetTickCount();
    break;
  case APP_STATE_CONNECT_TCP:
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] State: APP_STATE_CONNECT_TCP\r\n");
    cellular_disconnect();
    osDelay(2000);

    if(is_cdma_retarget())
    {
      get_cdma_retarget_ip(ip, &port);
      set_cdma_retarget(false);

    }
    else
    {
      get_ip(ip, &port);
    }

    ret = cellular_connect(ip, port);
    if (ret == 0)
    {
       DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] 서버 연결됨\r\n");
      osEventFlagsClear(s_app_event_flags, EVT_FLAG_MODEM_REBOOTED| EVT_FLAG_TCP_DISCONNECTED);
      goto LOOP_END;
    }

    if ((osKernelGetTickCount() - connect_wtd_starttime) > (60 * 1000))
    {
       DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] 서버 연결 타임아웃\r\n");
      if (sw_reset_done == false)
      {
        local_state = APP_STATE_SW_RESET;
        sw_reset_done = true;
      }
      if (hw_reset_done == false)
      {
        local_state = APP_STATE_HW_RESET;
        hw_reset_done = true;
      }

    }
    break;
    }
  }


LOOP_END:
  return 0;
}


void cellular_task(void *arg)
{
  uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
  uint8_t rx_buffer[512+32];
  int32_t ret;
  uint32_t network_wtd_starttime=0;
  eAPP_STATE_t state=APP_STATE_HW_RESET;
  uint32_t flags;
  uint32_t clear_mask = 0;
  int32_t len;
   uint16_t  rtu_id ;//= swap_uint16(get_config_app()->id);


  switch(config.cdma_model)
  {
    case eCDMA_NTLE9607:
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"NTLE9607 모뎀 사용\r\n");
          cellular_open(NTLE9607_MODEM);
      break;
    case eCDMA_TX700:
    default:
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"TX700 모뎀 사용\r\n");
    cellular_open(TX700_MODEM);
      break;
  }


    s_app_event_flags = osEventFlagsNew(NULL);
    if (s_app_event_flags == NULL)
    {
        debug_printf("[AppMain] 플래그 생성 실패\r\n");
        while (1)
        {
          osDelay(1000);
        }
    }

    cellular_set_reboot_callback(dispatcher_urc_reboot_cb);
    cellular_set_disconnect_callback(dispatcher_urc_tcp_disconnected_cb);
    cellular_set_dtmf_callback(dispatcher_urc_dtmf_cb);
    cellular_set_call_callback(ring_callback_task);
    cellular_set_sms_callback(sms_callback_task);

    while (1)
    {
      g_cdma_system.link_status = eCDMA_LINK_DOWN;
      connect_tcp(state);
      g_cdma_system.link_status = eCDMA_LINK_UP;
      network_wtd_starttime = osKernelGetTickCount();
      rtu_id = swap_uint16(config.id);
      cellular_send_tcp((uint8_t *)&rtu_id, 2);//AWS는 아이디 전송해야 수신측에서 AWS id로 인식 처리 
      
        while (1)
        {
          flags = osEventFlagsGet(s_app_event_flags);
          if (flags & EVT_FLAG_MODEM_REBOOTED)
          {
             DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"모뎀 리부팅됨\r\n");
            clear_mask |= EVT_FLAG_MODEM_REBOOTED;
          }
          if (flags & EVT_FLAG_TCP_DISCONNECTED)
          {
             DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"소켓 닫힘\r\n");
            clear_mask |= EVT_FLAG_TCP_DISCONNECTED;
          }
          if (clear_mask != 0)
          {
            osEventFlagsClear(s_app_event_flags, clear_mask);
            clear_mask = 0;
            state = APP_STATE_CONNECT_TCP;
            break;
          }


          ret = cellular_recv_tcp(rx_buffer, sizeof(rx_buffer) - 1, 5000);

          if (ret > 0)
          {
              network_wtd_starttime = osKernelGetTickCount();
              g_cdma_system.last_recv_time = time_timestamp();
              UPDATE_CNT(g_cdma_system.rx_cnt, 99);
              
              len = kma_cmd_handler(rx_buffer, ret, tx_buffer, eREQ_SOURCE_CDMA);
              if (len > 0)
              {
                  g_cdma_system.last_send_time = time_timestamp();
                  UPDATE_CNT(g_cdma_system.tx_cnt, 99);
               len =  cellular_send_tcp(tx_buffer, len);
              if(len<0)
              {
                DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[CellConn] 처리할 명령어 없음\r\n");
                state = APP_STATE_CONNECT_TCP;
                break;
              }
              
              }
              
    
           }

          if ((osKernelGetTickCount() - network_wtd_starttime) > (180 * 1000))
          {
             DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[AppMain] 유령 세션 연결 타임아웃\r\n");
            state = APP_STATE_CONNECT_TCP;
            break;

          }

          if(is_cdma_retarget())
          {
            state = APP_STATE_CONNECT_TCP;
            break;
          }
        }

    }
}


static void dispatcher_urc_reboot_cb(const uint8_t* data, size_t len, void* ctx)
{
  (void)ctx;
  (void)data;
  (void)len;

   DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[URC] *REBOOT URC 수신됨\r\n");
  osEventFlagsSet(s_app_event_flags, EVT_FLAG_MODEM_REBOOTED);
}

static void dispatcher_urc_tcp_disconnected_cb(const uint8_t* data, size_t len, void* ctx)
{
    (void)ctx;
    (void)data;
    (void)len; 

     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[URC] *TCPDISCONNECTED URC 수신됨\r\n");
    osEventFlagsSet(s_app_event_flags, EVT_FLAG_TCP_DISCONNECTED);
}
static void dispatcher_urc_dtmf_cb(const uint8_t* data, size_t len, void* ctx)
{
   DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[URC] *DTMF URC 수신됨: %.*s\r\n", len, data);
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


const osThreadAttr_t kCellulaTask_attributes = {
  .name = "cellular",
  .stack_size = TASK_STACK(TASK_CELLULAR_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_CELLULAR_DEF),
};
void cellularTask_init(void)
{
  mdoem_status_init();
  osThreadNew(cellular_task, NULL, &kCellulaTask_attributes);
}