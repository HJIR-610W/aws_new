#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ewrte.h"
#include "sockets.h"
#include "http_common.h"
const char *update_ok = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json\r\n"
                       "Content-Length: 18\r\n"
                       "\r\n"
                       "{\"success\":\"true\"}";

const char *http_404_response = 
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 144\r\n"
    "\r\n"
    "<html>"
    "<head><title>404 Not Found</title></head>"
    "<body>"
    "<h1>404 Not Found</h1>"
    "<p>The page you are looking for could not be found.</p>"
    "</body>"
    "</html>";





void http_parse(const char *http_request, http_request_t *req, uint16_t *pathCnt,
                uint16_t *queryCnt)
{
    char request_copy[512]; // 요청 문자열 복사본
    strncpy(request_copy, http_request, sizeof(request_copy) - 1);
    request_copy[sizeof(request_copy) - 1] = '\0'; // null-terminate

    char *query_start = NULL;
    char *saveptr_path;
    char *saveptr_query;
    uint16_t path_index = 0;
    uint16_t query_index = 0;

    // 쿼리 시작 위치 찾기
    query_start = strchr(request_copy, '?');

    // 쿼리 부분 분리
    if (query_start)
    {
        *query_start = '\0'; // '?'를 '\0'로 바꿔 URL 경로와 쿼리 문자열 분리
        query_start++;
    }

    // 경로 파싱
    char *token = strtok_r(request_copy, "/", &saveptr_path);
    while (token != NULL && path_index < MAX_PATHS)
    {
        strncpy(req->path[path_index], token, MAX_KEY_VALUE_LEN - 1);
        req->path[path_index][MAX_KEY_VALUE_LEN - 1] = '\0'; // null-terminate
        path_index++;
        token = strtok_r(NULL, "/", &saveptr_path);
    }
    *pathCnt = path_index; // 경로 개수 반환

    // 쿼리 파싱
    if (query_start)
    {
        token = strtok_r(query_start, "&", &saveptr_query);
        while (token != NULL && query_index < MAX_QUERIES)
        {
            char *key = strtok_r(token, "=", &saveptr_path);
            char *value = strtok_r(NULL, "=", &saveptr_path);

            if (key && value)
            {
                strncpy(req->query[query_index].key, key, MAX_KEY_VALUE_LEN - 1);
                req->query[query_index].key[MAX_KEY_VALUE_LEN - 1] = '\0';
                strncpy(req->query[query_index].value, value, MAX_KEY_VALUE_LEN - 1);
                req->query[query_index].value[MAX_KEY_VALUE_LEN - 1] = '\0';
                query_index++;
            }
            token = strtok_r(NULL, "&", &saveptr_query);
        }
    }
    *queryCnt = query_index; // 쿼리 개수 반환
}



void send_404_response(int conn)
{
  write(conn, (const unsigned char*)http_404_response, (size_t)strlen((char *)http_404_response));
}
                       
long get_content_length(const char *request) 
{
  char *endptr;
  char *content_length_str = strstr(request, "Content-Length: ");
    
    if(content_length_str)
    {
        return strtol(content_length_str + strlen("Content-Length: "),&endptr,10);
    }
    return -1; // Content-Length가 없으면 -1 반환
}






/**
 * @brief html ok 200 헤더 생성
 */
uint32_t make_http_ok_header(char *out,uint32_t outSize,char *body,const char *content_type)
{
  int32_t len=0;

  len += snprintf(&out[len],outSize - len,"HTTP/1.1 200 OK\r\n");
  len += snprintf(&out[len],outSize - len,"Content-Type: %s; charset=UTF-8\r\n",content_type);
  len += snprintf(&out[len],outSize - len,"Content-Length: %d\r\n\r\n",strlen(body));
  
  return len;
}


int parse_get_para(char *input, char **key, char **value,uint32_t max_prams)
{
    char *query_start =input;

    int count = 0;

    while (query_start && count < max_prams) {
        char *eq = strchr(query_start, '='); // '=' 위치 찾기
        if (!eq) break; // '='가 없으면 종료

        *eq = '\0'; // '='를 NULL로 변경하여 key 종료
        key[count] = query_start; // key에 시작 주소 저장

        query_start = eq + 1; // value 시작 위치로 이동
        char *amp = strchr(query_start, '&'); // '&' 위치 찾기
        if (amp) {
            *amp = '\0'; // '&'를 NULL로 변경하여 value 종료
            value[count] = query_start; // value에 시작 주소 저장
            query_start = amp + 1; // 다음 파라미터로 이동
        } else {
            value[count] = query_start; // 마지막 value 처리
            query_start = NULL; // 더 이상 파라미터 없음
        }

        count++;
    }

    return count; // 파싱된 파라미터 개수 반환
}


