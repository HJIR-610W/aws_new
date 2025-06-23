#include "task_http_server.h"
#include "http_api.h"
#include "websocket.h"
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
#include "app_file.h"

#define SERVER_RETRY_INTERVAL_MS 5000
#define CLIENT_CONNECT_TIMEOUT_MS 10000

static osThreadId_t g_httpServerTaskId = NULL;
static bool g_httpServerRunning = false;

const osThreadAttr_t http_server_task_attributes = {
    .name = "http_server",
    .stack_size = 4096,
    .priority = (osPriority_t)osPriorityRealtime2,
};

static void http_server_task(void *argument);
static void handle_client_request(int client_socket);

void noti_httpServerTask(uint32_t flag)
{
  if(g_httpServerTaskId != NULL) // NULL üũ �߰�
  {
    osThreadFlagsSet(g_httpServerTaskId,  flag);
  }
}


static int set_recv_timeout(int sockfd, uint32_t timeout_ms)
{
  struct timeval timeout;
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
  {
    task_printf("HTTP Server: SO_RCVTIMEO setting failed, error: %d\r\n", errno);
    return -1;
  }
  return 0;
}

void http_send_response(int client_socket, int status_code, const char* content_type, const char* body)
{
    char *response = aws_malloc(HTTP_BUFFER_SIZE);
    if (response == NULL) {
        task_printf("HTTP Server: Failed to allocate response buffer\r\n");
        return;
    }
    
    const char* status_text = (status_code == 200) ? "OK" :
                             (status_code == 404) ? "Not Found" : 
                             (status_code == 400) ? "Bad Request" : "Internal Server Error";

    int content_length = body ? strlen(body) : 0;
    int header_length = snprintf(response, HTTP_BUFFER_SIZE,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_text, content_type, content_length);

    send(client_socket, response, header_length, 0);

    if (content_length > 0 && body) {
        const char* data_ptr = body;
        int remaining = content_length;
        
        while (remaining > 0) {
            int chunk_size = (remaining > 1024) ? 1024 : remaining;
            int sent = send(client_socket, data_ptr, chunk_size, 0);
            
            if (sent <= 0) {
                task_printf("HTTP Server: Failed to send body data\r\n");
                break;
            }
            
            data_ptr += sent;
            remaining -= sent;
        }
    }

    task_printf("HTTP Server: Sent response %d %s (%d bytes)\r\n", status_code, status_text, content_length);
    aws_free(response);
}

static int is_websocket_request(const char* buffer)
{
    // WebSocket 요청인지 확인하기 위한 조건들
    int has_upgrade = (strstr(buffer, "Upgrade: websocket") != NULL || 
                      strstr(buffer, "upgrade: websocket") != NULL);
    int has_connection = (strstr(buffer, "Connection: Upgrade") != NULL || 
                         strstr(buffer, "connection: upgrade") != NULL ||
                         strstr(buffer, "Connection: upgrade") != NULL);
    int has_ws_key = (strstr(buffer, "Sec-WebSocket-Key:") != NULL ||
                     strstr(buffer, "sec-websocket-key:") != NULL);
    
    task_printf("WebSocket check - Upgrade: %d, Connection: %d, Key: %d\r\n", 
               has_upgrade, has_connection, has_ws_key);
    
    return has_upgrade && has_connection && has_ws_key;
}

static char* extract_websocket_key(const char* buffer)
{
    char* key_line = strstr(buffer, "Sec-WebSocket-Key:");
    if (!key_line) {
        key_line = strstr(buffer, "sec-websocket-key:");
    }
    
    if (!key_line) {
        task_printf("WebSocket: Key header not found\r\n");
        return NULL;
    }
    
    char* key_start = strchr(key_line, ':');
    if (!key_start) {
        task_printf("WebSocket: Colon not found in key line\r\n");
        return NULL;
    }
    
    key_start++;
    while (*key_start == ' ' || *key_start == '\t') {
        key_start++;
    }
    
    char* key_end = strstr(key_start, "\r\n");
    if (!key_end) {
        // 줄 끝을 찾을 수 없는 경우, 줄바꿈 문자로 다시 시도
        key_end = strchr(key_start, '\n');
        if (!key_end) {
            key_end = strchr(key_start, '\r');
        }
        if (!key_end) {
            task_printf("WebSocket: End of key line not found\r\n");
            return NULL;
        }
    }
    
    size_t key_len = key_end - key_start;
    if (key_len == 0 || key_len > 64) {
        task_printf("WebSocket: Invalid key length: %zu\r\n", key_len);
        return NULL;
    }
    
    char* key = aws_malloc(key_len + 1);
    if (key) {
        strncpy(key, key_start, key_len);
        key[key_len] = '\0';
        
        // 끝의 공백 제거
        while (key_len > 0 && (key[key_len-1] == ' ' || key[key_len-1] == '\t')) {
            key[key_len-1] = '\0';
            key_len--;
        }
        
        task_printf("WebSocket: Extracted key: '%s' (length: %zu)\r\n", key, key_len);
    } else {
        task_printf("WebSocket: Failed to allocate memory for key\r\n");
    }
    
    return key;
}

