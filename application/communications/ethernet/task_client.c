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
#include "os_user_def.h"

#define RECV_BUFF_SIZE 512
#define SERVER_RETRY_INTERVAL_MS 5000
#define CLIENT_CONNECT_TIMEOUT_MS 1000
#ifndef SERVER_REQ_TIMEOUT
#define SERVER_REQ_TIMEOUT 80000
#endif

tcp_system_t g_tcp_client_status;
osThreadId_t g_tcpClientTaskId;

const osThreadAttr_t tcpClientTask_attributes = {
  .name = "tcp_client",
  .stack_size = TASK_STACK(TASK_TCP_CLIENT_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_TCP_CLIENT_DEF),
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

extern uint8_t g_ethernet_phy_link;
static void tcp_client_service(int sock)
{
  uint8_t rbuffer[RECV_BUFF_SIZE];
  uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
  int32_t ret, len, err_code;
  uint16_t rtu_id;
  uint32_t start_tkme;
  rtu_id = swap_uint16(get_config_app()->id);

  send(sock, &rtu_id,2, 0);

  if (set_recv_timeout(sock, CLIENT_CONNECT_TIMEOUT_MS) < 0)
  {
    task_printf("타임아웃 설정 실패\r\n");
    return;
  }

  start_tkme = OS_GET_TICK();
  while (1)
  {
    ret = recv(sock, rbuffer, sizeof(rbuffer), 0);

    if (g_ethernet_phy_link == 0 )//link down
    {
     // break;
    }
      if (ret < 0)
      {

        if ((OS_GET_TICK() - start_tkme) > SERVER_REQ_TIMEOUT)
        {
          break;
        }
        err_code = errno;
        if (err_code == EAGAIN) //|| err_code == EWOULDBLOCK)
        {
          continue; // 타임아웃
        }
        else
        {
          task_printf("recv error on socket %d, errno: %d\r\n", sock, err_code);
          break;
        }
      }
      else if (ret == 0)
      {
        task_printf("Client: Connection closed by peer on socket %d\r\n", sock);
        break;
      }
      else
      {
        start_tkme = OS_GET_TICK();

        UPDATE_CNT(g_tcp_client_status.rx_cnt, 99);
        g_tcp_client_status.last_recv_time = time_timestamp();
        len = kma_cmd_handler(rbuffer, ret, tx_buffer, eREQ_SOURCE_ETH);
        if (len > 0)
        {
          int32_t total = 0;
          bool send_error = false;

          while (total < len)
          {
            ret = send(sock, tx_buffer + total, len - total, 0);
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
                reset_system("TCP client update");
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


/**
 * @brief 비차단모드로 connect하고 바로 연결이 안되면 타임아웃안에 연결 시도
 * 연결되면 차단모드로 변경되어 recv,send가 차단모드로 동작하도록함함
 */
int32_t connect_with_timeout(int sock, const struct sockaddr *addr, socklen_t addrlen, int timeout_ms)
{
    // 1. 소켓을 비차단 모드로 설정,connet, recv,send가 즉시 반환되도록 설정
    int32_t flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
       // perror("fcntl get failed");
        return -1;
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
       // perror("fcntl set failed");
        return -1;
    }
    // 2. lwip_connect 호출, 즉시 연결되지 않더라도 바로 리턴턴
    int32_t result = connect(sock, addr, addrlen);
    if (result == 0)
    {
        // 즉시 연결 성공
        fcntl(sock, F_SETFL, flags); // 원래 모드로 복구
        return 0;
    }
    else if(errno != EINPROGRESS)
    {
      // EINPROGRESS가 아닌 경우 실제 오류
       // perror("lwip_connect failed");
        return -1;
    }

//여기까지 왔다는건 서버연결이 안되었다는것

    // 3. select로 타임아웃 처리
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);

    result = select(sock + 1, NULL, &writefds, NULL, &tv);
    if (result > 0) {
        // 소켓이 쓰기 가능 상태인지 확인
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error == 0) {
            fcntl(sock, F_SETFL, flags); // 원래 모드로 복구
            return 0; // 연결 성공
        } else {
            errno = so_error;
           // perror("connect failed");
            return -1;
        }
    } else if (result == 0) {
        // 타임아웃 발생
        errno = ETIMEDOUT;
      //printf("Connection timed out\n");
        return -1;
    }else{
        // select 오류
      //perror("select failed");
        return -1;
    }
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
        sock = socket(PF_INET, SOCK_STREAM, 0);
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
        server_addr.sin_addr.s_addr = inet_addr(server_ip); 

        task_printf("서버 연결 시작 %s:%d\r\n", server_ip, config->eth_remote_server_port);

        if (connect_with_timeout(sock, (struct sockaddr *)&server_addr, sizeof(server_addr), 2000) != 0) 
        {
          task_printf("연결 실패 재시도\r\n");
          closesocket(sock);
          osDelay(SERVER_RETRY_INTERVAL_MS);
          continue;
        }

        task_printf("연결 성공\r\n");
        g_tcp_client_status.link_status = eLINK_UP;
        tcp_client_service(sock);
        g_tcp_client_status.link_status = eLINK_DOWN;

        closesocket(sock);
        osDelay(SERVER_RETRY_INTERVAL_MS);
    }
}

void tcpClientTask_init(void)
{
    g_tcpClientTaskId = osThreadNew(tcpClientTask, NULL, &tcpClientTask_attributes);
}
