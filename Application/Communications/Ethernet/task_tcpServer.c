#if 0 

#include "cmsis_os.h"
#include "config_app.h"
#include "driver_rtc.h"
#include "dev_io.h"
#include "lwip.h"
#include "utile_time.h"
#include "sockets.h"
#include "app_logging.h"
#include "app_socket.h"
#include "task_logging.h"
#include "task_tcpServer.h"
#include "utile.h"
#include "tcp_define.h"
#include "kma_protocol_handler.h"
#include "update_fw.h"

#define RECV_BUFF_SIZE 512
#define SERVER_RETRY_INTERVAL_MS 5000
#define CLIENT_CONNECT_TIMEOUT_MS 10000

tcp_status_t g_tcp_status;
osThreadId_t g_tcpSeverTaskId;

const osThreadAttr_t tcpServerTask_attributes = {
  .name = "tcpServerTask",
  .stack_size = 4096,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
  .priority = (osPriority_t) osPriorityNormal,
};

tcp_status_t *get_tcp_system(void) 
{ 
  return &g_tcp_status; 
}

void noti_tcpServerTask(uint32_t flag)
{
  if(g_tcpSeverTaskId)
  {
    osThreadFlagsSet(g_tcpSeverTaskId,  flag);
  }
  
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
void server_service(int sock)
{
  uint8_t rbuffer[RECV_BUFF_SIZE];
  uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
  int32_t ret, len, err_code;

  if (set_recv_timeout(sock, CLIENT_CONNECT_TIMEOUT_MS) < 0)
  {
    io_printf("타임아웃 설정 실패\r\n");
    return;
  }

  while (1)
  {
    ret = recv(sock, rbuffer, sizeof(rbuffer), 0);

    if (ret < 0)
    {
      err_code = errno;
      if (err_code == EAGAIN || err_code == EWOULDBLOCK)
      {
        continue;  // 타임아웃
      }
      else
      {
        io_printf("recv error on socket %d, errno: %d\r\n", sock, err_code);
        break;
      }

      io_printf("recv failed: errno=%d\r\n", err_code);
      break;  // 연결 종료 처리
    }
    else if (ret == 0)
    {
      io_printf("Client: Connection closed by peer on socket %d\r\n", sock);
      break;
    }
    else
    {
      UPDATE_CNT(g_tcp_status.rx_cnt, 99);
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
            io_printf("전송 실패 errno=%d\r\n", errno);
            send_error = true;
            break;
          }
          else
          {
            total += ret;
            UPDATE_CNT(g_tcp_status.tx_cnt, 99);
            if (get_firmware_update())
            {
              closesocket(sock);
              reset_system(0, "TCP client update");
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
  g_tcp_status.link_status = eLINK_DOWN;
}

void tcpServerTask(void *arg)
{
  int32_t opt = 1;
  int32_t sock = -1;     // 소켓 디스크립터 초기화
  int32_t newconn = -1;  // 클라이언트 소켓 디스크립터 초기화
  socklen_t size;        // accept에서 사용될 구조체 크기 변수 (socklen_t가 표준)
  struct sockaddr_in address, remotehost;
  struct sockaddr_in oldClient;  // 이전 클라이언트 정보 저장용
  int error_val = 0;             // getsockopt에서 사용될 오류 값 변수
  socklen_t len_error = sizeof(error_val);
  uint16_t local_port = (uint16_t)arg;  

  char client_ip_str[INET_ADDRSTRLEN];

  memset(&oldClient, 0, sizeof(oldClient));

  osThreadFlagsWait(0x00000001, osFlagsWaitAny, osWaitForever);

  while (1)
  {
    // 1. 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // IPPROTO_TCP 명시
    if (sock < 0)
    {
      osDelay(1000);
      continue;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
      closesocket(sock);
      sock = -1;
      osDelay(1000);
      continue;
    }

    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_val, &len_error) < 0)
    {
      closesocket(sock);
      sock = -1;
      osDelay(1000);
      continue;
    }
    if (error_val != 0)
    {
      closesocket(sock);
      sock = -1;
      osDelay(1000);
      continue;
    }


    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(local_port);
    address.sin_addr.s_addr = INADDR_ANY;  // 모든 IP 주소에서 오는 연결 허용

    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
      closesocket(sock);
      sock = -1;
      osDelay(1000);
      continue;
    }

    if (listen(sock, 5) < 0)
    {
      io_printf("TCP Server: Failed to listen on socket, errno: %d. Closing socket.\r\n", errno);
      closesocket(sock);
      sock = -1;
      osDelay(1000);
      continue;
    }

    size = sizeof(remotehost);

    while (1)
    {
      newconn = accept(sock, (struct sockaddr *)&remotehost, &size);

      if (newconn < 0)  // accept 실패
      {
        io_printf("TCP Server: accept() failed, errno: %d\r\n", errno);

        break; 
      }

      if (oldClient.sin_addr.s_addr != remotehost.sin_addr.s_addr ||
          oldClient.sin_port != remotehost.sin_port)
      {
        inet_ntop(AF_INET, &remotehost.sin_addr, client_ip_str, sizeof(client_ip_str));
        io_printf("Accepted : %s, Port: %d, Socket: %d\r\n", client_ip_str, ntohs(remotehost.sin_port), newconn);
        oldClient = remotehost;  
      }
      else
      {
        log_printf(L_INFO, "TCP Server: Re-accepted connection from same client, Socket: %d\r\n",
                   newconn);
      }


      enable_keepalive(newconn, 60000, 60000, 2);
    
      server_service(newconn);
      closesocket(newconn);
      newconn = -1;
    } 


    if (sock >= 0)
    {
      io_printf("TCP Server: Closing listening socket %d.\r\n", sock);
      closesocket(sock);
      sock = -1;
    }
    osDelay(100); 
  }  
}

void tcpServerTask_init(uint32_t flag)
{

  uint16_t local_port;

 local_port = get_config_app()->eth_local_port;

  g_tcp_status.link_status = eLINK_IDLE;

  g_tcpSeverTaskId = osThreadNew(tcpServerTask, (void*)local_port, &tcpServerTask_attributes);
}

#else
#include "cmsis_os.h"
#include "config_app.h"
#include "driver_rtc.h"
#include "dev_io.h"
#include "lwip.h"
#include "lwip/sockets.h" // Ensure sockets.h is included for socket functions
#include "lwip/inet.h"    // For inet_ntop
#include "utile_time.h"
#include "app_logging.h"
#include "app_socket.h"
#include "task_logging.h"
#include "task_tcpServer.h"
#include "utile.h"
#include "tcp_define.h"
#include "kma_protocol_handler.h"
#include "update_fw.h"
#include "user_heap.h"
#include <string.h> // For memset
#include <errno.h>  // For errno
#include <stdbool.h>// For bool type

#define RECV_BUFF_SIZE 512
#define SERVER_RETRY_INTERVAL_MS 5000
#define CLIENT_CONNECT_TIMEOUT_MS 10000 // Timeout for individual client operations
#define MAX_CONCURRENT_CLIENTS 2        // 최대 동시 접속 클라이언트 수


tcp_status_t g_tcp_status;
static osMutexId_t g_tcp_status_mutex; 

osThreadId_t g_tcpSeverTaskId;


typedef struct {
    osThreadId_t taskId;
    int          client_socket;
    bool         isActive;
    char         client_ip_str[INET_ADDRSTRLEN]; 
    uint16_t     client_port;                  
} client_slot_t;

static client_slot_t client_slots[MAX_CONCURRENT_CLIENTS];
static osMutexId_t client_slots_mutex;


const osThreadAttr_t tcpServerTask_attributes = {
  .name = "tcpServerTask",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityNormal,
};


const osThreadAttr_t clientHandlerTask_attributes = {
  .name = "clientHandler", 
  .stack_size = 4096, 
  .priority = (osPriority_t) osPriorityNormal,
};


static void client_handler_task(void *argument);
static void server_service_for_client(int sock, client_slot_t* slot); // server_service 수정본


tcp_status_t *get_tcp_system(void)
{
  return &g_tcp_status;
}

void noti_tcpServerTask(uint32_t flag)
{
  if(g_tcpSeverTaskId != NULL) // NULL 체크 추가
  {
    osThreadFlagsSet(g_tcpSeverTaskId,  flag);
  }
}

int set_recv_timeout(int sockfd, uint32_t timeout_ms)
{
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        io_printf("Failed to set SO_RCVTIMEO, errno: %d\r\n", errno);
        return -1;
    }
    return 0;
}


