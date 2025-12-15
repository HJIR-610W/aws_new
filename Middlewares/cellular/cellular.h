/**
 * @file cellular.h
 * @brief 셀룰러 모뎀 인터페이스 및 공통 정의
 * @version 1.0.0
 * @date 2025-12-13
 */

#ifndef  CELLULAR_H
#define CELLULAR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


#include "util_time.h"

typedef enum at_command_e
{
  AT_URC_TCP_DISCONNECTED = 0, //0
  AT_URC_RECV_SMS,
  AT_URC_RECV_RING,
  AT_URC_REBOOT,
  AT_URC_RECV_TCP,
  AT_URC_VOICE_END,
  AT_URC_RECV_DTMF,
  AT_SYNC_WRITE_IP_RESP,
  AT_TCP_OPEN_PPP,
  AT_TCP_CLOSE_PPP,
  AT_SYNC_OPEN_SOCKET,//10
  AT_SYNC_CLOSE_SOCKET,
  AT_TCP_SEND_DATA,
  AT_ASYNC_OPEN_VOICE,
  AT_ASYNC_OPEN_VOICE_RESP,
  AT_ASYNC_GET_RSSI,
  AT_SYNC_READ_RSSI_RESP,
  AT_SYNC_SMS_READ_RESP_OK,
  AT_ASYNC_SMS_READ_RESP_ERR,
  AT_ASYNC_SEND_RESP,
  AT_SYNC_SEND_DATA_RESP,//20
  AT_SYNC_OPEN_SOCKET_RESP_OK,
  AT_SYNC_OPEN_SOCKET_RESP_FAIL,
  AT_SYNC_OPEN_SOCKET_RESP,
  AT_SYNC_READ_NUM_RESP,
  AT_ASYNC_OFF_VOICE,
  AT_SYNC_CONNECT_VPN_RESP,
  AT_SYNC_OPEN_PPP_RESPKET,
  AT_SYNC_CLOSE_PPP_RESP,
  AT_SYNC_CLOSE_SOCKET_RESP,
  AT_ASYNC_DIAL_RESP,//30
  AT_ASYNC_DIAL_OFF,
  AT_SYNC_CONFIG_READ_RESP,
  AT_SYNC_NETWORK_SERVICE,
  AT_SYNC_SEND_TCP_RESP,
  AT_READ_NUM,
  AT_RESET_SW,
  AT_SYNC_RESET_SW_RESP,
  AT_MAX
}eAT_COMMAND_t;


typedef enum
{
  eAT_SYNC = 0,
  eAT_ASYNC,
  eAT_URC_SMS_RECV,
  eAT_URC_TCP_RECV,
  eAT_URC_CALL_RECV
}eAT_SYNC_t;

typedef struct at_comand_s
{
  eAT_COMMAND_t cmd;
  const char* cmd_string;
  eAT_SYNC_t sync_type;
}at_comand_t;

typedef struct {
  const at_comand_t* list;
  size_t count;
} at_cmd_table_t;

/* Forward declarations */
typedef struct cellular_if_t cellular_if_t;



typedef struct
{
  char number[12]; // 01012345678\0
  char message[300];
  DATE_TIME_BUF time;
}sms_t;


/* UART 인터페이스 구조체 */
typedef struct
{
  int32_t (*init)(int32_t num, void* opt, const char* owner);
  int32_t(*send)(int32_t num, const uint8_t* data, size_t dataLen);
  int32_t (*recv)(int32_t num, uint8_t* buffer, size_t rLen, uint32_t timeOutMs);
  int32_t (*recv_crlf)(int32_t num, char* buffer, size_t bSize, uint32_t tout_ms);

}uart_io_t;


/* Cellular API 함수 포인터 구조체 - 모든 함수는 첫 번째 인자로 cellular_if_t* 받음 */
typedef struct
{
  int32_t (*init)(cellular_if_t* pif);
  int32_t (*send_sms)(cellular_if_t* pif, char* num, char* msg);
  int32_t (*read_sms)(cellular_if_t* pif, sms_t* sms);
  int32_t (*read_num)(cellular_if_t* pif, char* buffer, size_t size);
  int32_t (*read_rssi)(cellular_if_t* pif, int16_t *rssi);
  int32_t (*open_tcp)(cellular_if_t* pif);
  int32_t (*close_tcp)(cellular_if_t* pif);
  int32_t (*open_ppp)(cellular_if_t* pif);
  int32_t (*close_ppp)(cellular_if_t* pif);
  int32_t (*send_tcp)(cellular_if_t* pif, uint8_t* data, size_t len);
  int32_t(*recv_tcp)(cellular_if_t* pif, uint8_t* buffer, size_t size, uint32_t timeout_ms);
  int32_t (*tcp_recv_handler)(cellular_if_t* pif, uint8_t* buffer, size_t len, size_t size);
  int32_t (*recv_call)(cellular_if_t* pif);
  int32_t (*dial)(cellular_if_t* pif, char* num, uint32_t waitTimeOutMs);
  int32_t (*vpn_init)(cellular_if_t* pif);
  int32_t (*set_vpn_config)(cellular_if_t* pif, char* id, char* pw, uint8_t ip[4], uint16_t port);
  int32_t (*read_vpn_config)(cellular_if_t* pif, char* buffer, size_t size);
  int32_t (*at_direct)(cellular_if_t* pif, const char*at_command, char* response, size_t size);
  int32_t (*check_network_service)(cellular_if_t* pif, char* buffer, size_t size);
  int32_t (*read_ring_number)(cellular_if_t* pif, const char* data, char* buffer, size_t size);
  char    (*get_dtmf)(cellular_if_t* pif, uint8_t * data);
  void    (*write_ip)(cellular_if_t* pif, uint8_t ip[4], uint16_t port);
  void    (*reset_sw)(cellular_if_t* pif);
  void    (*reset_hw)(cellular_if_t* pif);
  void    (*off_power_safe)(cellular_if_t* pif);
  int32_t(*recv_uart)(cellular_if_t* pif, uint8_t * buffer, size_t size,uint32_t timeout_ms);
}cellular_api_t;

/* resp_async_t / resp_sync_t removed: use dispatcher APIs and local buffers */

typedef struct
{
  size_t len;
  uint8_t data[512];
}tcp_data_t;


/* Cellular 인터페이스 메인 구조체 */
struct cellular_if_t
{
  uint8_t remote_ip[4];
  uint16_t remote_port;
  uint32_t rx_bytes;
  uint32_t tx_bytes;
  uint32_t error_count;
  char modem_name[32];
  int32_t uart_handle;
  uart_io_t uart_io;
  cellular_api_t api;
  at_cmd_table_t *at_cmd_table;
  void (*cdma_power_on)(void);
  void (*cdma_power_off)(void);
  int32_t cdma_power_handle;
};

#endif 
