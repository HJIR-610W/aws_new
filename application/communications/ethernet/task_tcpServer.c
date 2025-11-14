
#include <errno.h>    
#include <stdbool.h>  
#include <string.h>   

#include "app_logging.h"
#include "app_socket.h"
#include "cmsis_os.h"
#include "config_app.h"
#include "dev_io.h"
#include "drv_rtc.h"
#include "kma_protocol_handler.h"
#include "lwip.h"
#include "lwip/inet.h"     
#include "lwip/sockets.h" 
#include "task_logging.h"
#include "task_tcpServer.h"
#include "tcp_define.h"
#include "update_fw.h"
#include "user_heap.h"
#include "util_memory.h"
#include "util_time.h"
#include "os_user_def.h"

#define RECV_BUFF_SIZE 512
#define SERVER_RETRY_INTERVAL_MS 5000
#define CLIENT_CONNECT_TIMEOUT_MS 1000 
#define MAX_CONCURRENT_CLIENTS 3        // 최대 동시 접속 클라이언트 수
#ifndef SERVER_REQ_TIMEOUT
#define SERVER_REQ_TIMEOUT 80000
#endif

typedef struct
{
  osThreadId_t taskId;
  int client_socket;
  bool isActive;
  char client_ip_str[INET_ADDRSTRLEN];
  uint16_t client_port;
  tcp_system_t *status;
} client_slot_t;

static tcp_system_t g_tcp_status[MAX_CONCURRENT_CLIENTS];

static client_slot_t client_slots[MAX_CONCURRENT_CLIENTS];
static osMutexId_t client_slots_mutex;

osThreadId_t g_tcpSeverTaskId;


const osThreadAttr_t tcpServerTask_attributes = {
  .name = "tcp_server",
  .stack_size = TASK_STACK(TASK_TCP_SERVER_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_TCP_SERVER_DEF),
};

const osThreadAttr_t clientHandlerTask_attributes = {
  .name = "tcp_client", 
  .stack_size = TASK_STACK(TASK_CLIENT_HANDLER_DEF), 
  .priority = (osPriority_t)TASK_PRIO(TASK_CLIENT_HANDLER_DEF),
};


static void client_handler_task(void *argument);
static void server_service_for_client(int sock, client_slot_t* slot); // server_service 수정본

tcp_system_t *get_tcp_system(uint32_t number)
{
  return &g_tcp_status[number];
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
      task_printf("SO_RCVTIMEO 설정에 실패했습니다. 오류 번호: %d\r\n", errno);

      return -1;
    }
    return 0;
}




