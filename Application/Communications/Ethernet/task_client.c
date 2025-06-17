#include "app_logging.h"
#include "cmsis_os.h"
#include "config_app.h"
#include "dev_io.h"
#include "kma_protocol_handler.h"
#include "lwip.h"
#include "sockets.h"
#include "task_logging.h"
#include "tcp_define.h"
#include "update_fw.h"
#include "util_time.h"

#define RECV_BUFF_SIZE 512
#define SERVER_RETRY_INTERVAL_MS 5000
#define CLIENT_CONNECT_TIMEOUT_MS 10000

tcp_system_t g_tcp_client_status;
osThreadId_t g_tcpClientTaskId;

const osThreadAttr_t tcpClientTask_attributes = {
  .name = "tcp_client",
  .stack_size = 4096,
  .priority = (osPriority_t) osPriorityNormal,
};

tcp_system_t *get_tcp_client_system(void) 
{ 
  return &g_tcp_client_status; 
}


void noti_tcpClientTask(uint32_t flag)
{
  if(g_tcpClientTaskId)
  {
  osThreadFlagsSet(g_tcpClientTaskId,  flag);
  }
}

static int set_recv_timeout(int sockfd, uint32_t timeout_ms)
{
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

static void tcp_client_service(int sock)
{
  uint8_t rbuffer[RECV_BUFF_SIZE];
  uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
  int32_t ret, len, err_code;

  if (set_recv_timeout(sock, CLIENT_CONNECT_TIMEOUT_MS) < 0)
  {
    task_printf("타임아웃 설정 실패\r\n");
    return;
  }

  while (1)
  {
    ret = recv(sock, rbuffer, sizeof(rbuffer), 0);
    
    if(ret < 0)
    {
      err_code = errno;
      if (err_code == EAGAIN )//|| err_code == EWOULDBLOCK)
      {
        continue;//타임아웃
      }
      else
      {
        task_printf("recv error on socket %d, errno: %d\r\n", sock, err_code);
        break;
      }


    }
    else if(ret ==0)
    {
      task_printf("Client: Connection closed by peer on socket %d\r\n", sock);
      break;  
    }
    else
    {
     
      UPDATE_CNT(g_tcp_client_status.rx_cnt, 99);
      g_tcp_client_status.last_recv_time = time_timestamp();
      len = kma_cmd_handler(rbuffer, ret, tx_buffer, eREQ_SOURCE_ETH);
      if (len > 0)
      {
        int32_t total=0;
        bool send_error=false;

        while(total< len)
        {
          ret = send(sock, tx_buffer+total, len-total, 0);
          if (ret <= 0)
          {
            task_printf("전송 실패 errno=%d\r\n", errno);
            send_error = true;
            break;
          }
          else
          {
            total += ret;
            g_tcp_client_status.last_send_time = time_timestamp();
            UPDATE_CNT(g_tcp_client_status.tx_cnt, 99);
            if (get_firmware_update())
            {
              closesocket(sock);
              reset_system( "TCP client update");
            }
          }
        }
       if (send_error)
       {
         break;
       }
      }
    }
  }

  // 연결 종료 상태로 갱신
  g_tcp_client_status.link_status = eLINK_DOWN;
}

void tcpClientTask(void *arg)
{
  char server_ip[20];
    struct sockaddr_in server_addr;
    int sock;
    const config_t *config = get_config_app();


   osThreadFlagsWait(0x00000001, osFlagsWaitAny, osWaitForever);
   
   
    g_tcp_client_status.link_status = eLINK_IDLE;

    while (1)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            osDelay(SERVER_RETRY_INTERVAL_MS);
            continue;
        }

        snprintf(server_ip,sizeof(server_ip),"%d.%d.%d.%d",config->eth_remote_server_ip[0],
                 config->eth_remote_server_ip[1],
                 config->eth_remote_server_ip[2],
                 config->eth_remote_server_ip[3]);
                 
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(config->eth_remote_server_port);
        server_addr.sin_addr.s_addr = inet_addr(server_ip); // example: "192.168.0.10"

        task_printf("서버 연결 시작 %s:%d...\r\n", server_ip, config->eth_remote_server_port);

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        {
          task_printf("연결 실패 재시도\r\n");
          closesocket(sock);
          osDelay(SERVER_RETRY_INTERVAL_MS);
          continue;
        }

        task_printf("연결 성공\r\n");
        g_tcp_client_status.link_status = eLINK_UP;
        tcp_client_service(sock);
        g_tcp_client_status.link_status = eLINK_IDLE;

        closesocket(sock);
        osDelay(SERVER_RETRY_INTERVAL_MS);
    }
}

void tcpClientTask_init(void)
{
    g_tcpClientTaskId = osThreadNew(tcpClientTask, NULL, &tcpClientTask_attributes);
}
