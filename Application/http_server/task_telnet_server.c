#include "task_telnet_server.h"
#include "terminal_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmsis_os2.h"
#include "lwip.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "app_logging.h"
#include "task_logging.h"
#include "user_heap.h"
#include "util_time.h"

#define SERVER_RETRY_INTERVAL_MS 5000
#define TELNET_WELCOME_MSG "\r\n=== AWS Weather Station Telnet Console ===\r\n\r\n"

// 전역 변수
int g_telnet_server_mode_use = 0;  // 0: 클라이언트 모드, 1: 서버 모드

static osThreadId_t g_telnetServerTaskId = NULL;


// 서버 모드용 변수
static telnet_client_t g_telnet_clients[TELNET_MAX_CLIENTS];

// 클라이언트 모드용 변수 (TCP 중계)
static tcp_relay_client_t g_tcp_relay_client = {0};
static bool g_tcp_relay_enabled = false;

const osThreadAttr_t telnet_server_task_attributes = {
    .name = "telnet_server",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityRealtime2,
};

static void telnet_server_task(void *argument);
static void telnet_handle_client(telnet_client_t* client);
static void telnet_send_option(int socket, uint8_t cmd, uint8_t option);
static void telnet_process_data(telnet_client_t* client, const uint8_t* data, int len);
static void telnet_send_response(telnet_client_t* client, const char* response);
static void telnet_send_prompt(telnet_client_t* client);
static void telnet_init_client(telnet_client_t* client, int socket);
static void telnet_cleanup_client(telnet_client_t* client);
static telnet_client_t* telnet_find_free_client(void);
static void telnet_negotiate_options(telnet_client_t* client);
static void telnet_server_mode_task(void);
static void tcp_relay_output_callback(const char* data, size_t len);

void noti_telnetServerTask(uint32_t flag)
{
    if(g_telnetServerTaskId != NULL) {
        osThreadFlagsSet(g_telnetServerTaskId, flag);
    }
}

static int set_recv_timeout(int sockfd, uint32_t timeout_ms)
{
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        task_printf("Telnet Server: SO_RCVTIMEO setting failed, error: %d\r\n", errno);
        return -1;
    }
    return 0;
}

static void telnet_init_client(telnet_client_t* client, int socket)
{
    memset(client, 0, sizeof(telnet_client_t));
    client->socket = socket;
    client->connected = true;
    client->state = TELNET_STATE_NORMAL;
    client->echo_enabled = true;
    client->sga_enabled = true;
    client->window_width = 80;
    client->window_height = 24;
    client->line_pos = 0;
    
    task_printf("Telnet: Client initialized (socket: %d)\r\n", socket);
}

static void telnet_cleanup_client(telnet_client_t* client)
{
    if (client->connected) {
        closesocket(client->socket);
        client->connected = false;
        task_printf("Telnet: Client cleaned up (socket: %d)\r\n", client->socket);
    }
    memset(client, 0, sizeof(telnet_client_t));
    client->socket = -1;
}

static telnet_client_t* telnet_find_free_client(void)
{
    for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
        if (!g_telnet_clients[i].connected) {
            return &g_telnet_clients[i];
        }
    }
    return NULL;
}

static void telnet_send_option(int socket, uint8_t cmd, uint8_t option)
{
    uint8_t buf[3] = {TELNET_IAC, cmd, option};
    send(socket, buf, 3, 0);
    
    task_printf("Telnet: Sent option - IAC %d %d\r\n", cmd, option);
}

static void telnet_negotiate_options(telnet_client_t* client)
{
    // Echo를 서버에서 처리하도록 설정
    telnet_send_option(client->socket, TELNET_WILL, TELNET_OPT_ECHO);
    
    // Suppress Go Ahead 활성화
    telnet_send_option(client->socket, TELNET_WILL, TELNET_OPT_SGA);
    telnet_send_option(client->socket, TELNET_DO, TELNET_OPT_SGA);
    
    // Window size 협상 요청
    telnet_send_option(client->socket, TELNET_DO, TELNET_OPT_NAWS);
    
    task_printf("Telnet: Options negotiated for client\r\n");
}

static void telnet_send_response(telnet_client_t* client, const char* response)
{
    if (!client->connected || !response) {
        return;
    }
    
    send(client->socket, response, strlen(response), 0);
}

