#ifndef  NTLE9607_H
#define NTLE9607_H

#include <stddef.h>
#include <stdint.h>
#include "cellular.h"


/* 드라이버 초기화/정리 함수 */
int32_t ntle9607_open(cellular_if_t *pif);
void ntle9607_close(cellular_if_t *pif);

/* Cellular API 함수 선언 */
int32_t ntle9607_init(cellular_if_t* pif);
int32_t ntle9607_send_sms(cellular_if_t* pif, char* num, char* msg);
int32_t ntle9607_read_sms(cellular_if_t* pif, sms_t* sms);
int32_t ntle9607_read_num(cellular_if_t* pif, char* buffer, size_t size);
int32_t ntle9607_read_rssi(cellular_if_t* pif, int16_t *rssi);
int32_t ntle9607_open_tcp(cellular_if_t* pif);
int32_t ntle9607_close_tcp(cellular_if_t* pif);
int32_t ntle9607_open_ppp(cellular_if_t* pif);
int32_t ntle9607_close_ppp(cellular_if_t* pif);
int32_t ntle9607_send_tcp(cellular_if_t* pif, uint8_t* data, size_t len);
int32_t ntle9607_tcp_recv_handler(cellular_if_t* pif, uint8_t* buffer, size_t len,  size_t size);
int32_t ntle9607_recv_tcp(cellular_if_t* pif, uint8_t* buffer, size_t size, uint32_t timeout_ms);
int32_t ntle9607_recv_call(cellular_if_t* pif);
int32_t ntle9607_dial(cellular_if_t* pif, char* num, uint32_t waitTimeOutMs);
int32_t ntle9607_vpn_init(cellular_if_t* pif);
int32_t ntle9607_set_vpn_config(cellular_if_t* pif, char* id, char* pw, uint8_t ip[4], uint16_t port);
int32_t ntle9607_read_vpn_config(cellular_if_t* pif, char* buffer, size_t size);
int32_t ntle9607_at_direct(cellular_if_t* pif, const char* at_command, char* response, size_t size);
int32_t ntle9607_check_network_service(cellular_if_t* pif, char* buffer, size_t size);
int32_t ntle9607_read_ring_number(cellular_if_t* pif, const char* data, char* buffer, size_t size);
char    ntle9607_get_dtmf(cellular_if_t* pif, uint8_t* data);
void    ntle9607_write_ip(cellular_if_t* pif, uint8_t ip[4], uint16_t port);
void    ntle9607_reset_sw(cellular_if_t* pif);
void    ntle9607_reset_hw(cellular_if_t* pif);
void    ntle9607_off_power_safe(cellular_if_t* pif);


extern const at_cmd_table_t ntle9607_cmd_table;

#endif // ! NTLE9607_H
