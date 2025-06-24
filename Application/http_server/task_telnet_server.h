#ifndef TASK_TELNET_SERVER_H
#define TASK_TELNET_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

// Telnet 서버/클라이언트 설정
#define TELNET_SERVER_PORT          23
#define TELNET_MAX_CLIENTS          2
#define TELNET_BUFFER_SIZE          512
#define TELNET_RECV_TIMEOUT_MS      30000
#define TELNET_LINE_BUFFER_SIZE     256

// 클라이언트 모드 설정 (TCP 중계)
#define TELNET_RELAY_PORT           23001      // 중계 서버 포트
#define TELNET_RECONNECT_INTERVAL   5000      // 재연결 간격 (ms)

// 동작 모드
extern int g_telnet_server_mode_use;  // 0: 클라이언트 모드, 1: 서버 모드

// Telnet 프로토콜 명령어
#define TELNET_IAC          255  // Interpret As Command
#define TELNET_WILL         251  // Will option
#define TELNET_WONT         252  // Won't option
#define TELNET_DO           253  // Do option
#define TELNET_DONT         254  // Don't option
#define TELNET_SB           250  // Subnegotiation Begin
#define TELNET_SE           240  // Subnegotiation End

// Telnet 옵션
#define TELNET_OPT_ECHO     1    // Echo
#define TELNET_OPT_SGA      3    // Suppress Go Ahead
#define TELNET_OPT_NAWS     31   // Negotiate About Window Size

// Telnet 클라이언트 상태
typedef enum {
    TELNET_STATE_NORMAL,
    TELNET_STATE_IAC,
    TELNET_STATE_WILL,
    TELNET_STATE_WONT,
    TELNET_STATE_DO,
    TELNET_STATE_DONT,
    TELNET_STATE_SB,
    TELNET_STATE_SB_DATA
} telnet_state_t;

// Telnet 클라이언트 구조체 (서버 모드용)
typedef struct {
    int socket;
    bool connected;
    telnet_state_t state;
    char line_buffer[TELNET_LINE_BUFFER_SIZE];
    int line_pos;
    bool echo_enabled;
    bool sga_enabled;
    uint16_t window_width;
    uint16_t window_height;
} telnet_client_t;

// TCP 중계 클라이언트 구조체 (클라이언트 모드용)
typedef struct {
    int socket;
    bool connected;
    char relay_server_ip[16];
    uint16_t relay_server_port;
    uint32_t reconnect_count;
    telnet_state_t state;
    char line_buffer[TELNET_LINE_BUFFER_SIZE];
    int line_pos;
} tcp_relay_client_t;

// Telnet 서버 함수들
void telnet_server_task_init(void);
void noti_telnetServerTask(uint32_t flag);

// 공통 Telnet 프로토콜 처리 함수
static void telnet_process_common_data(void* client_ptr, bool is_server_mode, const uint8_t* data, int len);

// Telnet 서버 모드 함수들
static void telnet_handle_client(telnet_client_t* client);
static void telnet_send_option(int socket, uint8_t cmd, uint8_t option);
static void telnet_process_data(telnet_client_t* client, const uint8_t* data, int len);
static void telnet_send_response(telnet_client_t* client, const char* response);
static void telnet_send_prompt(telnet_client_t* client);

// Telnet 클라이언트 모드 함수들 (TCP 중계)
void telnet_client_mode_task(void);
void tcp_relay_connect(tcp_relay_client_t* client);
void tcp_relay_handle_connection(tcp_relay_client_t* client);
static void tcp_relay_process_data(tcp_relay_client_t* client, const uint8_t* data, int len);

// 설정 함수들
void telnet_set_relay_server(const char* ip, uint16_t port);

#endif // TASK_TELNET_SERVER_H