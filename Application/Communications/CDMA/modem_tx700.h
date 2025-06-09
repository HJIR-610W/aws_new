


#ifndef MODEM_tx700_H
#define MODEM_tx700_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "task_cellular.h"
#include "modem_if.h"
#include "at_cmd.h"


#define READ_RING_NUM_BUFF_MAX 20


M_RET_t tx700_init(void);
M_RET_t tx700_read_sms(sms_t *);
M_RET_t tx700_send_sms(char *num,char *msg);
void tx700_write_ip(uint8_t ip[4],uint16_t port);
M_RET_t tx700_open_ppp(void);
M_RET_t tx700_close_ppp(void);
M_RET_t tx700_open_socket(void);
M_RET_t tx700_close_socket(void);
void tx700_write_ip(uint8_t ip[4],uint16_t port);
void tx700_reset(uint8_t resetType,uint32_t delayMs);
M_RET_t tx700_recv_tcp(uint8_t *buff,uint16_t buffSize,uint16_t *recvLen,uint32_t timeOutMs);
M_RET_t tx700_send_tcp(uint8_t *data,uint16_t dataLen);
M_RET_t tx700_read_num(char *prNum,uint16_t numSize);
M_RET_t tx700_read_rssi(int16_t *rssi);
char tx700_get_dtmf(char *data);
void tx700_vpn_init(void);
void tx700_off_powerSafe(void);
M_RET_t tx700_read_ring_number(char *data,char *prNum,uint16_t numSize); 
M_RET_t tx700_recv_call(void);

M_RET_t tx700_dial(char *num, uint32_t waitTimeOutMs);
M_RET_t tx700_read_vpn(char *outBuffer,uint16_t outSize);
M_RET_t tx700_set_vpn(char *id,char *pw,uint8_t ip[4],uint16_t port);
M_RET_t tx700_at_direct(char *at,char *outBuffer,uint16_t outSize);

M_RET_t tx700_check_network_service(char *msgOut,uint16_t msgSize);
void tx700_recv_bin(driver_t *port, uint8_t *p_data, uint16_t data_len);
uint32_t tx700_get_count(void) ;
int32_t tx700_recv_handler(driver_t *uart,uint8_t *buffer, uint16_t buffer_size);

    extern const atCmd_t cmd_tx700[36];

#ifdef __cplusplus
}
#endif

#endif