extern uint8_t g_ethernet_phy_link;
static void server_service_for_client(int sock, client_slot_t* slot)
{

  uint8_t tx_buffer[KMA_TX_BUFFER_SIZE];
  int32_t ret, len, err_code;
  uint8_t *p_rx_buffer;
  uint32_t start_time;

  p_rx_buffer = pvPortMalloc(RECV_BUFF_SIZE);

  if(p_rx_buffer ==NULL)
  {
    return;
  }

  task_printf("클라이언트 핸들러: 클라이언트 %s:%u (소켓 %d) 처리 중\r\n", slot->client_ip_str,
            slot->client_port, sock);

  if (set_recv_timeout(sock, CLIENT_CONNECT_TIMEOUT_MS) < 0)
  {
    task_printf("클라이언트 핸들러 (%s:%u): 소켓 %d에 대한 타임아웃 설정 실패\r\n",
              slot->client_ip_str, slot->client_port, sock);
    vPortFree(p_rx_buffer);
    return;
  }

  start_time = OS_GET_TICK();
  while (1)
  {
    ret = recv(sock, p_rx_buffer, RECV_BUFF_SIZE, 0);

    if(g_ethernet_phy_link  ==0)
    {
      break;
    }
    if (ret < 0) // recv 오류
    {
      if((OS_GET_TICK() - start_time) > SERVER_REQ_TIMEOUT)
      {
        break;
      }
      err_code = errno;
      if (err_code == EAGAIN )//|| err_code == EWOULDBLOCK)
      {
       //  task_printf("Client Handler (%s:%u): recv timeout on socket %d\r\n", slot->client_ip_str, slot->client_port, sock);
        continue;  // 타임아웃, 다음 수신 시도
      }
      else
      {
        task_printf("클라이언트 핸들러 (%s:%u): 소켓 %d에서 수신 오류 발생, 오류 번호: %d\r\n",
                  slot->client_ip_str, slot->client_port, sock, err_code);

        break; // 그 외 오류는 루프 종료
      }
    }
    else if (ret == 0) // 상대방이 연결 정상 종료
    {
      task_printf("클라이언트 핸들러 (%s:%u): 소켓 %d에서 상대측이 연결을 종료함\r\n",
                slot->client_ip_str, slot->client_port, sock);

      break; // 루프 종료
    }
    else // 데이터 수신 성공 (ret > 0)
    {
      start_time = OS_GET_TICK();
      slot->status->last_recv_time = time_timestamp();
      UPDATE_CNT(slot->status->rx_cnt, 99);  // 스레드 안전한 카운터 업데이트
      len = kma_cmd_handler(p_rx_buffer, ret, tx_buffer, eREQ_SOURCE_ETH);

      if (len > 0) // 응답할 데이터가 있는 경우
      {
        int32_t total_sent = 0;
        bool send_error = false;

        while (total_sent < len)
        {
          ret = send(sock, tx_buffer + total_sent, len - total_sent, 0);
          slot->status->last_send_time = time_timestamp();
          if (ret <= 0)  // send 오류 또는 연결 종료
          {
            err_code = errno;
            task_printf(
                "클라이언트 핸들러 (%s:%u): 소켓 %d에서 전송 실패, 전송 %d/%d 바이트, 오류 번호: "
                "%d\r\n",
                slot->client_ip_str, slot->client_port, sock, total_sent, len,
                (ret < 0 ? err_code : 0));

            send_error = true;
            break; // 내부 send 루프 종료
          }
          total_sent += ret;
        }

        if (send_error)
        {
          break; // 외부 서비스 루프 종료
        }

        UPDATE_CNT(slot->status->tx_cnt, 99);  

        if (get_firmware_update())
        {
          task_printf(
              "클라이언트 핸들러 (%s:%u): TCP를 통해 펌웨어 업데이트 트리거됨. 시스템을 "
              "리셋합니다.\r\n",
              slot->client_ip_str, slot->client_port);

          closesocket(sock);
          reset_system( "TCP client update"); //리턴 없음
        }
      }
    }
  }
  task_printf("클라이언트 핸들러 (%s:%u): 소켓 %d에 대한 서비스 루프 종료\r\n", slot->client_ip_str, slot->client_port, sock);


  vPortFree(p_rx_buffer);


}