int http_parse_request(const char* buffer, size_t length, http_request_t* request)
{
    if (!buffer || !request || length == 0) {
        return -1;
    }

    memset(request, 0, sizeof(http_request_t));
    
    char method[16];
    if (sscanf(buffer, "%15s %255s", method, request->path) != 2) {
        return -1;
    }

    if (strcmp(method, "GET") == 0) {
        request->method = HTTP_METHOD_GET;
    } else if (strcmp(method, "POST") == 0) {
        request->method = HTTP_METHOD_POST;
    } else if (strcmp(method, "OPTIONS") == 0) {
        request->method = HTTP_METHOD_OPTIONS;
    } else {
        request->method = HTTP_METHOD_UNKNOWN;
    }

    char* query_start = strchr(request->path, '?');
    if (query_start) {
        *query_start = '\0';
        strncpy(request->query, query_start + 1, sizeof(request->query) - 1);
    }

    char* body_start = strstr(buffer, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        request->body = (char*)body_start;
        request->body_length = length - (body_start - buffer);
    }

    return 0;
}


const char* fallback_html_content = 
    "<!DOCTYPE html>"
    "<html><head><title>AWS Weather Station</title></head>"
    "<body>"
    "<h1>AWS Weather Station HTTP Server</h1>"
    "<h2>Sensor Data API</h2>"
    "<ul>"
    "<li>GET /api/sensor - Get current sensor data</li>"
    "<li>GET /api/device - Get device information</li>"
    "<li>POST /api/device - Set device configuration</li>"
    "<li>GET /api/sensor/config/{type} - Get sensor configuration</li>"
    "<li>POST /api/sensor/config/{type} - Set sensor configuration</li>"
    "</ul>"
    "</body></html>";

void http_handle_index_page(int client_socket)
{
    const char* file_path = "0:/index.html";
    FSIZE_t file_size = 0;
    FRESULT res;
    uint8_t* file_content = NULL;
    
    task_printf("HTTP Server: Reading index.html from SD card\r\n");
    
    res = get_file_size(file_path, &file_size);
    if (res != FR_OK || file_size == 0) {
        task_printf("HTTP Server: Failed to get file size or file empty (error: %d, size: %lu)\r\n", res, (unsigned long)file_size);
        http_send_response(client_socket, 200, "text/html; charset=utf-8", fallback_html_content);
        return;
    }
    
    if (file_size > HTTP_MAX_FILE_SIZE) {
        task_printf("HTTP Server: File too large (%lu bytes), using fallback\r\n", (unsigned long)file_size);
        http_send_response(client_socket, 200, "text/html; charset=utf-8", fallback_html_content);
        return;
    }
    
    file_content = (uint8_t*)aws_malloc(file_size + 1);
    if (file_content == NULL) {
        task_printf("HTTP Server: Failed to allocate memory for file content\r\n");
        http_send_response(client_socket, 200, "text/html; charset=utf-8", fallback_html_content);
        return;
    }
    
    res = read_file((char*)file_path, file_content, (uint32_t)file_size, 0);
    if (res != FR_OK) {
        task_printf("HTTP Server: Failed to read file from SD card (error: %d)\r\n", res);
        aws_free(file_content);
        http_send_response(client_socket, 200, "text/html; charset=utf-8", fallback_html_content);
        return;
    }
    
    file_content[file_size] = '\0';
    
    task_printf("HTTP Server: Successfully read %lu bytes from index.html\r\n", (unsigned long)file_size);
    http_send_response(client_socket, 200, "text/html; charset=utf-8", (char*)file_content);
    
    aws_free(file_content);
}

