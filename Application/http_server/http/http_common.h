

#ifndef HTTP_COMMOM_H
#define HTTP_COMMOM_H

#include <stdint.h>

#define MAX_PATHS 6
#define MAX_QUERIES 5
#define MAX_KEY_VALUE_LEN 20



typedef struct query_s
{
    char key[MAX_KEY_VALUE_LEN];
    char value[MAX_KEY_VALUE_LEN];
} query_t;

typedef struct http_request_s
{
  char path[MAX_PATHS][MAX_KEY_VALUE_LEN];
  query_t query[MAX_QUERIES];
} http_request_t;


long get_content_length(const char *request) ;
uint32_t make_http_ok_header(char *out,uint32_t outSize,char *body,const char *content_type);

int parse_get_para(char *input, char **key, char **value,uint32_t max_prams);
int parse_json_params(char* input, char** key, char** value, uint32_t max_params);

void send_404_response(int conn);


int32_t connect_http(uint8_t ip[4],uint32_t port);
void close_http(int conn);
int recv_httpBody(int conn, char *output, int outsize, int timeout_ms);
void http_parse(const char *http_request, http_request_t *req, uint16_t *pathCnt,
                uint16_t *queryCnt);

int set_recv_timeout(int sockfd, uint32_t timeout_ms);
int set_send_timeout(int sockfd, uint32_t timeout_ms);
extern const char *update_ok;
#endif