int parse_json_params(char* input, char** key, char** value, uint32_t max_params)
{
  char* ptr = (char*)input; 
  uint32_t param_count = 0;

  while (*ptr != '\0' && param_count < max_params) {
    // 키 시작 위치 탐색
    if (*ptr == '"') {
      key[param_count] = ptr + 1; // 키의 시작 주소 저장
      ptr = strchr(ptr + 1, '"'); // 키의 끝 " 위치 찾기
      if (ptr == NULL) {
        return 0; // 잘못된 JSON 형식
      }

      *ptr = '\0'; // "를 NULL로 처리
      ptr++; // 다음으로 이동

      // 값의 시작 위치 탐색
      ptr = strchr(ptr, ':');
      if (ptr == NULL) {
        return 0; // 잘못된 JSON 형식
      }

      ptr++; // : 다음으로 이동

      while (*ptr == ' ' || *ptr == '"') {
        ptr++; // 공백이나 " 스킵
      }

      value[param_count] = ptr; // 값의 시작 주소 저장

      // 값의 끝 위치 탐색
      if (*ptr == '"') {
        ptr = strchr(ptr, '"');
        if (ptr == NULL) {
          return 0; // 잘못된 JSON 형식
        }
      }
      else {
        while (*ptr != ',' && *ptr != '}' && *ptr != '\0') {
          ptr++; // , 또는 } 또는 문자열 끝까지 이동
        }
      }

      if (*ptr == ',' || *ptr == '}') {
        *ptr = '\0'; // 값의 끝에 NULL 추가
        ptr++; // 다음으로 이동
      }

      param_count++; // 파싱된 파라미터 증가
    }
    else {
      ptr++; // 다음 문자로 이동
    }
  }

  return param_count; // 파싱된 파라미터 수 반환
}

/**
 * @brief 비차단모드로 connect하고 바로 연결이 안되면 타임아웃안에 연결 시도
 * 연결되면 차단모드로 변경되어 recv,send가 차단모드로 동작하도록함함
 */
int32_t connect_with_timeout(int sock, const struct sockaddr *addr, socklen_t addrlen, int timeout_ms)
{
    // 1. 소켓을 비차단 모드로 설정,connet, recv,send가 즉시 반환되도록 설정
    int32_t flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
       // perror("fcntl get failed");
        return -1;
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
       // perror("fcntl set failed");
        return -1;
    }
    // 2. lwip_connect 호출, 즉시 연결되지 않더라도 바로 리턴턴
    int32_t result = connect(sock, addr, addrlen);
    if (result == 0)
    {
        // 즉시 연결 성공
        fcntl(sock, F_SETFL, flags); // 원래 모드로 복구
        return 0;
    }
    else if(errno != EINPROGRESS)
    {
      // EINPROGRESS가 아닌 경우 실제 오류
       // perror("lwip_connect failed");
        return -1;
    }

//여기까지 왔다는건 서버연결이 안되었다는것

    // 3. select로 타임아웃 처리
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);

    result = select(sock + 1, NULL, &writefds, NULL, &tv);
    if (result > 0) {
        // 소켓이 쓰기 가능 상태인지 확인
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error == 0) {
            fcntl(sock, F_SETFL, flags); // 원래 모드로 복구
            return 0; // 연결 성공
        } else {
            errno = so_error;
           // perror("connect failed");
            return -1;
        }
    } else if (result == 0) {
        // 타임아웃 발생
        errno = ETIMEDOUT;
      //printf("Connection timed out\n");
        return -1;
    }else{
        // select 오류
      //perror("select failed");
        return -1;
    }
}

/**
 * @brief 서버에 비차단 모드로 연결하고 소켓 리턴
 */