void http_handle_static_file(int client_socket, const char* file_path, const char* content_type)
{
    char full_path[64];
    FSIZE_t file_size = 0;
    FRESULT res;
    uint8_t* file_content = NULL;
    
    snprintf(full_path, sizeof(full_path), "0:/%s", file_path);
    task_printf("HTTP Server: Reading static file %s from SD card\r\n", full_path);
    
    res = get_file_size(full_path, &file_size);
    if (res != FR_OK || file_size == 0) {
        task_printf("HTTP Server: Static file not found or empty (error: %d)\r\n", res);
        http_send_response(client_socket, 404, "text/plain", "File not found");
        return;
    }
    
    if (file_size > HTTP_MAX_FILE_SIZE) {
        task_printf("HTTP Server: Static file too large (%lu bytes)\r\n", (unsigned long)file_size);
        http_send_response(client_socket, 413, "text/plain", "File too large");
        return;
    }
    
    file_content = (uint8_t*)aws_malloc(file_size);
    if (file_content == NULL) {
        task_printf("HTTP Server: Failed to allocate memory for static file\r\n");
        http_send_response(client_socket, 500, "text/plain", "Internal server error");
        return;
    }
    
    res = read_file(full_path, file_content, (uint32_t)file_size, 0);
    if (res != FR_OK) {
        task_printf("HTTP Server: Failed to read static file (error: %d)\r\n", res);
        aws_free(file_content);
        http_send_response(client_socket, 500, "text/plain", "Internal server error");
        return;
    }
    
    task_printf("HTTP Server: Successfully read %lu bytes from %s\r\n", (unsigned long)file_size, full_path);
    
    char *response = aws_malloc(HTTP_BUFFER_SIZE);
    if (response == NULL) {
        task_printf("HTTP Server: Failed to allocate response buffer for static file\r\n");
        aws_free(file_content);
        http_send_response(client_socket, 500, "text/plain", "Internal server error");
        return;
    }
    
    int header_length = snprintf(response, HTTP_BUFFER_SIZE,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n"
        "Cache-Control: public, max-age=3600\r\n"
        "\r\n",
        content_type, (unsigned long)file_size);

    send(client_socket, response, header_length, 0);
    send(client_socket, file_content, file_size, 0);
    
    aws_free(response);
    aws_free(file_content);
}

