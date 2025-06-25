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
#include "config_app.h"

#define SERVER_RETRY_INTERVAL_MS 5000
#define TELNET_WELCOME_MSG "\r\n=== AWS Telnet Console ===\r\n\r\n"

eTELNET_MODE_t g_telnet_server_mode_use = eTELNET_SERVER;

static osThreadId_t g_telnetServerTaskId = NULL;

static telnet_client_t g_telnet_clients[TELNET_MAX_CLIENTS];
static tcp_relay_client_t g_tcp_relay_client = {0};


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
static bool telnet_is_socket_valid(int socket);
static int telnet_safe_send(int socket, const void* data, size_t len);

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
    if (client->socket >= 0) {
        task_printf("Telnet: Closing client socket (socket: %d, connected: %s)\r\n", 
                   client->socket, client->connected ? "true" : "false");
        closesocket(client->socket);
    }
    
    client->connected = false;
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
    if (!telnet_is_socket_valid(socket)) {
        task_printf("Telnet: Cannot send option - invalid socket\r\n");
        return;
    }
    
    uint8_t buf[3] = {TELNET_IAC, cmd, option};
    if (telnet_safe_send(socket, buf, 3) >= 0) {
        task_printf("Telnet: Sent option - IAC %d %d\r\n", cmd, option);
    }
}



static bool telnet_is_socket_valid(int socket)
{
    if (socket < 0) {
        return false;
    }
    
    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        return false;
    }
    
    return (error == 0);
}

static int telnet_safe_send(int socket, const void* data, size_t len)
{
    if (!telnet_is_socket_valid(socket) || !data || len == 0) {
        return -1;
    }
    
    int result = send(socket, data, len, 0);
    if (result < 0) {
        if (errno == EPIPE || errno == ECONNRESET || errno == ECONNABORTED || errno == ENOTCONN) {
            task_printf("Telnet: Send failed - connection closed (socket: %d, error: %d)\r\n", socket, errno);
        } else if ( errno == EAGAIN) {
            task_printf("Telnet: Send would block (socket: %d)\r\n", socket);
        } else {
            task_printf("Telnet: Send error (socket: %d, error: %d)\r\n", socket, errno);
        }
    }
    
    return result;
}

static void telnet_send_response(telnet_client_t* client, const char* response)
{
    if (!client->connected || !response || !telnet_is_socket_valid(client->socket)) {
        return;
    }
    
    if (telnet_safe_send(client->socket, response, strlen(response)) < 0) {
        client->connected = false;
    }
}

static void telnet_send_prompt(telnet_client_t* client)
{
    if (!client->connected) {
        return;
    }
    
    const char* prompt = "AWS>>";
    telnet_send_response(client, prompt);
}


static telnet_client_t* g_current_telnet_client = NULL;

static void telnet_output_callback(const char* data, size_t len)
{
    if (g_current_telnet_client && g_current_telnet_client->connected && 
        telnet_is_socket_valid(g_current_telnet_client->socket) && data && len > 0) {
        if (telnet_safe_send(g_current_telnet_client->socket, data, len) < 0) {
            g_current_telnet_client->connected = false;
        }
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
    
    if (set_recv_timeout(client->socket, TELNET_RECV_TIMEOUT_MS) < 0) {
        task_printf("Telnet: Failed to set recv timeout for client\r\n");
        aws_free(buffer);
        return;
    }
    
    uint32_t last_activity = HAL_GetTick();
    const uint32_t ACTIVITY_TIMEOUT_MS = 300000;
    
    telnet_send_response(client, TELNET_WELCOME_MSG);
    telnet_send_prompt(client);
    
    terminal_bridge_init();
    terminal_bridge_set_output_callback(telnet_output_callback);
    
    task_printf("Telnet: Client handler started (socket: %d)\r\n", client->socket);
    
    while (client->connected)
    {
        uint32_t current_time = HAL_GetTick();
        if (current_time - last_activity > ACTIVITY_TIMEOUT_MS) {
            task_printf("Telnet: Client inactive timeout (socket: %d)\r\n", client->socket);
            client->connected = false;
            break;
        }
        
        int bytes_received = recv(client->socket, buffer, TELNET_BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                task_printf("Telnet: Client disconnected normally (socket: %d)\r\n", client->socket);
                client->connected = false;
            } else if (errno == EAGAIN ) {
                continue;
            } else if (errno == ECONNRESET || errno == ECONNABORTED || errno == ENOTCONN) {
                task_printf("Telnet: Client connection reset/aborted (socket: %d, error: %d)\r\n", client->socket, errno);
                client->connected = false;
            } else if (errno == EBADF || errno == EINVAL) {
                task_printf("Telnet: Invalid socket descriptor (socket: %d, error: %d)\r\n", client->socket, errno);
                client->connected = false;
            } else {
                task_printf("Telnet: Client recv error (socket: %d, error: %d)\r\n", client->socket, errno);
                client->connected = false;
            }
            break;
        }
        
        last_activity = current_time;
        task_printf("Telnet: Received %d bytes from client (socket: %d)\r\n", bytes_received, client->socket);
        telnet_process_data(client, buffer, bytes_received);
    }
    
    g_current_telnet_client = NULL;
    terminal_bridge_cleanup();
    aws_free(buffer);
    task_printf("Telnet: Client handler terminated (socket: %d)\r\n", client->socket);
}



