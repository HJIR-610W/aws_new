/**
 * @file cellular_api.c
 * @brief Cellular 드라이버 Wrapper API 구현
 * @version 1.0.0
 * @date 2025-12-13
 */

#include "cellular_api.h"
#include <stdio.h>

#include "at_parser.h"
#include "cellular_hal.h"
#include "dispatcher.h"
#include "cdma/ntle9607.h"
#include "cdma/tx700.h"
#include "util_memory.h"
#include "drv_rs232.h"
#include "system_err.h"
#include "drv_power.h"


static cellular_if_t* g_cellular_if = NULL;
struct cellular_if_t g_cdma_if;


/**
 * @brief Cellular 인터페이스 설정
 * @param pif cellular_if_t 구조체 포인터
 */
void cellular_set_interface(cellular_if_t* pif)
{
  g_cellular_if = pif;
  if (pif != NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스 설정: %s", pif->modem_name);
  }
}

/**
 * @brief 전역 인터페이스 포인터 반환
 */
cellular_if_t* cellular_get_interface(void)
{
  return g_cellular_if;
}


void cellular_init(void)
{
  g_cellular_if->api.init(g_cellular_if);
}


/**
 * @brief 모뎀 초기화
 */
int32_t cellular_open(int32_t modem_number)
{
  int ret=-1;

  uart_config_t uart_cfg = {
  .baud = 57600,
  .parity_index =0
  };

  // UART I/O 함수 포인터 설정
  g_cdma_if.uart_io.init = drv_uart_init;
  g_cdma_if.uart_io.recv_crlf = drv_uart_recv_crlf;
  g_cdma_if.uart_io.recv = drv_uart_recv;
  g_cdma_if.uart_io.send = drv_uart_send;




  // UART 초기화
  if (g_cdma_if.uart_io.init(DRV_UART_8_CDMA,&uart_cfg,NULL) != 0) {
    return -1;
  }
  g_cdma_if.uart_handle = DRV_UART_8_CDMA;

  if (modem_number == NTLE9607_MODEM)
  {
    ret = ntle9607_open(&g_cdma_if);
    g_cellular_if = &g_cdma_if;
    cellular_set_interface(g_cellular_if);
  }
  else if(modem_number == TX700_MODEM)
  {
    ret = tx700_open(&g_cdma_if);
    g_cellular_if = &g_cdma_if;
    cellular_set_interface(g_cellular_if);
  }
  else
  {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"지원하지 않는 모뎀 번호: %d", modem_number);
    return CELLULAR_ERR_NOT_INIT;
  }


  if (ret != 0)
  {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Modem driver initialization failed");
    return CELLULAR_ERR_NOT_INIT;
  }

  dispatcher_init();
  at_parser_start();


  return  CELLULAR_OK ;
}

/**
 * @brief 소프트웨어 리셋
 */
void cellular_reset_sw(void)
{
  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return;
  }

  if (g_cellular_if->api.reset_sw == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] reset_sw 함수가 초기화되지 않음");
    return;
  }

  g_cellular_if->api.reset_sw(g_cellular_if);
}

/**
 * @brief 하드웨어 리셋
 */
void cellular_reset_hw(void)
{
  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return;
  }

  if (g_cellular_if->api.reset_hw == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] reset_hw 함수가 초기화되지 않음");
    return;
  }

  g_cellular_if->api.reset_hw(g_cellular_if);
}

/**
 * @brief 안전한 전원 차단
 */
void cellular_off_power_safe(void)
{
  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return;
  }

  if (g_cellular_if->api.off_power_safe == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] off_power_safe 함수가 초기화되지 않음");
    return;
  }

  g_cellular_if->api.off_power_safe(g_cellular_if);
}

/**
 * @brief SMS 전송
 */
int32_t cellular_send_sms(char* num, char* msg)
{
  int32_t ret;

  if (num == NULL || msg == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.send_sms == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] send_sms 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.send_sms(g_cellular_if, num, msg);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief SMS 읽기
 */
int32_t cellular_read_sms(sms_t* sms)
{
  int32_t ret;

  if (sms == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.read_sms == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] read_sms 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.read_sms(g_cellular_if, sms);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief TCP 소켓 열기
 */
int32_t cellular_open_tcp(void)
{
  int32_t ret;

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.open_tcp == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] open_tcp 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.open_tcp(g_cellular_if);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief TCP 소켓 닫기
 */
int32_t cellular_disconnect(void)
{

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.close_tcp == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] close_tcp 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }
  return g_cellular_if->api.close_tcp(g_cellular_if);
}




/**
 * @brief TCP 수신 핸들러 tcp 데이터 추출
 */