static void telnet_send_prompt(telnet_client_t* client)
{
    if (!client->connected) {
        return;
    }
    
    const char* prompt = "AWS$ ";
    telnet_send_response(client, prompt);
}

// Telnet 브리지에서 콘솔 출력을 받는 콜백 함수
static telnet_client_t* g_current_telnet_client = NULL;

static void telnet_output_callback(const char* data, size_t len)
{
    if (g_current_telnet_client && g_current_telnet_client->connected && data && len > 0) {
        send(g_current_telnet_client->socket, data, len, 0);
    }
}

static void telnet_process_data(telnet_client_t* client, const uint8_t* data, int len)
{
    telnet_process_common_data(client, true, data, len);
}

static void telnet_handle_client(telnet_client_t* client)
{
    uint8_t* buffer = (uint8_t*)aws_malloc(TELNET_BUFFER_SIZE);
    if (buffer == NULL) {
        task_printf("Telnet: Failed to allocate buffer for client\r\n");
        return;
    }
    
    // 수신 타임아웃 설정
    if (set_recv_timeout(client->socket, TELNET_RECV_TIMEOUT_MS) < 0) {
        task_printf("Telnet: Failed to set recv timeout for client\r\n");
        aws_free(buffer);
        return;
    }
    
    // 옵션 협상
    telnet_negotiate_options(client);
    
    // 환영 메시지 전송
    telnet_send_response(client, TELNET_WELCOME_MSG);
    telnet_send_prompt(client);
    
    // 터미널 브리지 초기화 및 콜백 설정
    terminal_bridge_init();
    terminal_bridge_set_output_callback(telnet_output_callback);
    
    task_printf("Telnet: Client handler started (socket: %d)\r\n", client->socket);
    
    while (client->connected) {
        int bytes_received = recv(client->socket, buffer, TELNET_BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                task_printf("Telnet: Client disconnected normally\r\n");
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                task_printf("Telnet: Client timeout\r\n");
            } else {
                task_printf("Telnet: Client recv error: %d\r\n", errno);
            }
            break;
        }
        
        task_printf("Telnet: Received %d bytes from client\r\n", bytes_received);
        telnet_process_data(client, buffer, bytes_received);
    }
    
    // 정리
    g_current_telnet_client = NULL;
    terminal_bridge_cleanup();
    aws_free(buffer);
    task_printf("Telnet: Client handler terminated\r\n");
}

static void telnet_server_task(void *argument)
{
    (void)argument;

    osThreadFlagsWait(0x00000001, osFlagsWaitAny, osWaitForever);
    
    // 동작 모드에 따라 분기
    if (g_telnet_server_mode_use == 0) {
        task_printf("Telnet: Starting in CLIENT mode (relay connection)\r\n");
        telnet_client_mode_task();
    } else {
        task_printf("Telnet: Starting in SERVER mode (direct connection)\r\n");
        telnet_server_mode_task();
    }
}