#define UPDATE_TCP_STATUS_CNT(counter_field, max_val) \
    do { \
        if (g_tcp_status_mutex != NULL && osMutexAcquire(g_tcp_status_mutex, osWaitForever) == osOK) { \
            g_tcp_status.counter_field = (g_tcp_status.counter_field >= (max_val)) ? 0 : g_tcp_status.counter_field + 1; \
            osMutexRelease(g_tcp_status_mutex); \
        } else { /* 뮤텍스 에러 처리 또는 그냥 카운트 (정확도 저하 감수) */ \
            g_tcp_status.counter_field = (g_tcp_status.counter_field >= (max_val)) ? 0 : g_tcp_status.counter_field + 1; \
            io_printf("Warning: Failed to acquire g_tcp_status_mutex for counter update.\r\n"); \
        } \
    } while(0)



static void server_service_for_client(int sock, client_slot_t* slot)
{

  uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
  int32_t ret, len, err_code;
  
  uint8_t *p_rx_buffer;

  p_rx_buffer = aws_malloc(RECV_BUFF_SIZE);

  if(p_rx_buffer ==NULL)
  {
    return;
  }

  io_printf("Client Handler: Servicing client %s:%u on socket %d\r\n", slot->client_ip_str,
                   slot->client_port, sock);

  if (set_recv_timeout(sock, CLIENT_CONNECT_TIMEOUT_MS) < 0)
  {
    io_printf("Client Handler (%s:%u): Timeout set failed for socket %d\r\n", slot->client_ip_str, slot->client_port, sock);
    return;
  }

  while (1)
  {
    ret = recv(sock, p_rx_buffer, RECV_BUFF_SIZE, 0);

    if (ret < 0) // recv 오류
    {
      err_code = errno;
      if (err_code == EAGAIN || err_code == EWOULDBLOCK)
      {
       //  io_printf("Client Handler (%s:%u): recv timeout on socket %d\r\n", slot->client_ip_str, slot->client_port, sock);
        continue;  // 타임아웃, 다음 수신 시도
      }
      else
      {
        io_printf("Client Handler (%s:%u): recv error on socket %d, errno: %d\r\n", slot->client_ip_str, slot->client_port, sock, err_code);
        break; // 그 외 오류는 루프 종료
      }
    }
    else if (ret == 0) // 상대방이 연결 정상 종료
    {
      io_printf("Client Handler (%s:%u): Connection closed by peer on socket %d\r\n", slot->client_ip_str, slot->client_port, sock);
      break; // 루프 종료
    }
    else // 데이터 수신 성공 (ret > 0)
    {
      UPDATE_TCP_STATUS_CNT(rx_cnt, 99); // 스레드 안전한 카운터 업데이트
      len = kma_cmd_handler(p_rx_buffer, ret, tx_buffer, eREQ_SOURCE_ETH);

      if (len > 0) // 응답할 데이터가 있는 경우
      {
        int32_t total_sent = 0;
        bool send_error = false;

        while (total_sent < len)
        {
          ret = send(sock, tx_buffer + total_sent, len - total_sent, 0);
          if (ret <= 0) // send 오류 또는 연결 종료
          {
            err_code = errno;
            io_printf("Client Handler (%s:%u): send failed on socket %d, sent %d/%d, errno: %d\r\n",
                         slot->client_ip_str, slot->client_port, sock, total_sent, len, (ret < 0 ? err_code : 0));
            send_error = true;
            break; // 내부 send 루프 종료
          }
          total_sent += ret;
        }

        if (send_error)
        {
          break; // 외부 서비스 루프 종료
        }

        UPDATE_TCP_STATUS_CNT(tx_cnt, 99); 

        if (get_firmware_update())
        {
          io_printf("Client Handler (%s:%u): Firmware update triggered via TCP. Resetting system.\r\n", slot->client_ip_str, slot->client_port);
          closesocket(sock);
          reset_system(0, "TCP client update"); //리턴 없음
        }
      }
    }
  }
  io_printf("Client Handler (%s:%u): Service loop ended for socket %d.\r\n", slot->client_ip_str, slot->client_port, sock);

  aws_free(p_rx_buffer);

}


