
#include <stdio.h>
#include <stdbool.h>

#include "cmsis_os.h"
#include "cli_cmd.h"
#include "ewrte.h"
#include "http_common.h"
#include "http_fw.h"
#include "http_fwUpdateClient.h"
#include "sockets.h"


#define POST_URL "/fw_upload"
#define BUFFER_SIZE 1024

/**
 * @brief 파일 전송 POST upload 방식의 클라이언트 구현
 */
void fileUploadTask(void const *arg)
{
  int sock;
  int len;
  char *send_buffer;
  char *recv_buffer;

  fwTarget_t *pTarget = (fwTarget_t *)arg;
  char add[30];
  char bodyContent[100];

  snprintf(add,sizeof(add),"%d.%d.%d.%d",pTarget->ip[0],pTarget->ip[1],pTarget->ip[2],pTarget->ip[3]);

  // Large file simulation
  uint8_t*file_data = g_fwBuff;
  uint32_t file_data_len = g_fwLen;

  // Boundary for multipart/form-data
  const char *boundary = "--------------------------boundary123";

  // Calculate content-length for headers
  snprintf(bodyContent,sizeof(bodyContent),"Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n\r\n",pTarget->fileName);

  size_t content_length = file_data_len + strlen(boundary) * 2 + strlen(bodyContent) + 8;


  send_buffer = EwAlloc(BUFFER_SIZE);
  recv_buffer = EwAlloc(BUFFER_SIZE);
  // Construct HTTP POST headers
  snprintf(send_buffer, BUFFER_SIZE,
           "POST %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Content-Type: multipart/form-data; boundary=%s\r\n"
           "Content-Length: %zu\r\n"
           "Connection: close\r\n\r\n",
           POST_URL, add, boundary, content_length);


  sock = connect_http(pTarget->ip,pTarget->port);
  if(sock == -1)
  {
    send_response("[{\"messages\":\"sock failed\"}]");
    EwFree(send_buffer);
    EwFree(recv_buffer);
    osThreadTerminate(NULL);
  }
  else if(sock == -2)
  {
    send_response("[{\"messages\":\"server failed\"}]");
    EwFree(send_buffer);
    EwFree(recv_buffer);
    osThreadTerminate(NULL);
  }

  // Set socket timeout
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  // Send HTTP headers first
  if (send(sock, send_buffer, strlen(send_buffer), 0) < 0)
  {
    send_response("[{\"messages\":\"Failed to send headers\"}]");
    close(sock);
    EwFree(send_buffer);
    EwFree(recv_buffer);
    osThreadTerminate(NULL);
  }

  snprintf(send_buffer, BUFFER_SIZE,"%s\r\n%s", boundary,bodyContent);


  if(send(sock, send_buffer, strlen(send_buffer), 0) < 0)
  {
    send_response("[{\"messages\":\"Failed to send multipart header\"}]");
    closesocket(sock);
    EwFree(send_buffer);
    EwFree(recv_buffer);
    osThreadTerminate(NULL);
  }

  // Send file data in chunks
  size_t sent_bytes = 0;
  while (sent_bytes < file_data_len) {
    size_t chunk_size = (file_data_len - sent_bytes > BUFFER_SIZE) ? BUFFER_SIZE : (file_data_len - sent_bytes);
    if (send(sock, file_data + sent_bytes, chunk_size, 0) < 0) 
    {
      send_response("[{\"messages\":\"Failed to send file data\"}]");
      closesocket(sock);
      EwFree(send_buffer);
      EwFree(recv_buffer);
      osThreadTerminate(NULL);
    }
    sent_bytes += chunk_size;
  }

  // Send closing boundary
  snprintf(send_buffer, BUFFER_SIZE, "\r\n%s--\r\n", boundary);
  if (send(sock, send_buffer, strlen(send_buffer), 0) < 0)
  {
    send_response("[{\"messages\":\"Failed to send closing boundary\"}]");
    closesocket(sock);
    EwFree(send_buffer);
    EwFree(recv_buffer);
    osThreadTerminate(NULL);
  }

  len = recv_httpBody(sock,recv_buffer,BUFFER_SIZE,pTarget->waitTimeoutms);

  if(len > 0)
  {
    recv_buffer[len]=0;
    snprintf(send_buffer,BUFFER_SIZE,"[{\"response\":\"ok\"},%s]",recv_buffer);
  }
  else
  {
    snprintf(send_buffer,BUFFER_SIZE,"[{\"response\":\"timeout\"}]");
  }

  send_response(send_buffer);
  closesocket(sock);

  EwFree(send_buffer);
  EwFree(recv_buffer);

  osDelay(2000);
  osThreadTerminate(NULL);
}

void create_fwUpdateClientTask(fwTarget_t *pfwTarget)
{
  osThreadDef(fwUpdateClientTask, fileUploadTask, TASK_PRIORITY_FWUPDATE_CLIENT, 0, TASK_STACK_SIZE_WEBSERVER);
  osThreadCreate(osThread(fwUpdateClientTask), (void *)pfwTarget);
}