static void telnet_server_mode_task(void)
{
    int server_socket = -1;
    int client_socket = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int opt = 1;

    // 클라이언트 배열 초기화
    for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
        g_telnet_clients[i].socket = -1;
        g_telnet_clients[i].connected = false;
    }

    task_printf("Telnet Server: Starting on port %d\r\n", TELNET_SERVER_PORT);
       
    while (1) {
        if (server_socket < 0) {
            server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (server_socket < 0) {
                task_printf("Telnet Server: Socket creation failed, error: %d\r\n", errno);
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                task_printf("Telnet Server: SO_REUSEADDR failed, error: %d\r\n", errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(TELNET_SERVER_PORT);
            server_addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                task_printf("Telnet Server: Bind failed on port %d, error: %d\r\n", TELNET_SERVER_PORT, errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            if (listen(server_socket, TELNET_MAX_CLIENTS) < 0) {
                task_printf("Telnet Server: Listen failed, error: %d\r\n", errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            task_printf("Telnet Server: Listening on port %d (socket: %d)\r\n", TELNET_SERVER_PORT, server_socket);
        }

        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            if (errno == ECONNABORTED || errno == EINVAL) {
                closesocket(server_socket);
                server_socket = -1;
            }
            osDelay(100);
            continue;
        }

        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, sizeof(client_ip_str));
        task_printf("Telnet Server: New client connected %s:%u (socket: %d)\r\n", 
                   client_ip_str, ntohs(client_addr.sin_port), client_socket);

        // 빈 클라이언트 슬롯 찾기
        telnet_client_t* client = telnet_find_free_client();
        if (client == NULL) {
            task_printf("Telnet Server: No free client slots, rejecting connection\r\n");
            const char* reject_msg = "Server full. Please try again later.\r\n";
            send(client_socket, reject_msg, strlen(reject_msg), 0);
            closesocket(client_socket);
            continue;
        }

        // 클라이언트 초기화 및 처리
        telnet_init_client(client, client_socket);
        telnet_handle_client(client);
        telnet_cleanup_client(client);

        task_printf("Telnet Server: Client disconnected\r\n");
    }

    //if (server_socket >= 0) {
      //  closesocket(server_socket);
    //}


}

// =================================================================
// 클라이언트 모드 구현 (중계 서버 연결)
// =================================================================

static void telnet_client_mode_task(void)
{
    // 기본 중계 서버 설정
    if (!g_tcp_relay_enabled) {
        strcpy(g_tcp_relay_client.relay_server_ip, "192.168.1.174");
        g_tcp_relay_client.relay_server_port = TELNET_RELAY_PORT;
        g_tcp_relay_enabled = true;
    }
    
    // 터미널 브리지 초기화
    terminal_bridge_init();
    terminal_bridge_set_output_callback(tcp_relay_output_callback);
    
    task_printf("Telnet Client: Starting TCP relay to %s:%d\r\n", 
               g_tcp_relay_client.relay_server_ip, g_tcp_relay_client.relay_server_port);
    
    while (1) {
        if (!g_tcp_relay_client.connected) {
            tcp_relay_connect(&g_tcp_relay_client);
        }
        
        if (g_tcp_relay_client.connected) {
            tcp_relay_handle_connection(&g_tcp_relay_client);
        }
        
        osDelay(TELNET_RECONNECT_INTERVAL);
    }
}

static void tcp_relay_connect(tcp_relay_client_t* client)
{
    struct sockaddr_in server_addr;
    
    task_printf("TCP Relay: Attempting connection to %s:%d (attempt %lu)\r\n", 
               client->relay_server_ip, client->relay_server_port, client->reconnect_count + 1);
    
    client->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client->socket < 0) {
        task_printf("TCP Relay: Socket creation failed, error: %d\r\n", errno);
        return;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client->relay_server_port);
    
    if (inet_pton(AF_INET, client->relay_server_ip, &server_addr.sin_addr) <= 0) {
        task_printf("TCP Relay: Invalid IP address: %s\r\n", client->relay_server_ip);
        closesocket(client->socket);
        client->socket = -1;
        return;
    }
    
    if (connect(client->socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        task_printf("TCP Relay: Connection failed, error: %d\r\n", errno);
        closesocket(client->socket);
        client->socket = -1;
        client->reconnect_count++;
        return;
    }
    
    client->connected = true;
    client->reconnect_count = 0;
    client->state = TELNET_STATE_NORMAL;
    client->line_pos = 0;
    
    task_printf("TCP Relay: Connected successfully - ready for Telnet forwarding\r\n");
}

static void tcp_relay_handle_connection(tcp_relay_client_t* client)
{
    uint8_t* buffer = (uint8_t*)aws_malloc(TELNET_BUFFER_SIZE);
    if (buffer == NULL) {
        task_printf("TCP Relay: Failed to allocate buffer\r\n");
        return;
    }
    
    if (set_recv_timeout(client->socket, 5000) < 0) {
        task_printf("TCP Relay: Failed to set recv timeout\r\n");
        aws_free(buffer);
        return;
    }
    
    task_printf("TCP Relay: Connection established - processing Telnet data\r\n");
    
    while (client->connected) {
        // 중계서버에서 데이터 수신 (Telnet 프로토콜)
        int bytes_received = recv(client->socket, buffer, TELNET_BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                task_printf("TCP Relay: Server closed connection\r\n");
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue; // 타임아웃, 계속 시도
            } else {
                task_printf("TCP Relay: Recv error: %d\r\n", errno);
            }
            break;
        }
        
        task_printf("TCP Relay: Received %d bytes from relay server\r\n", bytes_received);
        
        // 받은 Telnet 데이터를 그대로 처리 (표준 Telnet 프로토콜)
        tcp_relay_process_data(client, buffer, bytes_received);
    }
    
    // 연결 정리
    if (client->socket >= 0) {
        closesocket(client->socket);
        client->socket = -1;
    }
    client->connected = false;
    
    aws_free(buffer);
    task_printf("TCP Relay: Connection handler terminated\r\n");
}

// 공통 Telnet 프로토콜 처리 함수
static void telnet_process_common_data(void* client_ptr, bool is_server_mode, const uint8_t* data, int len)
{
    telnet_client_t* server_client = is_server_mode ? (telnet_client_t*)client_ptr : NULL;
    tcp_relay_client_t* relay_client = is_server_mode ? NULL : (tcp_relay_client_t*)client_ptr;
    
    telnet_state_t* state = is_server_mode ? &server_client->state : &relay_client->state;
    char* line_buffer = is_server_mode ? server_client->line_buffer : relay_client->line_buffer;
    int* line_pos = is_server_mode ? &server_client->line_pos : &relay_client->line_pos;
    size_t line_len;
    for (int i = 0; i < len; i++) {
        uint8_t ch = data[i];
        
        switch (*state) {
            case TELNET_STATE_NORMAL:
                if (ch == TELNET_IAC) {
                    *state = TELNET_STATE_IAC;
                } else if (ch == '\r') {
                    // CR 무시 (CR+LF에서 LF만 처리)
                    continue;
                } else if (ch == '\n') {
                    // 라인 완료 - 명령 처리
                    line_len = *line_pos;
                     line_buffer[line_len++] = '\n';
                    line_buffer[line_len] = 0;

                    if (*line_pos >= 0) {
                        task_printf("Telnet: Command received: %s\r\n", line_buffer);
                        
                        // 서버 모드인 경우 현재 클라이언트 설정
                        if (is_server_mode) {
                            g_current_telnet_client = server_client;
                        }
                        
                        // 터미널 브리지로 명령 전송
                        terminal_bridge_send_command(line_buffer, line_len);

                        // 서버 모드인 경우만 엔터와 프롬프트 처리
                        if (is_server_mode) {
                            telnet_send_response(server_client, "\r\n");
                        }
                    } else if (is_server_mode) {
                        // 빈 라인의 경우 프롬프트만 표시 (서버 모드만)
                        telnet_send_response(server_client, "\r\n");
                        telnet_send_prompt(server_client);
                    }
                    
                    *line_pos = 0;
                } else if (ch == '\b' || ch == 127) {
                    // Backspace 처리
                    if (*line_pos > 0) {
                        (*line_pos)--;
                        // 서버 모드인 경우만 에코 처리
                        if (is_server_mode && server_client->echo_enabled) {
                            telnet_send_response(server_client, "\b \b");
                        }
                    }
                } else if (ch >= 32 && ch < 127) {
                    // 일반 문자 처리
                    if (*line_pos < TELNET_LINE_BUFFER_SIZE - 1) {
                        line_buffer[(*line_pos)++] = ch;
                        
                        // 서버 모드인 경우만 에코 처리
                        if (is_server_mode && server_client->echo_enabled) {
                            char echo_ch = ch;
                            send(server_client->socket, &echo_ch, 1, 0);
                        }
                    }
                } else if (ch == 3) {
                    // Ctrl+C 처리
                    if (is_server_mode) {
                        telnet_send_response(server_client, "^C\r\n");
                        telnet_send_prompt(server_client);
                    }
                    *line_pos = 0;
                } else if (ch == 4) {
                    // Ctrl+D 처리 (EOF)
                    if (is_server_mode) {
                        telnet_send_response(server_client, "\r\nGoodbye!\r\n");
                        server_client->connected = false;
                    } else {
                        relay_client->connected = false;
                    }
                    return;
                }
                break;
                
            case TELNET_STATE_IAC:
                switch (ch) {
                    case TELNET_WILL:
                        *state = TELNET_STATE_WILL;
                        break;
                    case TELNET_WONT:
                        *state = TELNET_STATE_WONT;
                        break;
                    case TELNET_DO:
                        *state = TELNET_STATE_DO;
                        break;
                    case TELNET_DONT:
                        *state = TELNET_STATE_DONT;
                        break;
                    case TELNET_SB:
                        *state = TELNET_STATE_SB;
                        break;
                    case TELNET_IAC:
                        // IAC IAC = literal IAC
                        if (*line_pos < TELNET_LINE_BUFFER_SIZE - 1) {
                            line_buffer[(*line_pos)++] = TELNET_IAC;
                        }
                        *state = TELNET_STATE_NORMAL;
                        break;
                    default:
                        *state = TELNET_STATE_NORMAL;
                        break;
                }
                break;
                
            case TELNET_STATE_WILL:
                task_printf("Telnet: Client WILL %d\r\n", ch);
                if (is_server_mode) {
                    if (ch == TELNET_OPT_SGA) {
                        server_client->sga_enabled = true;
                    } else if (ch == TELNET_OPT_NAWS) {
                        telnet_send_option(server_client->socket, TELNET_DO, TELNET_OPT_NAWS);
                    }
                }
                *state = TELNET_STATE_NORMAL;
                break;
                
            case TELNET_STATE_WONT:
                task_printf("Telnet: Client WONT %d\r\n", ch);
                if (is_server_mode && ch == TELNET_OPT_ECHO) {
                    server_client->echo_enabled = false;
                }
                *state = TELNET_STATE_NORMAL;
                break;
                
            case TELNET_STATE_DO:
                task_printf("Telnet: Client DO %d\r\n", ch);
                if (is_server_mode) {
                    if (ch == TELNET_OPT_ECHO) {
                        telnet_send_option(server_client->socket, TELNET_WILL, TELNET_OPT_ECHO);
                        server_client->echo_enabled = true;
                    } else if (ch == TELNET_OPT_SGA) {
                        telnet_send_option(server_client->socket, TELNET_WILL, TELNET_OPT_SGA);
                        server_client->sga_enabled = true;
                    }
                }
                *state = TELNET_STATE_NORMAL;
                break;
                
            case TELNET_STATE_DONT:
                task_printf("Telnet: Client DONT %d\r\n", ch);
                *state = TELNET_STATE_NORMAL;
                break;
                
            case TELNET_STATE_SB:
                if (ch == TELNET_OPT_NAWS) {
                    *state = TELNET_STATE_SB_DATA;
                } else {
                    *state = TELNET_STATE_NORMAL;
                }
                break;
                
            case TELNET_STATE_SB_DATA:
                if (ch == TELNET_SE) {
                    *state = TELNET_STATE_NORMAL;
                }
                break;
        }
    }
}

// 기존 함수들을 공통 함수 호출로 변경
static void tcp_relay_process_data(tcp_relay_client_t* client, const uint8_t* data, int len)
{
    telnet_process_common_data(client, false, data, len);
}

// 터미널 브리지에서 호출되는 출력 콜백 (클라이언트 모드용)
static void tcp_relay_output_callback(const char* data, size_t len)
{
    if (g_tcp_relay_client.connected && data && len > 0) {
        // 표준 Telnet 데이터를 그대로 중계 서버로 전송
        send(g_tcp_relay_client.socket, data, len, 0);
        task_printf("TCP Relay: Sent %zu bytes to relay server\r\n", len);
    }
}

// 설정 함수들
void telnet_set_relay_server(const char* ip, uint16_t port)
{
    if (ip && strlen(ip) < sizeof(g_tcp_relay_client.relay_server_ip)) {
        strcpy(g_tcp_relay_client.relay_server_ip, ip);
        g_tcp_relay_client.relay_server_port = port;
        g_tcp_relay_enabled = true;
        task_printf("TCP Relay: Server set to %s:%d\r\n", ip, port);
    }
}

void telnet_server_task_init(void)
{
    g_telnetServerTaskId = osThreadNew(telnet_server_task, NULL, &telnet_server_task_attributes);
    if (g_telnetServerTaskId == NULL) {
        task_printf("Telnet Server: Failed to create task\r\n");

    } else {
        task_printf("Telnet Server: Started successfully\r\n");

    }
}