
#include "aws_protocol.h"
#include "cmsis_os.h"
#include "config.h"
#include "driver_rtc.h"
#include "io.h"
#include "lwip.h"
#include "utile_time.h"
#include "sockets.h"
#include "app_logging.h"
#include "app_socket.h"

osThreadId_t g_tcpSeverTaskId;

const osThreadAttr_t tcpServerTask_attributes = {
  .name = "tcpServerTask",
  .stack_size = 2048,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
  .priority = (osPriority_t) osPriorityNormal,
};

void noti_tcpServerTask(uint32_t flag)
{
  osThreadFlagsSet(g_tcpSeverTaskId,  flag);
}

int set_recv_timeout(int sockfd, uint32_t timeout_ms)
{
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
      //  perror("Failed to set SO_RCVTIMEO");
      return -1;
    }
    
    return 0;
}

#define RECV_BUFF_SIZE 512
void server_service(int conn)
{
  int32_t ret;
  int32_t err;
  uint8_t rbuffer[RECV_BUFF_SIZE];
  uint8_t tbuffer[RECV_BUFF_SIZE];
  int32_t len;

    if(set_recv_timeout(conn,60000)<0)
    {
      return;
    }

  while(1)
  {
     ret = recv(conn, rbuffer, RECV_BUFF_SIZE, 0);
    if(ret <= 0)
    {
      err = errno;
      if(err == EWOULDBLOCK || err == EAGAIN)
      {
        continue;
      }

      return;
    }

     len = aws_cmd(rbuffer,ret,tbuffer,sizeof(tbuffer),0);
     if(len)
     {
      send(conn,tbuffer,len,0);
     }
  }
  
}


void tcpServerTask(void *arg)
{
    int32_t opt=1;
    int32_t sock;
      int32_t newconn, size;
    struct sockaddr_in address, remotehost,oldClient;
      int error = 0;
  socklen_t len = sizeof(error);
  uint32_t flag =(uint32_t)arg;
  char client_ip[INET_ADDRSTRLEN];

  osThreadFlagsWait(0x00000001,osFlagsWaitAny,osWaitForever);

  while(1)
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
    address.sin_port   = htons(8080);
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
        logging_printf(0, "Client:%s,%d",client_ip,ntohs(remotehost.sin_port));
      }
      
      oldClient = remotehost;
      enable_keepalive(newconn,60000,60000,2);

      if(newconn==-1)
      {
        break;
      }

      server_service(newconn);

      closesocket(newconn);

    }
    
    closesocket(sock);//103 ECONNABORTED

  }
}


void tcpServerTask_init(uint32_t flag)
{
  g_tcpSeverTaskId = osThreadNew(tcpServerTask, NULL, &tcpServerTask_attributes);
}