int32_t connect_http(uint8_t ip[4],uint32_t port)
{
  char buff[50];
  int32_t sock;
  struct sockaddr_in address;
  struct timeval
  {
        long tv_sec;         /* seconds */
        long tv_usec;        /* and microseconds */
  }opt;

  sock = socket(PF_INET,SOCK_STREAM,0);

  if(sock == -1)
  {
    return -1;
  }

  address.sin_family = AF_INET;
  address.sin_port   = htons(port);
  snprintf(buff,sizeof(buff),"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]) ;      
  address.sin_addr.s_addr = inet_addr(buff);    

  if (connect_with_timeout(sock, (struct sockaddr *)&address, sizeof(address), 1000) != 0) 
  {
    closesocket(sock);
    return -2;
  }

  opt.tv_sec = 2;
  opt.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(opt));
 
  return sock;

}

void close_http(int conn)
{
  closesocket(conn);
}



#define RECV_BUFFER_SIZE 1024 // 1KB 버퍼 크기

int recv_httpBody(int conn, char *output, int outsize, int timeout_ms)
{
  char *recv_buffer = NULL;
  int total_received = 0; // 전체 바디 수신된 크기
  int content_length = -1; // Content-Length 값
  int header_parsed = 0; // 헤더 파싱 완료 여부
  int body_start = 0; // 바디 시작 위치
  struct timeval timeout;

  recv_buffer = EwAlloc(RECV_BUFFER_SIZE);

  if(recv_buffer == NULL)
  {
    return 0;
  }

  // 타임아웃 설정
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  if (setsockopt(conn, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) 
  {
      snprintf(output,outsize,"Failed to set socket timeout\n");
      EwFree(recv_buffer);
    return -1;
  }

  // output 초기화
  memset(output, 0, outsize);

  while (1) 
  {
    // 데이터 수신
    int received = recv(conn, recv_buffer, RECV_BUFFER_SIZE - 1, 0);

    if (received < 0) 
    {
      if (errno == EWOULDBLOCK) 
      {
        // 타임아웃 발생
        snprintf(output,outsize,"Receive timed out\n");
        break;
      } 
      else 
      {
        // 수신 오류
        snprintf(output,outsize,"Receive error: %d\n", errno);
        return -1;
      }
    } 
    else if (received == 0) 
    {
      // 연결 종료
      snprintf(output,outsize,"Connection closed by peer\n");
      break;
    }

    recv_buffer[received] = '\0'; // null-terminate 처리

    // 헤더 파싱
    if (!header_parsed) 
    {
      char *header_end = strstr(recv_buffer, "\r\n\r\n"); // 헤더 끝 찾기
      if (header_end) 
      {
        header_parsed = 1;

        // Content-Length 확인
        char *content_length_str = strstr(recv_buffer, "Content-Length:");
        if (content_length_str) 
        {
          content_length = atoi(content_length_str + strlen("Content-Length:"));
        }

        if (content_length <= 0 || content_length >= outsize) 
        {
          snprintf(output,outsize,"Invalid Content-Length: %d\n", content_length);
          return -2;
        }

        // 바디 데이터 시작 위치 계산
        body_start = (header_end - recv_buffer) + 4; // "\r\n\r\n" 길이 추가
        int body_length = received - body_start;

        // 바디 데이터 복사
        if (body_length > 0) 
        {
          memcpy(output, recv_buffer + body_start, body_length);
          total_received += body_length;
        }
      }
    } 
    else 
    {
      // 헤더 파싱 후 바디 데이터만 추가 복사
      memcpy(output + total_received, recv_buffer, received);
      total_received += received;
    }

    // 모든 바디 데이터를 수신했는지 확인
    if (header_parsed && total_received >= content_length) 
    {
      output[content_length] = '\0'; // null-terminate 처리
      break;
    }
  }

  // 바디 데이터가 충분히 수신되지 않았으면 오류 처리
  if (total_received < content_length) 
  {
    snprintf(output,outsize,"Incomplete body received\n");
      EwFree(recv_buffer);
    return -3;
  }

  EwFree(recv_buffer);
  return total_received; // 실제 수신된 바디 데이터 크기 반환
}



int set_recv_timeout(int sockfd, uint32_t timeout_ms)
{
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
      //  perror("Failed to set SO_RCVTIMEO");
      return -1;
    }
    
    return 0;
}

int set_send_timeout(int sockfd, uint32_t timeout_ms)
{
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
      //  perror("Failed to set SO_RCVTIMEO");
      return -1;
    }
    
    return 0;
}