static void client_handler_task(void *argument)
{
  client_slot_t *slot = (client_slot_t *)argument;
  int client_socket_fd = slot->client_socket;

  slot->status->link_status = eLINK_UP;

  server_service_for_client(client_socket_fd, slot);

  closesocket(client_socket_fd);
  task_printf("클라이언트 핸들러 태스크: 클라이언트 소켓 %d (%s:%u) 닫힘\r\n", client_socket_fd,
            slot->client_ip_str, slot->client_port);

  if (osMutexAcquire(client_slots_mutex, osWaitForever) == osOK)
  {
    slot->isActive = false;
    slot->taskId = NULL; // 태스크 ID 초기화
    slot->client_socket = -1;
    osMutexRelease(client_slots_mutex);
  } else {
    task_printf(
        "오류: 정리를 위해 client_handler_task가 client_slots_mutex를 획득하지 못했습니다.\r\n");
  }

  slot->status->link_status = eLINK_DOWN;
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

  DEBUG_PRINTF("tcpServer task start\r\n");

  // client_slots 초기화
  if (osMutexAcquire(client_slots_mutex, osWaitForever) == osOK) {
      for (int i = 0; i < MAX_CONCURRENT_CLIENTS; ++i) {
          client_slots[i].isActive = false;
          client_slots[i].taskId = NULL;
          client_slots[i].client_socket = -1;
      }
      osMutexRelease(client_slots_mutex);
  } else {
    task_printf(
        "치명적 오류: 초기화를 위해 tcpServerTask가 client_slots_mutex를 획득하지 못했습니다.\r\n");

    return;
  }


  memset(&old_client_info, 0, sizeof(old_client_info));

  osThreadFlagsWait(0x00000001, osFlagsWaitAny, osWaitForever);
  



  while (1)
  {
    // 1. 리스닝 소켓 생성
    if (listen_sock < 0) // 이전 루프에서 소켓이 닫혔거나 초기 상태
    {
        listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listen_sock < 0)
        {
          task_printf(
              "TCP 서버: 수신 대기용 소켓 생성 실패, 오류 번호: %d. %dms 후 재시도합니다.\r\n",
              errno, SERVER_RETRY_INTERVAL_MS);

          osDelay(SERVER_RETRY_INTERVAL_MS);
          continue;
        }

        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
          task_printf("TCP 서버: SO_REUSEADDR 설정 실패, 오류 번호: %d.\r\n", errno);

          closesocket(listen_sock);
          listen_sock = -1;
          osDelay(SERVER_RETRY_INTERVAL_MS);
          continue;
        }

        // SO_ERROR 확인 (선택적)
        if (getsockopt(listen_sock, SOL_SOCKET, SO_ERROR, &error_val, &len_error) < 0 || error_val != 0)
        {
          task_printf(
              "TCP 서버: 소켓 생성 또는 setsockopt 이후 오류 발생. 오류값: %d, errno: %d.\r\n",
              error_val, errno);

          closesocket(listen_sock);
          listen_sock = -1;
          osDelay(SERVER_RETRY_INTERVAL_MS);
          continue;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(local_port);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
          task_printf("TCP 서버: 포트 %u 바인딩 실패, 오류 번호: %d.\r\n", local_port, errno);

          closesocket(listen_sock);
          listen_sock = -1;
          osDelay(SERVER_RETRY_INTERVAL_MS);
          continue;
        }

        if (listen(listen_sock, MAX_CONCURRENT_CLIENTS + 1) < 0) // 백로그는 동시 클라이언트 수보다 약간 크게
        {
          task_printf("TCP 서버: 수신 대기(listen) 실패, 오류 번호: %d.\r\n", errno);

          closesocket(listen_sock);
          listen_sock = -1;
          osDelay(SERVER_RETRY_INTERVAL_MS);
          continue;
        }
        task_printf("TCP 서버: 포트 %u에서 수신 대기 중 (소켓: %d).\r\n", local_port, listen_sock);


    }

    // 2. 클라이언트 연결 수락
    remotehost_size = sizeof(remote_addr);
    new_conn_sock = accept(listen_sock, (struct sockaddr *)&remote_addr, &remotehost_size);

    if (new_conn_sock < 0)
    {
      task_printf(
          "TCP 서버: accept() 실패, 오류 번호: %d. 필요 시 수신 대기 소켓을 다시 생성합니다.\r\n",
          errno);

      // accept 실패 시 리스닝 소켓 자체에 문제가 있을 수 있음.
      // ECONNABORTED, EMFILE, ENFILE 등의 오류에 따라 리스닝 소켓을 닫고 다시 시도.
      if (errno == ECONNABORTED || errno == EINVAL)
      {  // EINVAL은 listen_sock이 더 이상 유효하지 않음을 의미할 수 있음
        closesocket(listen_sock);
        listen_sock = -1;  // 다음 루프에서 리스닝 소켓 재생성
      }
        osDelay(100); // 짧은 지연 후 다음 accept 시도 또는 소켓 재생성
        continue;
    }

    // 새 클라이언트 정보 로깅
    inet_ntop(AF_INET, &remote_addr.sin_addr, client_ip_str_buffer, sizeof(client_ip_str_buffer));
    if (old_client_info.sin_addr.s_addr != remote_addr.sin_addr.s_addr ||
        old_client_info.sin_port != remote_addr.sin_port)
    {
      task_printf("TCP 서버: %s:%u 로부터 소켓 %d에서 연결 수락\r\n", client_ip_str_buffer,
                ntohs(remote_addr.sin_port), new_conn_sock);

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
      task_printf(
          "오류: 슬롯 검색을 위해 tcpServerTask가 client_slots_mutex를 획득하지 못했습니다.\r\n");

      closesocket(new_conn_sock);  
      new_conn_sock = -1;
      continue;
    }


    if (slot_index != -1) 
    {
      //uxTaskNumber
        client_slots[slot_index].taskId = osThreadNew(client_handler_task, &client_slots[slot_index], &clientHandlerTask_attributes);
        if (client_slots[slot_index].taskId == NULL)
        {
          task_printf("TCP 서버: %s:%u에 대한 client_handler_task 생성 실패.\r\n",
                    client_slots[slot_index].client_ip_str, client_slots[slot_index].client_port);

          closesocket(new_conn_sock);  // 태스크 생성 실패 시 소켓 닫기
          new_conn_sock = -1;
          // 슬롯 다시 비활성화
          if (osMutexAcquire(client_slots_mutex, osWaitForever) == osOK)
          {
            client_slots[slot_index].isActive = false;
            client_slots[slot_index].client_socket = -1;
            osMutexRelease(client_slots_mutex);
          }
        }
        else
        {
          client_slots[slot_index].status->rx_cnt = 0;
          client_slots[slot_index].status->tx_cnt = 0;
          client_slots[slot_index].status->last_recv_time = 0;
          client_slots[slot_index].status->last_send_time = 0;
          task_printf(
              "TCP 서버: %s:%u에 대한 client_handler_task 생성됨 (슬롯 %d, 태스크 ID: %p)\r\n",
              client_slots[slot_index].client_ip_str, client_slots[slot_index].client_port,
              slot_index, client_slots[slot_index].taskId);
          task_printf("%04d-%02d-%02d %02d:%02d:%02d\r\n",Date_Time.Year,Date_Time.Month,Date_Time.Day,
            Date_Time.Hour,Date_Time.Min,Date_Time.Sec);
          new_conn_sock = -1;  // 소켓 제어권이 핸들러 태스크로 넘어감
        }
    }
    else // 사용 가능한 슬롯 없음 (최대 클라이언트 수 도달)
    {
      task_printf("TCP 서버: 최대 동시 접속자 수(%d) 도달. %s:%u의 연결을 거부합니다.\r\n",
                MAX_CONCURRENT_CLIENTS, client_ip_str_buffer, ntohs(remote_addr.sin_port));

      closesocket(new_conn_sock);
      new_conn_sock = -1;
      osDelay(100);  // 잠시 후 다시 accept 시도 (너무 빠른 루프 방지)
    }
  } // end while(1) for server
}


