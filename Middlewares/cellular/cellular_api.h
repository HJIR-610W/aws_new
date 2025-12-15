#ifndef CELLULAR_API_H
#define CELLULAR_API_H

#include "cellular.h"
#include <stddef.h>
#include <stdint.h>
#include "at_parser.h"

#define NTLE9607_MODEM 0
#define TX700_MODEM   1



/**
 * @file cellular_api.h
 * @brief Cellular 드라이버 Wrapper API
 *
 * 이 파일은 g_cdma_if 함수 포인터를 안전하게 호출하기 위한 wrapper 함수를 제공합니다.
 * - NULL 체크를 내부에서 처리
 * - 일관된 에러 처리
 * - 함수 포인터 직접 노출 방지
 * - 로깅/디버깅 추가 용이
 */

/* 에러 코드 정의 */
#define CELLULAR_OK                0
#define CELLULAR_ERR_NOT_INIT     -1   /* 드라이버 미초기화 */
#define CELLULAR_ERR_NULL_PTR     -2   /* NULL 포인터 */
#define CELLULAR_ERR_TIMEOUT      -3   /* 타임아웃 */
#define CELLULAR_ERR_FAILED       -4   /* 일반 오류 */

/* 초기화 및 제어 */
int32_t cellular_open(int32_t modem_number);
void cellular_reset_sw(void);
void cellular_reset_hw(void);
void cellular_off_power_safe(void);


int32_t cellular_send_sms(char* num, char* msg);
int32_t cellular_read_sms(sms_t* sms);


int32_t cellular_connect(uint8_t ip[4], uint16_t port);
int32_t cellular_disconnect(void);
int32_t cellular_send_tcp(uint8_t* data, size_t len); 
int32_t cellular_recv_tcp(uint8_t* buffer, size_t size, uint32_t timeout_ms);  
int32_t cellular_reset_data_usage(void); // 데이터 사용량 초기화 함수
int32_t cellular_get_data_usage(uint32_t* rx_bytes, uint32_t* tx_bytes); // 데이터 사용량 읽기 함수
int32_t cellular_open_ppp(void);
int32_t cellular_close_ppp(void);

/* VPN 관련 */
int32_t cellular_vpn_init(void);
int32_t cellular_set_vpn_config(char* id, char* pw, uint8_t ip[4], uint16_t port);
int32_t cellular_read_vpn_config(char* buffer, size_t size);

/* 네트워크 정보 */
int32_t cellular_read_rssi(int16_t *rssi);
int32_t cellular_read_num(char* buffer, size_t size);
int32_t cellular_check_network_service(char* buffer, size_t size);
void cellular_write_ip(uint8_t ip[4], uint16_t port);

/* 음성 통화 */
int32_t cellular_recv_call(void);
int32_t cellular_dial(char* num, uint32_t waitTimeOutMs);

char cellular_get_dtmf(uint8_t* data);


int32_t cellular_at_direct(const char* at_command, char* response, size_t size);
void cellular_set_interface(cellular_if_t* pif);

//AT parser용 TCP 수신 핸들러
int32_t cellular_tcp_recv_handler(uint8_t* buffer, size_t len, size_t size);
int32_t cellular_read_ring_number(const char* data, char* number, size_t size);

cellular_if_t* cellular_get_interface(void);



int32_t cellular_recv_uart_at(uint8_t* buffer, size_t size, uint32_t timeout_ms);
void cellular_reset_hw(void);
void cellular_init(void);

at_cmd_table_t* cellular_get_at_cmd_table(void);

/* URC callback setters - application should call these to register handlers */
void cellular_set_ring_task(at_parser_task_fn_t fn);
void cellular_set_sms_task(at_parser_task_fn_t fn);
void cellular_subscribe(const char* prefix, void (*cb)(const uint8_t*, size_t, void*), void* ctx);



void cellular_set_sms_callback(at_parser_task_fn_t cb);
void cellular_set_call_callback(at_parser_task_fn_t cb);
void cellular_set_reboot_callback(void (*cb)(const uint8_t*, size_t, void*));
void cellular_set_disconnect_callback(void (*cb)(const uint8_t*, size_t, void*));
void cellular_set_dtmf_callback(void (*cb)(const uint8_t*, size_t, void*));


#endif /* CELLULAR_API_H */
