#include <stdio.h>
#include <string.h>

#include "cli_input.h"
#include "cli_key_code.h"
#include "dev_io.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip.h"
#include "lwip/ip_addr.h"
#include "lwip/netdb.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"

// 사용자 설정값을 저장할 구조체
typedef struct
{
  uint8_t eth_ip[4];
  uint8_t eth_subnet[4];
  uint8_t eth_gateway[4];
} eth_eth_config_t;

eth_eth_config_t eth_config;

#define PING_ID 0xAFAF
#define PING_DATA_SIZE 32
#define PING_COUNT 4
#define PING_DELAY_MS 1000
#define PING_TIMEOUT_MS 1000
#define STATIC_IPH_HL(ip_hdr) ((ip_hdr->_v_hl) & 0x0F)
static u16_t ping_seq_num = 0;

// 체크섬 계산
static u16_t ping_checksum(void *data, int len) { return inet_chksum(data, len); }

// Ping 전송
static err_t ping_send(int s, struct sockaddr_in *to)
{


  struct icmp_echo_hdr
  {
    u8_t type;
    u8_t code;
    u16_t chksum;
    u16_t id;
    u16_t seqno;
  } __attribute__((packed));

  struct icmp_echo_hdr *iecho;
  struct timeval timeout = {PING_TIMEOUT_MS / 1000, (PING_TIMEOUT_MS % 1000) * 1000};
  char buf[sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE];

  iecho = (struct icmp_echo_hdr *)buf;
  iecho->type = ICMP_ECHO;
  iecho->code = 0;
  iecho->id = PING_ID;
  iecho->seqno = htons(++ping_seq_num);

  memset(buf + sizeof(struct icmp_echo_hdr), 0xa5, PING_DATA_SIZE);

  iecho->chksum = 0;
  iecho->chksum = ping_checksum(iecho, sizeof(buf));

  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  return sendto(s, buf, sizeof(buf), 0, (struct sockaddr *)to, sizeof(*to));
}

// Ping 수신
static err_t ping_recv(int s, struct sockaddr_in *from)
{
  char buf[64];
  socklen_t fromlen = sizeof(*from);
  int len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)from, &fromlen);

  if (len >= (int)(sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr)))
  {
    struct ip_hdr *iphdr = (struct ip_hdr *)buf;
    struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)(buf + (STATIC_IPH_HL(iphdr) * 4));

    if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num)))
    {
      io_printf("Ping response from %s: seq=%d\r\n",
                   ipaddr_ntoa((const ip_addr_t *)&from->sin_addr), ntohs(iecho->seqno));
      return 0;
    }
  }

  io_printf("Ping timeout or invalid reply\r\n");
  return -1;
}

void ping_task(const char *target_ip)
{

  struct sockaddr_in dest_addr;
  char send_buf[40];   // Ping 데이터 버퍼
  char recv_buf[128];  // 수신 데이터 버퍼
  int sock;
  int seq = 0;  // ICMP Echo Request의 시퀀스 번호


 

  // 대상 주소 설정
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_addr.s_addr = inet_addr(target_ip);
  dest_addr.sin_port = 0;  // ICMP는 포트 사용 안 함


  io_printf("Pinging %s with %d bytes of data:\r\n", target_ip, PING_DATA_SIZE);


  // 소켓 생성
  sock = socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);
  if (sock < 0)
  {

    return;
  }

  // 송수신 타임아웃 설정
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));



  for (int i = 0; i < 32; i++)
  {
    send_buf[i + 8] = 'a' + i;
  }

  while(1)
  {
    struct icmp_echo_hdr *icmp_hdr = (struct icmp_echo_hdr *)send_buf;
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    int recv_len;

    // ICMP Echo Request 생성
    icmp_hdr->type = ICMP_ECHO;
    icmp_hdr->code = 0;
    icmp_hdr->chksum = 0;          // 하드웨어에서 처리 함
    icmp_hdr->id = htons(0x1234);  // 임의의 식별자
    icmp_hdr->seqno = htons(seq++);
    // Ping 데이터 전송
    uint32_t start_time = osKernelGetTickCount();  // 시작 시간 측정
    if (sendto(sock, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&dest_addr,
               sizeof(dest_addr)) < 0)
    {

      close(sock);

    }

    // Ping 응답 수신
    recv_len =
        recvfrom(sock, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&from_addr, &from_len);

    if (recv_len > 0)
    {
 
      uint32_t end_time = osKernelGetTickCount();  // 종료 시간 측정
      uint32_t rtt = (end_time - start_time);

      // TTL 확인
      struct ip_hdr *ip_hdr = (struct ip_hdr *)recv_buf;
      int ttl = ip_hdr->_ttl;

      io_printf("Reply from %s bytes=%d time=%dms TTL=%d\r\n", target_ip, recv_len,
                      rtt == 0 ? 1 : rtt, ttl);
    }
    else
    {

      close(sock);

    }
    // 1초 간격으로 대기
    if (get_key(1000) == KEY_CODE_CTRL_C)
    {
      break;
    }
  }

  // 소켓 닫기
  close(sock);



}
// Ping 실행
void lwip_ping_test(const char *target_ip)
{
  struct sockaddr_in addr;
  int s;

  s = socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);
  if (s < 0)
  {
    io_printf("Ping: 소켓 생성 실패\r\n");
    return;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(target_ip);

  io_printf("Pinging %s with %d bytes of data:\r\n", target_ip, PING_DATA_SIZE);

  for (int i = 0; i < PING_COUNT; i++)
  {
    if (ping_send(s, &addr) > 0)
    {
      ping_recv(s, &addr);
    }
    else
    {
      io_printf("Ping: 요청 전송 실패\r\n");
    }

    sys_msleep(PING_DELAY_MS);
  }

  closesocket(s);
  io_printf("Ping 테스트 종료\r\n");
}

