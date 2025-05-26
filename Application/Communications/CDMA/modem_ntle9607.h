


#ifndef MODEM_NTLE9607_H
#define MODEM_NTLE9607_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "task_cellular.h"
#include "modem_if.h"
#include "at_cmd.h"


#define READ_RING_NUM_BUFF_MAX 20


M_RET_t ntle9607_init(void);
M_RET_t ntle9607_read_sms(sms_t *);
M_RET_t ntle9607_send_sms(char *num,char *msg);
void ntle9607_write_ip(uint8_t ip[4],uint16_t port);
M_RET_t ntle9607_open_ppp(void);
M_RET_t ntle9607_close_ppp(void);
M_RET_t ntle9607_open_socket(void);
M_RET_t ntle9607_close_socket(void);
void ntle9607_write_ip(uint8_t ip[4],uint16_t port);
void ntle9607_reset(uint8_t resetType,uint32_t delayMs);
M_RET_t ntle9607_recv_tcp(uint8_t *buff,uint16_t buffSize,uint16_t *recvLen,uint32_t timeOutMs);
M_RET_t ntle9607_send_tcp(uint8_t *data,uint16_t dataLen);
M_RET_t ntle9607_read_num(char *prNum,uint16_t numSize);
M_RET_t ntle9607_read_rssi(int16_t *rssi);
char ntle9607_get_dtmf(char *data);
void ntle9607_vpn_init(void);
void ntle9607_off_powerSafe(void);
M_RET_t ntle9607_read_ringNum(char *data,char *prNum,uint16_t numSize); 
M_RET_t ntle_9607_recv_call(void);

M_RET_t ntle_9607_dial(char *num, uint32_t waitTimeOutMs);
M_RET_t ntle_9607_read_vpn(char *outBuffer,uint16_t outSize);
M_RET_t ntle_9607_set_vpn(char *id,char *pw,uint8_t ip[4],uint16_t port);
M_RET_t ntle_9607_at_direct(char *at,char *outBuffer,uint16_t outSize);

M_RET_t ntle9607_check_network_service(char *msgOut,uint16_t msgSize);

void ntle9607_recv_bin(void *port, char *pData, uint16_t dataLen);

extern const atCmd_t cmd_ntle9607[];

#ifdef __cplusplus
}
#endif

#endif
