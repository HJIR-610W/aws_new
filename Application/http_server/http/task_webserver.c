

#include <stdint.h>
#include <time.h>
#include <errno.h>

#include "AppShRAM.h"
#include "AppEth.h"
#include "board_rtcc.h"
#include "board_time.h"
#include "cli_cmd.h"
#include "cmsis_os.h"
#include "ewrte.h"
#include "http_websocketServer.h"
#include "manageSystem.h"
#include "sockets.h"
#include "systemLogging.h"
#include "task_webserverService.h"
#include "task_sriverAPI.h"



extern void enable_keepalive(int sock, int idle_time, int interval, int max_probes);


#define WEB_SERVER_PORT 8080

osThreadId g_webserverTaskId = NULL;

/**
 * @brief 이더넷이 초기화 되었음을 알림
 */
void noti_webserverNetAvailabe(void)
{
  if(g_webserverTaskId)
  {
  osSignalSet(g_webserverTaskId,ETH_INIT_OK);//이더넷 초기화 완료 알림
  }
}



void webserverTask(void const *argument)
{
  int32_t opt=1;
  int32_t sock;
  int32_t newconn, size;
  uint32_t port = (uint32_t)argument;
  struct sockaddr_in address, remotehost,oldClient;
  int error = 0;
  socklen_t len = sizeof(error);
  char client_ip[INET_ADDRSTRLEN];

  //이더넷이 초기화 될때까지 대기 
  osSignalWait(ETH_INIT_OK, osWaitForever);

  do
  {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sock < 0) 
    {//옵션에서 최대사용 가능한 socket이 전부 사용중인 경우,또는 기타 이유
      osDelay(1000);
      continue;
    }

    // 이미 사용중인 로컬 주소(포트)를 재사용 할 수 있도록 허용
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
      closesocket(sock);
      osDelay(1000);
      continue;
    }
    

    if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
    {
      closesocket(sock);
      osDelay(1000);
      continue;
    }
  



    address.sin_family = AF_INET;
    address.sin_port   = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
    {
      closesocket(sock);
      osDelay(1000);
      continue;
    }
  
    if(listen(sock, 1) < 0)
    {
      closesocket(sock);
      osDelay(1000);
      continue;
    }
    size = sizeof(remotehost);

    while(1)
    {
      newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);

      //클라리언트 접속 기록 저장
      if(oldClient.sin_addr.s_addr != remotehost.sin_addr.s_addr&&oldClient.sin_port != remotehost.sin_port )
      {
        inet_ntop(AF_INET, &remotehost.sin_addr, client_ip, sizeof(client_ip)); 
        logging_write(LOG_CODE_WEB, "Client:%s,%d",client_ip,ntohs(remotehost.sin_port));
      }
      
      oldClient = remotehost;
      enable_keepalive(newconn,60000,60000,2);

      if(newconn==-1)
      {
        break;
      }

      http_server_service(newconn);
      //create_webserverService((void *)newconn);//메모리 부족해서 task실행 못함 오직 1개만 접속
      closesocket(newconn);

    }
    
    closesocket(sock);//103 ECONNABORTED
    
       
  }while(1);
}



/**
 * @brief http 서버를 실행한다.
 * 
 */
void webserverTask_init(void)
{
  create_responseMsgQ();//웹서버내에서 주고받을 메시지 처리용
  osThreadDef(webserverTask, webserverTask, TASK_PRIORITY_WEBSERVER, 0, TASK_STACK_SIZE_WEBSERVER);
  g_webserverTaskId = osThreadCreate(osThread(webserverTask), (void *)WEB_SERVER_PORT);
}