// 사용자 입력으로 네트워크 설정
void network_setup_from_user(void)
{
  char ip_str[32], netmask_str[32], gw_str[32];

  io_printf("\r\n[ 네트워크 설정을 진행합니다. ]\r\n");

  // IP 주소 입력
  io_printf("IP 주소를 입력하세요 (예: 192.168.1.100): ");
  cli_scanf_s("%s", ip_str,sizeof(ip_str));

  // 서브넷 마스크 입력
  io_printf("서브넷 마스크를 입력하세요 (예: 255.255.255.0): ");
  cli_scanf_s("%s", netmask_str,sizeof(netmask_str));

  // 게이트웨이 입력
  io_printf("게이트웨이를 입력하세요 (예: 192.168.1.1): ");
  cli_scanf_s("%s", gw_str,sizeof(gw_str));

  // 문자열을 숫자 배열로 변환
  ip4_addr_t ipaddr, netmask, gw;
  if (!ip4addr_aton(ip_str, &ipaddr))
  {
    io_printf("잘못된 IP 주소입니다.\r\n");
    return;
  }
  if (!ip4addr_aton(netmask_str, &netmask))
  {
    io_printf("잘못된 서브넷 마스크입니다.\r\n");
    return;
  }
  if (!ip4addr_aton(gw_str, &gw))
  {
    io_printf("잘못된 게이트웨이입니다.\r\n");
    return;
  }

  // IP 정보 복사
  memcpy(eth_config.eth_ip, &ipaddr, sizeof(eth_config.eth_ip));
  memcpy(eth_config.eth_subnet, &netmask, sizeof(eth_config.eth_subnet));
  memcpy(eth_config.eth_gateway, &gw, sizeof(eth_config.eth_gateway));

  io_printf("\r\n네트워크 설정 완료:\r\n");
  io_printf("IP: %s\r\n", ip4addr_ntoa(&ipaddr));
  io_printf("Netmask: %s\r\n", ip4addr_ntoa(&netmask));
  io_printf("Gateway: %s\r\n", ip4addr_ntoa(&gw));
}


extern void MX_LWIP_Init(uint8_t ip[4],uint8_t mask[4],uint8_t gateway[4]);
// 전체 테스트 함수

int g_lwip_init=0;
void test_eth(void)
{
  char ping_ip_str[32];

  io_printf("\r\n[ Ethernet + Ping 테스트 시작 ]\r\n");

#if 0 
  // 사용자로부터 네트워크 설정 입력받기
  network_setup_from_user();
#else
  eth_config.eth_ip[0]=192;
  eth_config.eth_ip[1]=168;
  eth_config.eth_ip[2]=1;
  eth_config.eth_ip[3]=177;
  
  eth_config.eth_gateway[0]=192;
  eth_config.eth_gateway[1]=168;
  eth_config.eth_gateway[2]=1;
  eth_config.eth_gateway[3]=1;  
  
  eth_config.eth_subnet[0]=255;
  eth_config.eth_subnet[1]=255;
  eth_config.eth_subnet[2]=255;
  eth_config.eth_subnet[3]=0;  
  
#endif
  // 네트워크 인터페이스 초기화
  io_printf("\r\n네트워크 인터페이스 초기화 중...\r\n");
  if (g_lwip_init==0)
  {
    g_lwip_init = 1;
     MX_LWIP_Init(eth_config.eth_ip, eth_config.eth_subnet, eth_config.eth_gateway);
  }
  io_printf("네트워크 인터페이스 초기화 완료\r\n");
  

  // 사용자로부터 Ping 대상 입력
  io_printf("\r\nPing 테스트를 실행합니다.\r\n");
  io_printf("IPv4 주소 형식으로 입력해주세요 (예: 192.168.1.1)\r\n");
  io_printf("입력>");
  cli_scanf_s("%s", ping_ip_str,sizeof(ping_ip_str));

  // Ping 실행
  //lwip_ping_test(ping_ip_str);
  ping_task(ping_ip_str);
   io_printf("\r\n[ Ethernet + Ping 테스트 종료 ]\r\n");
}