static void client_handler_task(void *argument)
{
  client_slot_t *slot = (client_slot_t *)argument;
  int client_socket_fd = slot->client_socket;

  server_service_for_client(client_socket_fd, slot);

  closesocket(client_socket_fd);
  io_printf("Client Handler Task: Closed client socket %d (%s:%u)\r\n", client_socket_fd, slot->client_ip_str, slot->client_port);

  if (osMutexAcquire(client_slots_mutex, osWaitForever) == osOK)
  {
    slot->isActive = false;
    slot->taskId = NULL; // 태스크 ID 초기화
    slot->client_socket = -1;
    osMutexRelease(client_slots_mutex);
  } else {
    io_printf("Error: client_handler_task failed to acquire client_slots_mutex for cleanup.\r\n");
  }

  osThreadExit();
}


void tcpServerTask(void *arg)
{
  int32_t opt = 1;
  int32_t listen_sock = -1;
  int32_t new_conn_sock = -1;
  socklen_t remotehost_size;
  struct sockaddr_in server_addr, remote_addr;
  struct sockaddr_in old_client_info; // IP 로깅용
  int error_val = 0;
  socklen_t len_error = sizeof(error_val);
  uint16_t local_port = (uint16_t)(uintptr_t)arg; // void*를 uint16_t로 안전하게 변환

  char client_ip_str_buffer[INET_ADDRSTRLEN];

  // client_slots 초기화
  if (osMutexAcquire(client_slots_mutex, osWaitForever) == osOK) {
      for (int i = 0; i < MAX_CONCURRENT_CLIENTS; ++i) {
          client_slots[i].isActive = false;
          client_slots[i].taskId = NULL;
          client_slots[i].client_socket = -1;
      }
      osMutexRelease(client_slots_mutex);
  } else {
      io_printf("FATAL: tcpServerTask failed to acquire client_slots_mutex for init.\r\n");
      return;
  }


  memset(&old_client_info, 0, sizeof(old_client_info));

  osThreadFlagsWait(0x00000001, osFlagsWaitAny, osWaitForever);
  
  if (g_tcp_status_mutex != NULL) { 
      if (osMutexAcquire(g_tcp_status_mutex, osWaitForever) == osOK) {
          g_tcp_status.link_status = eLINK_IDLE; // 또는 eLINK_LISTENING
          osMutexRelease(g_tcp_status_mutex);
      }
  }


  while (1)
  {
    // 1. 리스닝 소켓 생성
    if (listen_sock < 0) // 이전 루프에서 소켓이 닫혔거나 초기 상태
    {
        listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listen_sock < 0)
        {
            io_printf("TCP Server: Failed to create listening socket, errno: %d. Retrying in %dms.\r\n", errno, SERVER_RETRY_INTERVAL_MS);
            osDelay(SERVER_RETRY_INTERVAL_MS);
            continue;
        }

        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            io_printf("TCP Server: Failed to set SO_REUSEADDR, errno: %d.\r\n", errno);
            closesocket(listen_sock); listen_sock = -1;
            osDelay(SERVER_RETRY_INTERVAL_MS);
            continue;
        }

        // SO_ERROR 확인 (선택적)
        if (getsockopt(listen_sock, SOL_SOCKET, SO_ERROR, &error_val, &len_error) < 0 || error_val != 0)
        {
            io_printf("TCP Server: Socket error after creation/setsockopt. Error: %d, Errno: %d.\r\n", error_val, errno);
            closesocket(listen_sock); listen_sock = -1;
            osDelay(SERVER_RETRY_INTERVAL_MS);
            continue;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(local_port);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            io_printf("TCP Server: Failed to bind to port %u, errno: %d.\r\n", local_port, errno);
            closesocket(listen_sock); listen_sock = -1;
            osDelay(SERVER_RETRY_INTERVAL_MS);
            continue;
        }

        if (listen(listen_sock, MAX_CONCURRENT_CLIENTS + 1) < 0) // 백로그는 동시 클라이언트 수보다 약간 크게
        {
            io_printf("TCP Server: Failed to listen, errno: %d.\r\n", errno);
            closesocket(listen_sock); listen_sock = -1;
            osDelay(SERVER_RETRY_INTERVAL_MS);
            continue;
        }
        io_printf("TCP Server: Listening on port %u (Socket: %d).\r\n", local_port, listen_sock);
        if (g_tcp_status_mutex != NULL) {
            if (osMutexAcquire(g_tcp_status_mutex, osWaitForever) == osOK) {
                 g_tcp_status.link_status = eLINK_UP;
                 osMutexRelease(g_tcp_status_mutex);
            }
        }
    }

    // 2. 클라이언트 연결 수락
    remotehost_size = sizeof(remote_addr);
    new_conn_sock = accept(listen_sock, (struct sockaddr *)&remote_addr, &remotehost_size);

    if (new_conn_sock < 0)
    {
        io_printf("TCP Server: accept() failed, errno: %d. Re-creating listening socket if necessary.\r\n", errno);
        // accept 실패 시 리스닝 소켓 자체에 문제가 있을 수 있음.
        // ECONNABORTED, EMFILE, ENFILE 등의 오류에 따라 리스닝 소켓을 닫고 다시 시도.
        if (errno == ECONNABORTED || errno == EINVAL) { // EINVAL은 listen_sock이 더 이상 유효하지 않음을 의미할 수 있음
             closesocket(listen_sock);
             listen_sock = -1; // 다음 루프에서 리스닝 소켓 재생성
        }
        osDelay(100); // 짧은 지연 후 다음 accept 시도 또는 소켓 재생성
        continue;
    }

    // 새 클라이언트 정보 로깅
    inet_ntop(AF_INET, &remote_addr.sin_addr, client_ip_str_buffer, sizeof(client_ip_str_buffer));
    if (old_client_info.sin_addr.s_addr != remote_addr.sin_addr.s_addr ||
        old_client_info.sin_port != remote_addr.sin_port)
    {
        io_printf("TCP Server: Accepted connection from %s:%u on socket %d\r\n",
                     client_ip_str_buffer, ntohs(remote_addr.sin_port), new_conn_sock);
        old_client_info = remote_addr;
    }

    // Keepalive 설정
    enable_keepalive(new_conn_sock, 60000, 60000, 2);


    // 3. 사용 가능한 클라이언트 슬롯 찾고 핸들러 태스크 생성
    int slot_index = -1;
    if (osMutexAcquire(client_slots_mutex, osWaitForever) == osOK)
    {
        for (int i = 0; i < MAX_CONCURRENT_CLIENTS; ++i)
        {
            if (!client_slots[i].isActive)
            {
                slot_index = i;
                client_slots[i].isActive = true; // 슬롯 사용 표시
                client_slots[i].client_socket = new_conn_sock;
                strncpy(client_slots[i].client_ip_str, client_ip_str_buffer, INET_ADDRSTRLEN-1);
                client_slots[i].client_ip_str[INET_ADDRSTRLEN-1] = '\0';
                client_slots[i].client_port = ntohs(remote_addr.sin_port);
                break;
            }
        }
        osMutexRelease(client_slots_mutex);
    } else {
        io_printf("Error: tcpServerTask failed to acquire client_slots_mutex to find slot.\r\n");
        closesocket(new_conn_sock); // 뮤텍스 획득 실패 시 소켓 닫고 다음 연결 시도
        new_conn_sock = -1;
        continue;
    }


    if (slot_index != -1) // 사용 가능한 슬롯 찾음
    {
      //uxTaskNumber
        client_slots[slot_index].taskId = osThreadNew(client_handler_task, &client_slots[slot_index], &clientHandlerTask_attributes);
        if (client_slots[slot_index].taskId == NULL)
        {
            io_printf("TCP Server: Failed to create client_handler_task for %s:%u.\r\n", client_slots[slot_index].client_ip_str, client_slots[slot_index].client_port);
            closesocket(new_conn_sock); // 태스크 생성 실패 시 소켓 닫기
            new_conn_sock = -1;
            // 슬롯 다시 비활성화
            if (osMutexAcquire(client_slots_mutex, osWaitForever) == osOK) {
                client_slots[slot_index].isActive = false;
                client_slots[slot_index].client_socket = -1;
                osMutexRelease(client_slots_mutex);
            }
        }
        else
        {
            io_printf("TCP Server: client_handler_task created for %s:%u (Slot %d, TaskID: %p)\r\n",
                         client_slots[slot_index].client_ip_str, client_slots[slot_index].client_port, slot_index, client_slots[slot_index].taskId);
            new_conn_sock = -1; // 소켓 제어권이 핸들러 태스크로 넘어감
        }
    }
    else // 사용 가능한 슬롯 없음 (최대 클라이언트 수 도달)
    {
        io_printf("TCP Server: Max concurrent clients (%d) reached. Rejecting connection from %s:%u.\r\n",
                     MAX_CONCURRENT_CLIENTS, client_ip_str_buffer, ntohs(remote_addr.sin_port));
        closesocket(new_conn_sock);
        new_conn_sock = -1;
        osDelay(100); // 잠시 후 다시 accept 시도 (너무 빠른 루프 방지)
    }
  } // end while(1) for server
}