int32_t cellular_tcp_recv_handler(uint8_t* buffer, size_t len, size_t size)
{
  int32_t ret;

  if (buffer == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.tcp_recv_handler == NULL) {
    /* TX700은 이 함수가 없을 수 있음 */
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.tcp_recv_handler(g_cellular_if, buffer, len, size);
  return ret;
}



int32_t cellular_connect(uint8_t ip[4], uint16_t port)
{
  int ret;
#if 0 
  uint8_t ip[4] = {0};

  if (sscanf(ipv4_string, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) != 4) {
    return CELLULAR_ERR_FAILED;
  }
#endif
  g_cellular_if->remote_ip[0] = ip[0];
  g_cellular_if->remote_ip[1] = ip[1];
  g_cellular_if->remote_ip[2] = ip[2];
  g_cellular_if->remote_ip[3] = ip[3];

  g_cellular_if->remote_port = port;




  cellular_write_ip(ip, port);
  ret = cellular_open_tcp();

  if (ret == 0) {

  }
  else {
    ret = CELLULAR_CONN_ERR_REFUSED;
  }

  return ret;
}

/**
 * @brief TCP 데이터 전송 (단일 세션)
 * @return 
 */
int32_t cellular_send_tcp(uint8_t* data, size_t len)
{
  // TCP 데이터 전송을 시도합니다.
  int32_t bytes_sent = g_cellular_if->api.send_tcp(g_cellular_if, data, len);

  // 전송이 성공하면 (0 이상의 값) tx_bytes에 합산합니다.
  if (bytes_sent >= 0) {
    g_cellular_if->tx_bytes += bytes_sent;
  }
  return bytes_sent;
}

int32_t cellular_recv_tcp(uint8_t* buffer, size_t len, uint32_t timeout_ms)
{
  // TCP 데이터 수신을 시도합니다.
  int32_t bytes_received = g_cellular_if->api.recv_tcp(g_cellular_if, buffer, len, timeout_ms);

  // 수신이 성공하면 (0 이상의 값) rx_bytes에 합산합니다.
  if (bytes_received >= 0) {
    g_cellular_if->rx_bytes += bytes_received;
  }
  return bytes_received;
}

/**
 * @brief 데이터 사용량 (rx_bytes, tx_bytes)을 0으로 초기화합니다.
 */
int32_t cellular_reset_data_usage(void)
{
  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }
  g_cellular_if->rx_bytes = 0;
  g_cellular_if->tx_bytes = 0;
   DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 데이터 사용량 초기화됨 (rx_bytes: %lu, tx_bytes: %lu)",
               g_cellular_if->rx_bytes, g_cellular_if->tx_bytes);
  return CELLULAR_OK;
}

/**
 * @brief 현재 데이터 사용량 (rx_bytes, tx_bytes)을 읽어옵니다.
 * @param rx_bytes 수신 바이트를 저장할 포인터
 * @param tx_bytes 송신 바이트를 저장할 포인터
 * @return 성공 시 CELLULAR_OK, 실패 시 에러 코드
 */
int32_t cellular_get_data_usage(uint32_t* rx_bytes, uint32_t* tx_bytes)
{
  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }
  if (rx_bytes == NULL || tx_bytes == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] NULL 포인터 에러");
    return CELLULAR_ERR_NULL_PTR;
  }

  *rx_bytes = g_cellular_if->rx_bytes;
  *tx_bytes = g_cellular_if->tx_bytes;

  return CELLULAR_OK;
}

/**
 * @brief PPP 연결 열기
 */
int32_t cellular_open_ppp(void)
{
  int32_t ret;

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.open_ppp == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] open_ppp 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.open_ppp(g_cellular_if);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief PPP 연결 닫기
 */
int32_t cellular_close_ppp(void)
{
  int32_t ret;

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.close_ppp == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] close_ppp 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.close_ppp(g_cellular_if);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief VPN 초기화
 */
int32_t cellular_vpn_init(void)
{
  int32_t ret;

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.vpn_init == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] vpn_init 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.vpn_init(g_cellular_if);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief VPN 설정
 */
