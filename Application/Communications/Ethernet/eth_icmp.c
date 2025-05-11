
#include <string.h>
#include <stdio.h>

#include "eth_icmp.h"

#include "cmsis_os2.h"
#include "lwip/icmp.h"
#include "lwip/ip.h"
#include "lwip/sockets.h"
#include "lwip/inet_chksum.h"

#if 0 

/**
 * @brief 핑테스트를 하고 결과값을 메시지형식으로 전달
 */
void ping_task(const void *arg)
{
    ping_req_t *params = (ping_req_t *)arg;  // 매개변수 구조체
    struct sockaddr_in dest_addr;
    char send_buf[40];  // Ping 데이터 버퍼
    char recv_buf[128]; // 수신 데이터 버퍼
    int sock;
    int seq = 0;        // ICMP Echo Request의 시퀀스 번호
    int i;
    int len;
    uint8_t recved_cnt=0;
    char message[200]={"{\"message\":\"unknown err\"}"};
    // 대상 주소 설정
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(params->dest_ip);
    dest_addr.sin_port = 0; // ICMP는 포트 사용 안 함

    // 소켓 생성
    sock = socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);
    if (sock < 0) {
          send_response("{\"message\":\"sock fail\"}");
        osThreadTerminate(NULL);
        return;
    }

    // 송수신 타임아웃 설정
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    len = snprintf(message,sizeof(message),"{\"message\":[");

    for(int i = 0 ; i< 32; i++)
    {
      send_buf[i+8]='a'+i;
    }
    
    for (i = 0; i < params->cnt; i++) 
    {
       struct icmp_echo_hdr *icmp_hdr = (struct icmp_echo_hdr *)send_buf;
       struct sockaddr_in from_addr;
       socklen_t from_len = sizeof(from_addr);
       int recv_len;

        // ICMP Echo Request 생성
        icmp_hdr->type = ICMP_ECHO;
        icmp_hdr->code = 0;
        icmp_hdr->chksum = 0;//하드웨어에서 처리 함
        icmp_hdr->id = htons(0x1234); // 임의의 식별자
        icmp_hdr->seqno = htons(seq++);
        // Ping 데이터 전송
        uint32_t start_time = osKernelSysTick();  // 시작 시간 측정
        if (sendto(sock, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
        {
          send_response("{\"message\":\"send timeout\"}");
          close(sock);
          osThreadTerminate(NULL);
        }

        // Ping 응답 수신
        recv_len = recvfrom(sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&from_addr, &from_len);

        if (recv_len > 0) 
        {
          recved_cnt++;
            uint32_t end_time = osKernelSysTick();  // 종료 시간 측정
            uint32_t rtt = (end_time - start_time) ;

            // TTL 확인
            struct ip_hdr *ip_hdr = (struct ip_hdr *)recv_buf;
            int ttl = ip_hdr->_ttl;

               len += snprintf(&message[len],sizeof(message)-len,"\"Reply from %s bytes=%d time=%dms TTL=%d\",",
                               params->dest_ip, recv_len, rtt==0?1:rtt, ttl);
        } else 
        {
          send_response("{\"message\":\"recv timeout\"}");
          close(sock);
          osThreadTerminate(NULL);
        }
        // 1초 간격으로 대기
        osDelay(1000);
    }

    // 소켓 닫기
    close(sock);

    if(recved_cnt == 0)
    {
      snprintf(message,sizeof(message),"{\"message\":\"timeout\"}");
    }
    else
    {
      len--;
      message[len]=0;
      snprintf(&message[len],sizeof(message)-len,"]}");
    }
    
    send_response(message);
   osThreadTerminate(NULL);
}



void create_pingTask(ping_req_t *pPingReq)
{
  osThreadDef(ping_task, ping_task, TASK_PRIORITY_PING_TASK, 0, TASK_STACK_SIZE_PING);
  osThreadCreate(osThread(ping_task), (void *)pPingReq);
}

#endif