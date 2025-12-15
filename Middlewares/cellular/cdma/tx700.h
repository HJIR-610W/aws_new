#ifndef TX700_H
#define TX700_H

#include <stddef.h>
#include <stdint.h>
#include "cellular.h"


/* 드라이버 초기화/정리 함수 */
int32_t tx700_open(cellular_if_t *pif);
void tx700_close(cellular_if_t *pif);

/* Cellular API 함수 선언 */
int32_t tx700_init(cellular_if_t* p_if);
int32_t tx700_send_sms(cellular_if_t* p_if, char* p_number_str, char* p_message_str);
int32_t tx700_read_sms(cellular_if_t* p_if, sms_t* p_sms);
int32_t tx700_read_num(cellular_if_t* p_if, char* p_buffer, size_t buffer_size);
int32_t tx700_read_rssi(cellular_if_t* p_if, int16_t *p_rssi_value);
int32_t tx700_open_tcp(cellular_if_t* p_if);
int32_t tx700_close_tcp(cellular_if_t* p_if);
int32_t tx700_open_ppp(cellular_if_t* p_if);
int32_t tx700_close_ppp(cellular_if_t* p_if);
int32_t tx700_send_tcp(cellular_if_t* p_if, uint8_t* p_data_buffer, size_t data_length);
int32_t tx700_recv_tcp(cellular_if_t* p_if, uint8_t* p_buffer, size_t buffer_size, uint32_t timeout_ms);
int32_t tx700_recv_call(cellular_if_t* p_if);
int32_t tx700_dial(cellular_if_t* p_if, char* p_number_str, uint32_t timeout_ms_val);
int32_t tx700_vpn_init(cellular_if_t* p_if);
int32_t tx700_set_vpn_config(cellular_if_t* p_if, char* p_id_str, char* p_password_str, uint8_t p_ip_address[4], uint16_t port_num);
int32_t tx700_read_vpn_config(cellular_if_t* p_if, char* p_buffer, size_t buffer_size);
int32_t tx700_at_direct(cellular_if_t* p_if, const char* p_at_command_str, char* p_response_buffer_out, size_t buffer_size_out);
int32_t tx700_check_network_service(cellular_if_t* p_if, char* p_buffer, size_t buffer_size);
int32_t tx700_read_ring_number(cellular_if_t* p_if, const char* p_data_buffer, char* p_buffer_out, size_t buffer_size_out);
char    tx700_get_dtmf(cellular_if_t* p_if, uint8_t* p_data_buffer);
void    tx700_write_ip(cellular_if_t* p_if, uint8_t p_ip_address[4], uint16_t port_num);
void    tx700_reset_sw(cellular_if_t* p_if);
void    tx700_reset_hw(cellular_if_t* p_if);
void    tx700_off_power_safe(cellular_if_t* p_if);


extern const at_cmd_table_t tx700_cmd_table;

#endif // ! TX700_H
