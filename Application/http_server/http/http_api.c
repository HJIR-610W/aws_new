#include <stdint.h>
#include <string.h>

#include "cli_cmd.h"
#include "ewrte.h"
#include "http_common.h"
#include "sockets.h"


//GET /api/read_config?offset=0&len=10
//GET /api/read_system
//POST /api/write_config
//GET /api/{명령어}?파라미터



extern uint32_t parse_argsWithToken(char* str, char* argv[10], char token);

#define OUTBUFF_SIZE 10*1024


/* 
GET  /api/{명령어}?{키}={값} HTTP/1.1<CR><LF>

예) GET /api/config?offset=0&cnt=10


POST /api/{명령어} HTTP/1.1<CR><LF>


예)POST /api/config HTTP/1.1<CR><LF>
body에는 json 형태로 매개변수 전달
{"pw":"7777","offset":100,"data":"303030","dataLen":3}
*/
void http_api(int conn,char *recv_buff)
{
  char *resource[3];
  char *pBodyStart = NULL;
  char *pParaStart=NULL;
  char *pEnd=NULL;
  char *outBuff=NULL;
  char buff[100];


  if(strstr(recv_buff,"GET"))//GET 처리
  {
      pEnd = strstr(recv_buff," HTTP");
      *pEnd = 0;//cmd=read_config&offset=0&readCnt=10 만 전달 목적적

      pParaStart = strchr(recv_buff,'?');

      if(pParaStart)
      {
        *pParaStart = 0;
        pParaStart++;
      }
      parse_argsWithToken(recv_buff,resource,'/');
      outBuff = EwAlloc(OUTBUFF_SIZE);
      if(outBuff)
      {
        cli_cmd_get(resource[2],pParaStart,outBuff,OUTBUFF_SIZE);

        make_http_ok_header((char *)buff,sizeof(buff),outBuff,"application/json");
        write(conn, (const uint8_t*)(buff), (size_t)strlen((char *)buff));
        send(conn, outBuff, strlen(outBuff), 0);
     
        EwFree(outBuff);
      }
  
  }
  else//POST 처리,body는 {"cmd":"write_config","offset":11,"data":"303030","dataLen":3}
  {
      pBodyStart = strstr(recv_buff,"\r\n\r\n");
      if(pBodyStart)
      {
        pBodyStart +=4;
      }

      pEnd = strstr(recv_buff," HTTP");
      *pEnd = 0;//cmd=read_config&offset=0&readCnt=10 만 전달 목적적

      parse_argsWithToken(recv_buff,resource,'/');


      outBuff = EwAlloc(OUTBUFF_SIZE);
      if(outBuff)
      {
        cli_cmd_post(resource[2],pBodyStart,outBuff,OUTBUFF_SIZE);
        make_http_ok_header((char *)buff,sizeof(buff),outBuff,"application/json");

        write(conn, (const uint8_t*)(buff), (size_t)strlen((char *)buff));
        send(conn, outBuff, strlen(outBuff), 0);
 
        EwFree(outBuff);
      }
  }

}