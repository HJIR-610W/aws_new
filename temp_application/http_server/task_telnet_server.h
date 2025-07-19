#ifndef TASK_TELNET_SERVER_H
#define TASK_TELNET_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

#include "config_app.h"
// Telnet / 

#define TELNET_MAX_CLIENTS          1
#define TELNET_BUFFER_SIZE          512
#define TELNET_RECV_TIMEOUT_MS      30000
#define TELNET_LINE_BUFFER_SIZE     256

//    (TCP )
#define TELNET_RECONNECT_INTERVAL   5000      //   (ms)

//  
extern eTELNET_MODE_t g_telnet_server_mode_use;  // 0:  , 1:  

// Telnet  
#define TELNET_IAC          255  // Interpret As Command (0xFF)
#define TELNET_WILL         251  // Will option (0xFB)
#define TELNET_WONT         252  // Won't option (0xFC)  
#define TELNET_DO           253  // Do option (0xFD)
#define TELNET_DONT         254  // Don't option (0xFE)
#define TELNET_SB           250  // Subnegotiation Begin (0xFA)
#define TELNET_SE           240  // Subnegotiation End (0xF0)

// Telnet control commands (triggered by key combinations)
#define TELNET_IP           244  // Interrupt Process (0xF4) - Ctrl+C (standard)
#define TELNET_AO           245  // Abort Output (0xF5) - Ctrl+O  
#define TELNET_AYT          246  // Are You There (0xF6) - Ctrl+T
#define TELNET_EC           247  // Erase Character (0xF7) - Backspace
#define TELNET_EL           248  // Erase Line (0xF8) - Ctrl+U or Ctrl+C in PuTTY
#define TELNET_GA           249  // Go Ahead (0xF9)
#define TELNET_BRK          243  // Break (0xF3) - Ctrl+Break
#define TELNET_DM           242  // Data Mark (0xF2)
#define TELNET_NOP          241  // No Operation (0xF1)

// Telnet 
#define TELNET_OPT_ECHO     1    // Echo
#define TELNET_OPT_SGA      3    // Suppress Go Ahead
#define TELNET_OPT_NAWS     31   // Negotiate About Window Size

// Telnet  
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

// Telnet   ( )
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

// TCP    ( )
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

// Telnet  
void telnet_server_task_init(void);
void noti_telnetServerTask(uint32_t flag);

//  Telnet   
static void telnet_process_common_data(void* client_ptr, bool is_server_mode, const uint8_t* data, int len);

// Telnet   
static void telnet_handle_client(telnet_client_t* client);
static void telnet_send_option(int socket, uint8_t cmd, uint8_t option);
static void telnet_process_data(telnet_client_t* client, const uint8_t* data, int len);
static void telnet_send_response(telnet_client_t* client, const char* response);
static void telnet_send_prompt(telnet_client_t* client);

// Telnet    (TCP )
void telnet_client_mode_task(void);
void tcp_relay_connect(tcp_relay_client_t* client);
void tcp_relay_handle_connection(tcp_relay_client_t* client);
static void tcp_relay_process_data(tcp_relay_client_t* client, const uint8_t* data, int len);

//  
void telnet_set_relay_server(const char* ip, uint16_t port);

#endif // TASK_TELNET_SERVER_H