void tcpServerTask_init(uint32_t flag) // flag 매개변수는 현재 사용되지 않음
{
  uint16_t local_port;

  // 뮤텍스 생성
  client_slots_mutex = osMutexNew(NULL); // 기본 속성으로 뮤텍스 생성
  if (client_slots_mutex == NULL) {
    task_printf("치명적 오류: client_slots_mutex 생성 실패.\r\n");

    // 시스템 초기화 실패 처리
    return;
  }



  local_port = get_config_app()->eth_local_port;

  for (int i = 0; i < ETH_CLIENT_MAX;i++)
  {
    g_tcp_status[i].link_status = eLINK_IDLE;  // 태스크 시작 시 업데이트
    client_slots[i].status = &g_tcp_status[i];
    strcpy(client_slots[i].client_ip_str, "-");
    client_slots[i].status->client_ip_str = client_slots[i].client_ip_str;
  }


  g_tcpSeverTaskId =
      osThreadNew(tcpServerTask, (void *)(uintptr_t)local_port, &tcpServerTask_attributes);
  if (g_tcpSeverTaskId == NULL) {
    task_printf("치명적 오류: tcpServerTask 생성 실패.\r\n");

    osMutexDelete(client_slots_mutex);

  } else {
    task_printf("TCP 서버 태스크 초기화 완료. 네트워크를 기다리는 중...\r\n");
  }
}