void tcpServerTask_init(uint32_t flag) // flag 매개변수는 현재 사용되지 않음
{
  uint16_t local_port;

  // 뮤텍스 생성
  client_slots_mutex = osMutexNew(NULL); // 기본 속성으로 뮤텍스 생성
  if (client_slots_mutex == NULL) {
      io_printf("FATAL: Failed to create client_slots_mutex.\r\n");
      // 시스템 초기화 실패 처리
      return;
  }

  g_tcp_status_mutex = osMutexNew(NULL);
  if (g_tcp_status_mutex == NULL) {
      io_printf("FATAL: Failed to create g_tcp_status_mutex.\r\n");
      // client_slots_mutex는 생성되었으므로 필요시 삭제
      osMutexDelete(client_slots_mutex);
      return;
  }


  local_port = get_config_app()->eth_local_port;

  g_tcp_status.link_status = eLINK_IDLE; // 태스크 시작 시 업데이트


  g_tcpSeverTaskId = osThreadNew(tcpServerTask, (void*)(uintptr_t)local_port, &tcpServerTask_attributes);
  if (g_tcpSeverTaskId == NULL) {
      io_printf("FATAL: Failed to create tcpServerTask.\r\n");

      osMutexDelete(client_slots_mutex);
      osMutexDelete(g_tcp_status_mutex);
  } else {
      io_printf("TCP Server Task initialized. Waiting for network...\r\n");

  }
}

#endif