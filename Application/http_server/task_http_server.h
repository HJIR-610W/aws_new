

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "cmsis_os2.h"
#include "lwip/sockets.h"
#include "app_file.h"
#include <stdint.h>
#include <stdbool.h>

#define HTTP_SERVER_PORT 8080
#define HTTP_BUFFER_SIZE 4096
#define HTTP_MAX_CLIENTS 5
#define HTTP_RECV_TIMEOUT_MS 30000
#define HTTP_MAX_FILE_SIZE (64 * 1024)  // 64KB 파일 크기 제한

typedef enum {
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_UNKNOWN
} http_method_t;

typedef struct {
    char path[256];
    char query[256];
    http_method_t method;
    char *body;
    size_t body_length;
    char *headers;
    size_t headers_length;
} http_request_t;

typedef struct {
    int status_code;
    char content_type[64];
    char *body;
    size_t body_length;
} http_response_t;

void http_server_task_init(void);
void noti_httpServerTask(uint32_t flag);

void http_send_response(int client_socket, int status_code, const char* content_type, const char* body);
int http_parse_request(const char* buffer, size_t length, http_request_t* request);

void http_handle_index_page(int client_socket);
void http_handle_static_file(int client_socket, const char* file_path, const char* content_type);

#endif