static void handle_client_request(int client_socket)
{
    char *buffer = aws_malloc(HTTP_BUFFER_SIZE);
    if (buffer == NULL) {
        task_printf("HTTP Server: Failed to allocate buffer memory\r\n");
        http_send_response(client_socket, 500, "text/plain", "Internal Server Error");
        return;
    }
    
    memset(buffer, 0, HTTP_BUFFER_SIZE);

    if (set_recv_timeout(client_socket, HTTP_RECV_TIMEOUT_MS) < 0) {
        task_printf("HTTP Server: Failed to set recv timeout\r\n");
        aws_free(buffer);
        return;
    }

    int bytes_received = recv(client_socket, buffer, HTTP_BUFFER_SIZE - 1, 0);
    
    if (bytes_received <= 0) {
        task_printf("HTTP Server: No data received from client\r\n");
        aws_free(buffer);
        return;
    }

    buffer[bytes_received] = '\0';
    task_printf("HTTP Server: Received %d bytes\r\n", bytes_received);

    if (is_websocket_request(buffer)) {
        task_printf("HTTP Server: WebSocket upgrade request detected\r\n");
        
        char* ws_key = extract_websocket_key(buffer);
        if (ws_key) {
            if (websocket_handshake(client_socket, ws_key) == 0) {
                task_printf("HTTP Server: WebSocket handshake successful\r\n");
                websocket_handle_connection(client_socket);
            } else {
                task_printf("HTTP Server: WebSocket handshake failed\r\n");
            }
            aws_free(ws_key);
        } else {
            task_printf("HTTP Server: WebSocket key not found\r\n");
            http_send_response(client_socket, 400, "text/plain", "Bad WebSocket Request");
        }
        
        aws_free(buffer);
        return;
    }

    http_request_t request;
    if (http_parse_request(buffer, bytes_received, &request) < 0) {
        http_send_response(client_socket, 400, "text/plain", "Bad Request");
        aws_free(buffer);
        return;
    }

    task_printf("HTTP Server: %s %s\r\n", 
                request.method == HTTP_METHOD_GET ? "GET" : 
                request.method == HTTP_METHOD_POST ? "POST" : "OTHER",
                request.path);

    if (request.method == HTTP_METHOD_OPTIONS) {
        http_send_response(client_socket, 200, "text/plain", "");
    }
    else if (strncmp(request.path, "/api/sensor/config/", 19) == 0) {
        char sensor_type[16];
        strncpy(sensor_type, request.path + 19, sizeof(sensor_type) - 1);
        sensor_type[sizeof(sensor_type) - 1] = '\0';
        
        if (request.method == HTTP_METHOD_GET) {
            http_api_handle_sensor_config_get(client_socket, sensor_type);
        } else if (request.method == HTTP_METHOD_POST) {
            http_api_handle_sensor_config_post(client_socket, request.body, sensor_type);
        }
    }
    else if (strcmp(request.path, "/api/sensor") == 0 && request.method == HTTP_METHOD_GET) {
        http_api_handle_sensor_get(client_socket);
    }
    else if (strcmp(request.path, "/api/device") == 0) {
        if (request.method == HTTP_METHOD_GET) {
            http_api_handle_device_get(client_socket);
        } else if (request.method == HTTP_METHOD_POST) {
            http_api_handle_device_post(client_socket, request.body);
        }
    }
    else if (strcmp(request.path, "/favicon.ico") == 0) {
        http_handle_static_file(client_socket, "favicon.ico", "image/x-icon");
    }
    else if (strcmp(request.path, "/") == 0 || strcmp(request.path, "/index.html") == 0) {
        http_handle_index_page(client_socket);
    }
    else if (strncmp(request.path, "/", 1) == 0 && strlen(request.path) > 1) {
        const char* file_path = request.path + 1;
        const char* content_type = "text/plain";
        
        if (strstr(file_path, ".html") || strstr(file_path, ".htm")) {
            content_type = "text/html; charset=utf-8";
        } else if (strstr(file_path, ".css")) {
            content_type = "text/css";
        } else if (strstr(file_path, ".js")) {
            content_type = "application/javascript";
        } else if (strstr(file_path, ".png")) {
            content_type = "image/png";
        } else if (strstr(file_path, ".jpg") || strstr(file_path, ".jpeg")) {
            content_type = "image/jpeg";
        } else if (strstr(file_path, ".gif")) {
            content_type = "image/gif";
        } else if (strstr(file_path, ".ico")) {
            content_type = "image/x-icon";
        }
        
        http_handle_static_file(client_socket, file_path, content_type);
    }
    else {
        http_send_response(client_socket, 404, "text/plain", "Not Found");
    }
    
    aws_free(buffer);
}

static void http_server_task(void *argument)
{
    (void)argument;
    
    int server_socket = -1;
    int client_socket = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int opt = 1;

    task_printf("HTTP Server: Starting on port %d\r\n", HTTP_SERVER_PORT);

    osThreadFlagsWait(0x00000001, osFlagsWaitAny, osWaitForever);
       
       
    while (1) {
        if (server_socket < 0) {
            server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (server_socket < 0) {
                task_printf("HTTP Server: Socket creation failed, error: %d\r\n", errno);
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                task_printf("HTTP Server: SO_REUSEADDR failed, error: %d\r\n", errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(HTTP_SERVER_PORT);
            server_addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                task_printf("HTTP Server: Bind failed on port %d, error: %d\r\n", HTTP_SERVER_PORT, errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            if (listen(server_socket, HTTP_MAX_CLIENTS) < 0) {
                task_printf("HTTP Server: Listen failed, error: %d\r\n", errno);
                closesocket(server_socket);
                server_socket = -1;
                osDelay(SERVER_RETRY_INTERVAL_MS);
                continue;
            }

            task_printf("HTTP Server: Listening on port %d (socket: %d)\r\n", HTTP_SERVER_PORT, server_socket);
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
        task_printf("HTTP Server: New client connected %s:%u (socket: %d)\r\n", 
                   client_ip_str, ntohs(client_addr.sin_port), client_socket);

        handle_client_request(client_socket);

        closesocket(client_socket);
        task_printf("HTTP Server: Client disconnected\r\n");
    }

    if (server_socket >= 0) {
        closesocket(server_socket);
    }

    task_printf("HTTP Server: Task terminated\r\n");
}




void http_server_task_init(void)
{
    g_httpServerTaskId = osThreadNew(http_server_task, NULL, &http_server_task_attributes);
    if (g_httpServerTaskId == NULL) {
        task_printf("HTTP Server: Failed to create task\r\n");
        g_httpServerRunning = false;
    } else {
        task_printf("HTTP Server: Started successfully\r\n");
    }
}