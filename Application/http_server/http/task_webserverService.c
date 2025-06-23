
#include <stdint.h>
#include <time.h>

#include "AppShRAM.h"
#include "AppEth.h"
#include "board_rtcc.h"
#include "board_time.h"
#include "cmsis_os.h"
#include "ewrte.h"
#include "http_api.h"
#include "http_common.h"
#include "http_updateForm.h"
#include "http_websocketServer.h"
#include "pRtuBinRcv.h"
#include "pRtuParam.h"
#include "sockets.h"
#include "task_sriverAPI.h"
#include "webserver_request.h"

#define RECV_BUFF_SIZE 1024 /*recv함수의 수신 버퍼 크기*/


/**
 * @brief 웹서버 처리
 * @param conn 클라이언트 소켓
 */
void http_server_service(int conn) 
{
  char *recv_buffer =NULL;
  int ret;
  int err;



  if(set_recv_timeout(conn,60000)<0)
  {
    return;
  }
  

  recv_buffer = EwAlloc(RECV_BUFF_SIZE);
  if(!recv_buffer)
  {
    return;
  }

  do
  {
    do
    {
    ret = recv(conn, recv_buffer, RECV_BUFF_SIZE-1, 0);
    if(ret <= 0)
    {
      err = errno;
      if(err == EWOULDBLOCK || err == EAGAIN)
      {
        continue;
      }

      EwFree(recv_buffer);
      return;
    }
    }while(ret<=0);
    
    recv_buffer[ret] = 0; // 문자열로 만든다, post인데 upload인경우는 문자열 안됨
    //그러나 upload 처리하는곳에서 길이로 처리함,그외는 http는 전무 문자열열

    if(strncmp(recv_buffer,"GET /favicon.ico",16)==0)//아이콘 전송(화진)
    {
      send_icon(conn);
      continue;
    }
    else if(strncmp(recv_buffer, "GET /", 5) == 0)
    {
      if (strstr(recv_buffer, "Upgrade: websocket"))
      {
        handle_websocket_handshake(conn, recv_buffer);
        // WebSocket 데이터 처리 루프로 진입
        handle_websocket_connection(conn);
        break;
      }
      else if (strncmp(recv_buffer, "GET /console.html", 17) == 0)
      {
        send_html(conn); // HTML 페이지 전송
        break;
      }
      else if(strncmp(recv_buffer,"GET /api",8)==0)//api사용시 
      {
        http_api(conn,recv_buffer);
      }
      else if (strncmp(recv_buffer, "GET /update.html", 16) == 0)//html기반 업데이트
      {
        http_get_update(conn);
      }
      else
      {
        send_404_response(conn);
      }
    }
    else if (strncmp(recv_buffer, "POST /fw_update", 14) == 0)
    {
      http_post_uploadFile(conn,recv_buffer,ret,eFW_UPLOAD_UPDATE);
    }
    else if (strncmp(recv_buffer, "POST /fw_upload", 12) == 0)
    {
      http_post_uploadFile(conn,recv_buffer,ret,eFW_UPLOAD);
    }
    else if (strncmp(recv_buffer, "POST /file_upload", 12) == 0)
    {
      http_post_uploadFile(conn,recv_buffer,ret,eFW_FILE_UOLOAD);
    }
    else if(strncmp(recv_buffer,"POST /api/sriverObsOper/getStatusData",37)==0)//소하천천
    {
      http_getStatusData(conn,recv_buffer,ret);
    }
    else if(strncmp(recv_buffer,"POST /api/sriverObsOper/getBulkData",35)==0)
    {
      http_getBulkData(conn,recv_buffer,ret);
    }
    else if(strncmp(recv_buffer,"POST /api",9)==0)
    {
      http_api(conn,recv_buffer);
    }
    else
    {
      send_404_response(conn);
    }

  } while(0);
  
  if(recv_buffer)
  {
    EwFree(recv_buffer);
  }

}

/**
 * @brief 미사용
 */
void webserviceTask(void const *arg)
{
  int conn = (int)arg;

  http_server_service(conn);
  osThreadTerminate(NULL);

}

/**
 * @brief 멀티 유저를 위해 추가되었으나
 * 메모리 부족
 * 소켓 종료 처리안되어있어서 미사용
 */
void create_webserverService(void *arg) 
{
  osThreadDef(webserviceTask, webserviceTask, TASK_PRIORITY_WEBSERVER_SERVICE, 0,
              TASK_STACK_SIZE_WEBSERVICE);
  osThreadCreate(osThread(webserviceTask), (void *)arg);
}