static void telnet_server_mode_task(void)
{
    int server_socket = -1;
    int client_socket = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int opt = 1;
    uint16_t telnet_server_port = get_config_app()->dev_telnet_port;

    for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
        memset(&g_telnet_clients[i], 0, sizeof(telnet_client_t));
        g_telnet_clients[i].socket = -1;
        g_telnet_clients[i].connected = false;
    }

    task_printf("Telnet Server: Starting on port %d\r\n", telnet_server_port);

    while (1) {
        if (server_socket < 0)
        {
            server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (server_socket < 0)
            {
                task_printf("Telnet Server: Socket creation failed, error: %d\r\n", errno);
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            {
                task_printf("Telnet Server: SO_REUSEADDR failed, error: %d\r\n", errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }
            
#ifdef SO_REUSEPORT
            if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
            {
                task_printf("Telnet Server: SO_REUSEPORT failed, error: %d (continuing)\r\n", errno);
            }
#endif
            
            struct linger linger_opt = {0, 0};
            if (setsockopt(server_socket, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt)) < 0)
            {
                task_printf("Telnet Server: SO_LINGER failed, error: %d (continuing)\r\n", errno);
            }

            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(telnet_server_port);
            server_addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            {
                task_printf("Telnet Server: Bind failed on port %d, error: %d\r\n", telnet_server_port, errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            if (listen(server_socket, TELNET_MAX_CLIENTS) < 0)
            {
                task_printf("Telnet Server: Listen failed, error: %d\r\n", errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            task_printf("Telnet Server: Listening on port %d (socket: %d)\r\n", telnet_server_port,
                        server_socket);
        }

        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            if (errno == ECONNABORTED) {
                task_printf("Telnet Server: Connection aborted during accept (errno: %d)\r\n", errno);
                osDelay(50);
                continue;
            } else if (errno == EINVAL || errno == EBADF) {
                task_printf("Telnet Server: Invalid server socket, recreating (error: %d)\r\n", errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            } else if (errno == EAGAIN ) {
                osDelay(50);
                continue;
            } else if (errno == EMFILE || errno == ENFILE) {
                task_printf("Telnet Server: Too many open files (error: %d)\r\n", errno);
                osDelay(1000);
                continue;
            } else if (errno == ENOBUFS || errno == ENOMEM) {
                task_printf("Telnet Server: No buffer space available (error: %d)\r\n", errno);
                osDelay(1000);
                continue;
            } else {
                task_printf("Telnet Server: Accept failed (error: %d)\r\n", errno);
                osDelay(200);
                continue;
            }
        }

        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, sizeof(client_ip_str));
        task_printf("Telnet Server: New client connected %s:%u (socket: %d)\r\n", 
                   client_ip_str, ntohs(client_addr.sin_port), client_socket);

        telnet_client_t* client = telnet_find_free_client();
        if (client == NULL)
        {
            task_printf("Telnet Server: No free client slots, rejecting connection\r\n");
            const char* reject_msg = "Server full. Please try again later.\r\n";
            telnet_safe_send(client_socket, reject_msg, strlen(reject_msg));
            closesocket(client_socket);
            continue;
        }


        struct linger client_linger = {1, 2};
        setsockopt(client_socket, SOL_SOCKET, SO_LINGER, &client_linger, sizeof(client_linger));
        
        int keepalive = 1;
        setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
        
        int keepidle = 60;   // 60sec idle before keepalive starts
        setsockopt(client_socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
        
        int keepintvl = 60;  // 10sec interval between probes
        setsockopt(client_socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
        
        int keepcnt = 3;     // 3 failed probes = connection closed
        setsockopt(client_socket, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
        
        struct timeval send_timeout = {30, 0};
        setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout));
        
        struct timeval recv_timeout = {60, 0};
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));
        
        int nodelay = 1;
        setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
        
        task_printf("Telnet Server: Client socket options configured (socket: %d)\r\n", client_socket);
        
        telnet_init_client(client, client_socket);
        telnet_handle_client(client);
        telnet_cleanup_client(client);
        
        task_printf("Telnet Server: Client session ended, ready for next connection\r\n");
    }




}

// =================================================================
// Client mode functions (TCP relay functionality)
// =================================================================

static void telnet_client_mode_task(void)
{
  uint8_t* ip = get_config_app()->dev_telnet_ip;
  uint16_t port = get_config_app()->dev_telnet_port;


    snprintf(g_tcp_relay_client.relay_server_ip, sizeof(g_tcp_relay_client.relay_server_ip),
             "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    g_tcp_relay_client.relay_server_port = port;

    
    // Initialize terminal bridge
    terminal_bridge_init();
    terminal_bridge_set_output_callback(tcp_relay_output_callback);
    
    
    while (1)
    {
        if (!g_tcp_relay_client.connected)
        {
            tcp_relay_connect(&g_tcp_relay_client);
        }
        
        if (g_tcp_relay_client.connected)
        {
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
    if (client->socket < 0)
    {
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
    if (buffer == NULL)
    {
        task_printf("TCP Relay: Failed to allocate buffer\r\n");
        return;
    }
    
    if (set_recv_timeout(client->socket, 5000) < 0)
    {
        task_printf("TCP Relay: Failed to set recv timeout\r\n");
        aws_free(buffer);
        return;
    }
    
    
    while (client->connected)
    {
        int bytes_received = recv(client->socket, buffer, TELNET_BUFFER_SIZE, 0);
        
        if (bytes_received <= 0)
        {
            if (bytes_received == 0)
            {
                task_printf("TCP Relay: Server closed connection\r\n");
                break;
            }
            else if (errno == EAGAIN)
            {
                continue; // Timeout, continue trying
            }
            else
            {
                task_printf("TCP Relay: Recv error: %d\r\n", errno);
            }
            break;
        }
        tcp_relay_process_data(client, buffer, bytes_received);
    }
    
    // Connection cleanup
    if (client->socket >= 0) {
        closesocket(client->socket);
        client->socket = -1;
    }
    client->connected = false;
    
    aws_free(buffer);

}
//ctrl+c ff f8
//ctrl+break ff f3

// Common Telnet protocol processing functions
// Common structure for both client types
typedef struct {
    telnet_state_t* state;
    char* line_buffer;
    int* line_pos;
    int socket;
    bool* connected;
    bool echo_enabled;
} telnet_common_client_t;

// Helper function to create common client structure
static telnet_common_client_t telnet_get_common_client(void* client_ptr, bool is_server_mode)
{
    telnet_common_client_t common = {0};
    
    if (is_server_mode) {
        telnet_client_t* server_client = (telnet_client_t*)client_ptr;
        common.state = &server_client->state;
        common.line_buffer = server_client->line_buffer;
        common.line_pos = &server_client->line_pos;
        common.socket = server_client->socket;
        common.connected = &server_client->connected;
        common.echo_enabled = server_client->echo_enabled;
    } else {
        tcp_relay_client_t* relay_client = (tcp_relay_client_t*)client_ptr;
        common.state = &relay_client->state;
        common.line_buffer = relay_client->line_buffer;
        common.line_pos = &relay_client->line_pos;
        common.socket = relay_client->socket;
        common.connected = &relay_client->connected;
        common.echo_enabled = false; // Relay clients don't echo
    }
    
    return common;
}

// Unified command processing function - mode independent
static void telnet_process_command(const char* command, size_t len)
{
    task_printf("Telnet: Processing command: %s\r\n", command);
    // Send command to terminal bridge - same for both modes
    terminal_bridge_send_command(command, len);
}

// Unified response function
static void telnet_send_unified_response(telnet_common_client_t* common, const char* response)
{
    if (common->socket > 0 && *common->connected && telnet_is_socket_valid(common->socket)) {
        if (telnet_safe_send(common->socket, response, strlen(response)) < 0) {
            *common->connected = false;
        }
    }
}

static void telnet_process_common_data(void* client_ptr, bool is_server_mode, const uint8_t* data, int len)
{
    telnet_common_client_t common = telnet_get_common_client(client_ptr, is_server_mode);
    size_t line_len;
    for (int i = 0; i < len; i++) {
        uint8_t ch = data[i];
        
        switch (*common.state) {
            case TELNET_STATE_NORMAL:
                if (ch == TELNET_IAC) {
                    *common.state = TELNET_STATE_IAC;
                } else if (ch == '\r') {
                    // Skip CR (handle LF in CR+LF sequence)
                    continue;
                } else if (ch == '\n' || ch==0x03||ch == 0x11) {
                    if(ch=='\n')
                    {
                    // Line complete - process command (mode independent)
                    line_len = *common.line_pos;
                    common.line_buffer[line_len++] = '\n';
                    common.line_buffer[line_len] = 0;
                    }
                    else
                    {
                      common.line_buffer[0] = ch;
                      *common.line_pos =0 ;
                      line_len =1;
                    }    
                    if (*common.line_pos >= 0) {
                        // Set global client for server mode only (connection management)
                        if (is_server_mode) {
                            g_current_telnet_client = (telnet_client_t*)client_ptr;
                        }
                        
                        // Process command - unified for both modes
                        telnet_process_command(common.line_buffer, line_len);

                        // Send response - mode independent
                        telnet_send_unified_response(&common, "\r\n");
                    }
                    
                    *common.line_pos = 0;
                } else if (ch == '\b' || ch == 127) {
                    // Backspace processing - mode independent
                    if (*common.line_pos > 0) {
                        (*common.line_pos)--;
                        // Echo only for server mode with echo enabled
                        if (is_server_mode && common.echo_enabled) {
                            telnet_send_unified_response(&common, "\b \b");
                        }
                    }
                } else if (ch >= 32 && ch < 127) {
                    // Regular character processing - mode independent
                    if (*common.line_pos < TELNET_LINE_BUFFER_SIZE - 1) {
                        common.line_buffer[(*common.line_pos)++] = ch;
                        
                        // Echo only for server mode with echo enabled
                        if (is_server_mode && common.echo_enabled && telnet_is_socket_valid(common.socket)) {
                            char echo_ch = ch;
                            if (telnet_safe_send(common.socket, &echo_ch, 1) < 0) {
                                *common.connected = false;
                                return;
                            }
                        }
                    }
                }
                else if (ch == 4) {
                    // Ctrl+D processing (EOF) - mode independent
                    if (is_server_mode) {
                        telnet_send_unified_response(&common, "\r\nGoodbye!\r\n");
                    }
                    *common.connected = false;
                    return;
                }
                break;
                
            case TELNET_STATE_IAC:
                switch (ch) {
                    case TELNET_WILL:
                        *common.state = TELNET_STATE_WILL;
                        break;
                    case TELNET_WONT:
                        *common.state = TELNET_STATE_WONT;
                        break;
                    case TELNET_DO:
                        *common.state = TELNET_STATE_DO;
                        break;
                    case TELNET_DONT:
                        *common.state = TELNET_STATE_DONT;
                        break;
                    case TELNET_SB:
                        *common.state = TELNET_STATE_SB;
                        break;
                    case TELNET_IAC:
                        // IAC IAC = literal IAC
                        if (*common.line_pos < TELNET_LINE_BUFFER_SIZE - 1) {
                            common.line_buffer[(*common.line_pos)++] = TELNET_IAC;
                        }
                        *common.state = TELNET_STATE_NORMAL;
                        break;
                    default:
                        *common.state = TELNET_STATE_NORMAL;
                        break;
                }
                break;
                
            case TELNET_STATE_WILL:
                task_printf("Telnet: Client WILL %d\r\n", ch);
                if (is_server_mode) {
                    telnet_client_t* server_client = (telnet_client_t*)client_ptr;
                    if (ch == TELNET_OPT_SGA) {
                        server_client->sga_enabled = true;
                    } else if (ch == TELNET_OPT_NAWS) {
                        telnet_send_option(server_client->socket, TELNET_DO, TELNET_OPT_NAWS);
                    }
                }
                *common.state = TELNET_STATE_NORMAL;
                break;
                
            case TELNET_STATE_WONT:
                task_printf("Telnet: Client WONT %d\r\n", ch);
                if (is_server_mode && ch == TELNET_OPT_ECHO) {
                    telnet_client_t* server_client = (telnet_client_t*)client_ptr;
                    server_client->echo_enabled = false;
                }
                *common.state = TELNET_STATE_NORMAL;
                break;
                
            case TELNET_STATE_DO:
                task_printf("Telnet: Client DO %d\r\n", ch);
                if (is_server_mode) {
                    telnet_client_t* server_client = (telnet_client_t*)client_ptr;
                    if (ch == TELNET_OPT_ECHO) {
                        telnet_send_option(server_client->socket, TELNET_WILL, TELNET_OPT_ECHO);
                        server_client->echo_enabled = true;
                    } else if (ch == TELNET_OPT_SGA) {
                        telnet_send_option(server_client->socket, TELNET_WILL, TELNET_OPT_SGA);
                        server_client->sga_enabled = true;
                    }
                }
                *common.state = TELNET_STATE_NORMAL;
                break;
                
            case TELNET_STATE_DONT:
                task_printf("Telnet: Client DONT %d\r\n", ch);
                *common.state = TELNET_STATE_NORMAL;
                break;
                
            case TELNET_STATE_SB:
                if (ch == TELNET_OPT_NAWS) {
                    *common.state = TELNET_STATE_SB_DATA;
                } else {
                    *common.state = TELNET_STATE_NORMAL;
                }
                break;
                
            case TELNET_STATE_SB_DATA:
                if (ch == TELNET_SE) {
                    *common.state = TELNET_STATE_NORMAL;
                }
                break;
        }
    }
}

// Wrapper function for calling common function from relay functions
static void tcp_relay_process_data(tcp_relay_client_t* client, const uint8_t* data, int len)
{
    telnet_process_common_data(client, false, data, len);
}

// Output callback called by terminal bridge (client mode)
static void tcp_relay_output_callback(const char* data, size_t len)
{
    if (g_tcp_relay_client.connected && telnet_is_socket_valid(g_tcp_relay_client.socket) && data && len > 0)
    {
        // Send standard Telnet data as-is to relay server
        if (telnet_safe_send(g_tcp_relay_client.socket, data, len) < 0) {
            g_tcp_relay_client.connected = false;
        }
    }
}

// Utility functions
void telnet_set_relay_server(const char* ip, uint16_t port)
{
    if (ip && strlen(ip) < sizeof(g_tcp_relay_client.relay_server_ip))
    {
        strcpy(g_tcp_relay_client.relay_server_ip, ip);
        g_tcp_relay_client.relay_server_port = port;

        task_printf("TCP Relay: Server set to %s:%d\r\n", ip, port);
    }
}

static void telnet_server_task(void* argument)
{
  (void)argument;

  osThreadFlagsWait(0x00000001, osFlagsWaitAny, osWaitForever);

  g_telnet_server_mode_use = get_config_app()->dev_telnet_mode;

  if (g_telnet_server_mode_use == eTELNET_CLIENT)
  {  
    telnet_client_mode_task();
  }
  else
  {
    telnet_server_mode_task();
  }
}


void telnet_server_task_init(void)
{

  g_telnetServerTaskId = osThreadNew(telnet_server_task, NULL, &telnet_server_task_attributes);

}