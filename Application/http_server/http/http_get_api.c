
#include <stdio.h>
#include <stdbool.h>

#include "cli_cmd.h"
#include "cmsis_os.h"
#include "http_common.h"
#include "http_get_api.h"
#include "ewrte.h"
#include "sockets.h"



#define GET_API_BUFF_SIZE 4096

int recv_httpBody(int conn, char *output, int outsize, int timeout_ms);


void send_get_api(uint8_t ip[4],uint32_t port,const char *path,
                     char *outBuff,uint32_t buffSize) 
{
  char *buff = EwAlloc(GET_API_BUFF_SIZE);
  int len=0;
  int sock;
  int bodyLen  =  0;

  if(buff == NULL)
  {
    snprintf(outBuff, buffSize,"[{\"messages\":\"EwAlloc failed\"}]");
    return ;
  }

  sock = connect_http(ip,port);

  if(sock == -1)
  {
    snprintf(outBuff, buffSize,"[{\"messages\":\"sock failed\"}]");
    return ;
  }
  else if(sock == -2)
  {
    snprintf(outBuff, buffSize,"[{\"messages\":\"server failed\"}]");
    return ;
  }

  snprintf(buff, GET_API_BUFF_SIZE,"GET %s HTTP/1.1\r\n"
                                     "Content-Length: 0\r\n\r\n",path);
    len = send(sock, buff, strlen(buff), 0);//네크워크 장애로 전송 실패시 네크워크 복구되면 일괄 전송

  bodyLen = recv_httpBody(sock,buff,GET_API_BUFF_SIZE,1000);
  if(bodyLen > 0)
  {
    //json 배열로 리턴,응답 메시지는 무조건 json이어야함
    len = snprintf(&outBuff[0], buffSize,"[{\"messages\":\"ok\"}");
    if((buffSize-len)>bodyLen)
    {
      len += snprintf(&outBuff[len], buffSize - len,",");
      memcpy(&outBuff[len],buff,bodyLen);
      len += bodyLen;
    }
      snprintf(&outBuff[len], buffSize-len,"]");
  }
  else //에러 값 처리 
  {
    if(buffSize<strlen(buff))
    {
      make_jsonMessages(outBuff,buffSize,buff);
    }
  }

  EwFree(buff);
  close_http(sock);


}


//errno
void send_post_api(uint8_t ip[4],uint32_t port,char *path,
                   char *data,char *outBuff,uint32_t buffSize)
{
  char *buff = EwAlloc(GET_API_BUFF_SIZE);
  int len=0;
  int bodyLen;
  int sock;

  sock = connect_http(ip,port);

  if(sock == -1)
  {
    snprintf(outBuff, buffSize,"[{\"messages\":\"sock failed\"}]");
    return ;
  }
  else if(sock == -2)
  {
    snprintf(outBuff, buffSize,"[{\"messages\":\"server failed\"}]");
    return ;
  }

  len = snprintf(buff, GET_API_BUFF_SIZE,"POST %s HTTP/1.1\r\n"
                                         "Content-Length: %d\r\n\r\n"
                                         ,path,strlen(data));

  memcpy(&buff[len],data,strlen(data));
  
  len += strlen(data);

  len = send(sock, buff, len, 0); 

  bodyLen = recv_httpBody(sock,buff,GET_API_BUFF_SIZE,10000);
  if(bodyLen > 0)
  {
    //json 배열로 리턴,응답 메시지는 무조건 json이어야함
    len = snprintf(&outBuff[0], buffSize,"[{\"response\":\"ok\"}");
    if((buffSize-len)>bodyLen)
    {
      len += snprintf(&outBuff[len], buffSize - len,",");
      memcpy(&outBuff[len],buff,bodyLen);
      len += bodyLen;
    }
    snprintf(&outBuff[len], buffSize-len,"]");
  }
  EwFree(buff);
  close_http(sock);
}