int32_t cellular_set_vpn_config(char* id, char* pw, uint8_t ip[4], uint16_t port)
{
  int32_t ret;

  if (id == NULL || pw == NULL || ip == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.set_vpn_config == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] set_vpn_config 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.set_vpn_config(g_cellular_if, id, pw, ip, port);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief VPN 설정 읽기
 */
int32_t cellular_read_vpn_config(char* buffer, size_t size)
{
  int32_t ret;

  if (buffer == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.read_vpn_config == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] read_vpn_config 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.read_vpn_config(g_cellular_if, buffer, size);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief RSSI (신호 강도) 읽기
 */
int32_t cellular_read_rssi(int16_t *rssi)
{
  int32_t ret;

  if (rssi == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.read_rssi == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] read_rssi 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.read_rssi(g_cellular_if, rssi);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief 전화번호 읽기
 */
int32_t cellular_read_num(char* buffer, size_t size)
{
  int32_t ret;

  if (buffer == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.read_num == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] read_num 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.read_num(g_cellular_if, buffer, size);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief IP 주소 및 포트 설정
 */
void cellular_write_ip(uint8_t ip[4], uint16_t port)
{
  if (ip == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] IP 주소가 NULL");
    return;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return;
  }

  if (g_cellular_if->api.write_ip == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] write_ip 함수가 초기화되지 않음");
    return;
  }

  g_cellular_if->api.write_ip(g_cellular_if, ip, port);
}

/**
 * @brief 네트워크 서비스 상태 확인
 */
int32_t cellular_check_network_service(char* buffer, size_t size)
{
  int32_t ret;

  if (buffer == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.check_network_service == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] check_network_service 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.check_network_service(g_cellular_if, buffer, size);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief 전화 수신
 */
int32_t cellular_recv_call(void)
{
  int32_t ret;

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.recv_call == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] recv_call 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.recv_call(g_cellular_if);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief 전화 걸기
 */
int32_t cellular_dial(char* num, uint32_t waitTimeOutMs)
{
  int32_t ret;

  if (num == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.dial == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] dial 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.dial(g_cellular_if, num, waitTimeOutMs);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief Ring 수신 시 전화번호 읽기
 */
int32_t cellular_read_ring_number(const char* data, char* buffer, size_t size)
{
  int32_t ret;

  if (data == NULL || buffer == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.read_ring_number == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] read_ring_number 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.read_ring_number(g_cellular_if, data, buffer, size);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}

/**
 * @brief DTMF 톤 가져오기
 */
char cellular_get_dtmf(uint8_t* data)
{
  char dtmf;

  if (data == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] DTMF 데이터가 NULL");
    return 0;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.get_dtmf == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] get_dtmf 함수가 초기화되지 않음");
    return 0;
  }

  dtmf = g_cellular_if->api.get_dtmf(g_cellular_if, data);
  return dtmf;
}

/**
 * @brief AT 명령 직접 실행
 */
int32_t cellular_at_direct(const char* at_command, char* response, size_t size)
{
  int32_t ret;

  if (at_command == NULL || response == NULL) {
    return CELLULAR_ERR_NULL_PTR;
  }

  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  if (g_cellular_if->api.at_direct == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] at_direct 함수가 초기화되지 않음");
    return CELLULAR_ERR_NOT_INIT;
  }

  ret = g_cellular_if->api.at_direct(g_cellular_if, at_command, response, size);
  return (ret == 0) ? CELLULAR_OK : CELLULAR_ERR_FAILED;
}





at_cmd_table_t *cellular_get_at_cmd_table(void)
{
  if (g_cellular_if == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] 인터페이스가 설정되지 않음");
    return NULL;
  }
  return g_cellular_if->at_cmd_table;
}



void cellular_set_ring_task(at_parser_task_fn_t fn)
{
 at_parser_set_ring_task(fn);
}

void cellular_set_sms_task(at_parser_task_fn_t fn)
{
 at_parser_set_sms_task(fn);
}

void cellular_subscribe(const char* prefix, void (*cb)(const uint8_t*, size_t, void*), void* ctx)
{
  dispatcher_subscribe(prefix, cb, ctx);
}



void cellular_set_sms_callback(at_parser_task_fn_t cb)
{
  at_parser_set_sms_task(cb);
}


void cellular_set_call_callback(at_parser_task_fn_t cb)
{
  at_parser_set_ring_task(cb);

}

void cellular_set_reboot_callback(void (*cb)(const uint8_t*, size_t, void*))
{
  for (uint32_t i = 0; i < cellular_get_at_cmd_table()->count; i++)
  {
    if (cellular_get_at_cmd_table()->list[i].cmd == AT_URC_REBOOT)
    {
      const char* prefix = cellular_get_at_cmd_table()->list[i].cmd_string;
      dispatcher_subscribe(prefix, cb, NULL);
      return;
    }
  }

}

void cellular_set_disconnect_callback(void (*cb)(const uint8_t*, size_t, void*))
{
  for (uint32_t i = 0; i < cellular_get_at_cmd_table()->count; i++)
  {
    if (cellular_get_at_cmd_table()->list[i].cmd == AT_URC_TCP_DISCONNECTED)
    {
      const char* prefix = cellular_get_at_cmd_table()->list[i].cmd_string;
      dispatcher_subscribe(prefix, cb, NULL);
      return;
    }
  }

}

void cellular_set_dtmf_callback(void (*cb)(const uint8_t*, size_t, void*))
{
  for (uint32_t i = 0; i < cellular_get_at_cmd_table()->count; i++)
  {
    if (cellular_get_at_cmd_table()->list[i].cmd == AT_URC_RECV_DTMF)
    {
      const char* prefix = cellular_get_at_cmd_table()->list[i].cmd_string;
      dispatcher_subscribe(prefix, cb, NULL);
      return;
    }
  }

}

int32_t cellular_recv_uart_at(uint8_t *buffer, size_t size,uint32_t timeout_ms)
{
  if (g_cellular_if == NULL) {
    return 0;
  }
  if (g_cellular_if->api.recv_uart == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"[Cellular API] drv_uart_recv 함수가 초기화되지 않음");
    return 0;
  }

    return g_cellular_if->api.recv_uart(g_cellular_if, buffer, size, timeout_ms);


}