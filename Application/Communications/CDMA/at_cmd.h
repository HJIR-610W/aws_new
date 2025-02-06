
#ifndef AT_CMD_H
#define AT_CMD_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdint.h>

enum
{
    AT_ASYNC_RESP_TCP_DISCONNECTED=0,
    AT_ASYNC_RESP_SMS_RECEIVED,
    AT_ASYNC_RESP_RING_RECEIVED,
    AT_ASYNC_RESP_REBOOT,
    AT_ASYNC_RESP_TCP_RECV,
    AT_ASYNC_RESP_VOICE_END,
    AT_ASYNC_RESP_DTMF,
    AT_TCP_WRITE_IP,
    AT_TCP_OPEN_PPP,
    AT_TCP_CLOSE_PPP,
    AT_TCP_OPEN_SOCKET,
    AT_TCP_CLOSE_SOCKET,
    AT_TCP_SEND_DATA,
    AT_ASYNC_OPEN_VOICE,
    AT_ASYNC_OPEN_VOICE_RESP,
    AT_ASYNC_GET_RSSI,
    AT_ASYNC_GET_RSSI_RESP,
    AT_ASYNC_SMS_READ_RESP_OK,
    AT_ASYNC_SMS_READ_RESP_ERR,
    AT_SMS_SEND_RESP_OK,
    AT_TCP_SEND_DATA_RESP,
    AT_TCP_OPEN_SOCKET_RESP_OK,
    AT_TCP_OPEN_SOCKET_RESP_FAIL,
    AT_TCP_OPEN_SOCKET_RESP,
    AT_TCP_READ_NUM_RESP,
    AT_ASYNC_OFF_VOICE,
    AT_TCP_CONNECT_VPN_RESP,
    AT_TCP_OPEN_PPP_RESP,
    AT_TCP_CLOSE_PPP_RESP,
    AT_TCP_CLOSE_SOCKET_RESP,
    AT_TCP_RESET_SW_RESP,
    AT_ASYNC_DIAL_RESP,
    AT_ASYNC_DIAL_OFF,
    AT_ASYNC_CONFIG_READ_RESP,
    AT_MAX
};



    

    typedef struct atCmd_s
    {
        uint32_t cmd;
        char *cmdStr;   // at 명령어 또는 응답
        void (*fsend)(void);
    }atCmd_t;

#ifdef __cplusplus
}
#endif

#endif