
#include <string.h>

#include "AppDevTest.h"
#include "AppFetch.h"
#include "AppShRAM.h"
#include "board_address.h"
#include "board_rtcc.h"
#include "cmsis_os.h"
#include "cli_cmd.h"
#include "ewrte.h"
#include "fw_sdcd.h"
#include "http_updateForm.h"
#include "http_fw.h"
#include "http_common.h"
#include "main.h"
#include "ProcShare.h"
#include "pRtuBinRcv.h"
#include "pRtuParam.h"
#include "RtosTimer.h"
#include "sockets.h"


#define FW_BUFF_SIZE     2*1024*1024 /*2MB*/


#define UPLOAD_DATA_SIZE 2*1024*1024 /*2MB*/
#define RECV_BUFF_SIZE   4096  

const char *html = 
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>Firmware Update</title>\n"
"    <style>\n"
"        #progress-container {\n"
"            width: 100%;\n"
"            background-color: #f3f3f3;\n"
"            border: 1px solid #ccc;\n"
"            border-radius: 5px;\n"
"            margin: 20px 0;\n"
"            padding: 3px;\n"
"        }\n"
"        #progress-bar {\n"
"            height: 20px;\n"
"            width: 0;\n"
"            background-color: #4caf50;\n"
"            text-align: center;\n"
"            line-height: 20px;\n"
"            color: white;\n"
"            border-radius: 5px;\n"
"        }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <h1>Firmware Update</h1>\n"
"    <form id=\"upload-form\">\n"
"        <input type=\"file\" id=\"file\" name=\"file\" required>\n"
"        <button type=\"submit\">Upload</button>\n"
"    </form>\n"
"    <div id=\"progress-container\">\n"
"        <div id=\"progress-bar\">0%</div>\n"
"    </div>\n"
"    <script>\n"
"        document.getElementById('upload-form').addEventListener('submit', function (e) {\n"
"            e.preventDefault();\n"
"            const fileInput = document.getElementById('file');\n"
"            const file = fileInput.files[0];\n"
"            if (!file) {\n"
"                alert('Please select a file to upload.');\n"
"                return;\n"
"            }\n"
"            const formData = new FormData();\n"
"            formData.append('file', file);\n"
"            const xhr = new XMLHttpRequest();\n"
"            xhr.upload.addEventListener('progress', function (e) {\n"
"                if (e.lengthComputable) {\n"
"                    const percentComplete = Math.round((e.loaded / e.total) * 100);\n"
"                    const progressBar = document.getElementById('progress-bar');\n"
"                    progressBar.style.width = percentComplete + '%';\n"
"                    progressBar.textContent = percentComplete + '%';\n"
"                }\n"
"            });\n"
"            xhr.addEventListener('load', function () {\n"
"                if (xhr.status === 200) {\n"
"                    const response = JSON.parse(xhr.responseText);\n"
"                    alert(JSON.stringify(response, null, 2));"
"                } else {\n"
"                    alert('Failed to upload file. Server responded with status: ' + xhr.status);\n"
"                }\n"
"            });\n"
"            xhr.addEventListener('error', function () {\n"
"                alert('Upload failed due to a network error.');\n"
"            });\n"
"            xhr.open('POST', '/fw_update');\n"
"            xhr.send(formData);\n"
"        });\n"
"    </script>\n";
/*
"</body>\n"
"</html>\n";
*/


/**
 * @brief post upload 데이터에서 원본 데이터만 추출
 * @param post_body 
 */
uint32_t parse_postUploadData(char *post_body, uint32_t body_len, uint8_t *out, uint32_t outSize) 
{
  char boundary[128];
  char *boundary_end = strstr(post_body, "\r\n");


  memset(out,0,outSize);
  strncpy(boundary, post_body, boundary_end - post_body);
  boundary[boundary_end - post_body] = '\0';

  // Find the beginning of the file data
  char *data_start = strstr(boundary_end, "\r\n\r\n");
  if (!data_start) {
    return 0; // File data start not found
  }

  data_start += 4; // Skip \r\n\r\n
  
  uint32_t data_len = &post_body[body_len-1] - data_start - strlen(boundary) -5;


  // Copy the file data to the output buffer
  memcpy(out, data_start, data_len);

  return data_len;
}


void write_response(int conn,char *msg)
{
  char header[200];

  make_http_ok_header((char*)header,sizeof(header),msg,"application/json");  
  write(conn, (const unsigned char*)(header), (size_t)strlen((char *)header));
  write(conn, (const unsigned char*)(msg), (size_t)strlen(msg));
}
  int total_received =0;


#define CONTENT_LENGTH_STR "Content-Length: " //공백 있어야함함



void http_post_uploadFile(int conn,char *post,uint32_t len,ePOST_FILE_TYPE_t fileType)
{
  const char *product_name;
  char *content_length_str = strstr(post, CONTENT_LENGTH_STR);
  bool result=false;
  char *body=NULL;
  char *header=NULL;
  char buff[100];

  int content_length = 0;
  uint32_t fwSize;
  uint32_t product_code=0;



  if(g_fwBuff==NULL)
  {
    g_fwBuff = EwAlloc(UPLOAD_FILE_SIZE);
  }

  if (content_length_str)
  {
    content_length_str += strlen(CONTENT_LENGTH_STR);
    content_length = atoi(content_length_str);
  } 
  else
  {
    return;
  }

  body = EwAlloc(FW_BUFF_SIZE);
  
  if(body==0)
  {
    write_response(conn,"{\"message\":\"mem fail\"}");
    return;
  }
  // Find the end of the headers
  char *body_start = strstr(post, "\r\n\r\n");
  if (!body_start)
  {
    write_response(conn,"{\"message\":\"recv err\"}");
    return;
  }

  // Move to the start of the body
  body_start += 4; // Skip "\r\n\r\n"
  int received_body_len =  len - (body_start - post);

  // Copy the already received body into g_buff
  if (received_body_len > 0)
  {
    memcpy(body, body_start, received_body_len);
  }

 total_received = received_body_len;

  // Set recv timeout to 100ms
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec =  1000; 
  if (setsockopt(conn, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    write_response(conn,"{\"message\":\"socket err\"}");
    return;
  }
  // Receive remaining data
  while (total_received < content_length)
  {
    int remaining = content_length - total_received;
    int to_receive = (remaining > 1024) ? 1024 : remaining;

    int bytes = recv(conn, body + total_received, to_receive, 0);
    if (bytes > 0) {
      total_received += bytes;
    } else if (bytes == 0) {

      break;
    } else {
      if ( errno == EWOULDBLOCK) {
        break;
      } else {

        break;
      }
    }
  }

  if(total_received != content_length)
  {
    write_response(conn,"{\"message\":\"Some of the data was not successfully received\"}");
    
    if(body)
    {
      EwFree(body);
    }
    return;
  }

  if( content_length == 0)
  {
    write_response(conn,"{\"message\":\"Please upload file\"}");
    
    if(body)
    {
      EwFree(body);
    }
    return;
  }




  fwSize = parse_postUploadData((char *)body,content_length,g_fwBuff,UPLOAD_FILE_SIZE);

  g_fwLen = fwSize;
 
  if(fileType == eFW_FILE_UOLOAD)
  {
    snprintf(body,FW_BUFF_SIZE,"{\"message\":\"File uploaded successfully\",\"file size\":%d}",g_fwLen);
    write_response(conn,body);
    EwFree(body);
    return;
  }

#define HEADER_BUFF_SIZE 512
  header = EwAlloc(HEADER_BUFF_SIZE);


  result = check_fwFileCRC(g_fwBuff,g_fwLen,&product_code);

  if(result==true&&g_fwLen)
  {
    product_name = convert_pcodeTostr(product_code);
    snprintf(buff,sizeof(buff),"{\"message\":\"File uploaded successfully\",\"err\":\"-\",\"device\":\"%s\"}",product_name);

    if(fileType == eFW_UPLOAD_UPDATE)
    {
      uint8_t err;
      char temp[30];

      err = update_localUpdate();
      if(err)
      {
        make_updateErrMsgJson(err,temp,sizeof(temp));
        snprintf(buff,sizeof(buff),"{\"message\":\"File uploaded successfully\","
        "\"err\":%s,\"device\":\"unknown\"}",temp);
        g_fwLen = 0;
      }
    }

  }
  else
  {
    g_fwLen = 0;
    snprintf(buff,sizeof(buff),"{\"message\":\"crc fail\",\"file_len\":%d}",g_fwLen);
  }

  make_http_ok_header((char*)header,HEADER_BUFF_SIZE,buff,"application/json");  
  send(conn, (const unsigned char*)(header), (size_t)strlen((char *)header),0);
  send(conn, (const unsigned char*)(buff), (size_t)strlen(buff),0);
  
  if(header)
  {
    EwFree(header);
  }

  if(body)
  {
    EwFree(body);
  }
  
  osDelay(1000);
}




uint32_t make_upload_html_ajax(char *out,uint32_t outSize)
{
  int32_t len = 0;
  uint32_t ver;


  len += snprintf(&out[len],outSize - len,"%s",html);
  len += snprintf(&out[len],outSize - len,"%04d-%02d-%02d %02d:%02d:%02d<br>",
  Date_Time.Year,Date_Time.Month,Date_Time.Day,Date_Time.Hour,Date_Time.Min,Date_Time.Sec);
    len += snprintf(&out[len],outSize - len,"RESET CNT:%d<br>",ConfigRTU.m_cRstCnt);
    len += snprintf(&out[len],outSize - len,"ID:%d<br>",ConfigRTU.RtuId);

  ver  = get_fw_ver();
  len += snprintf(&out[len],outSize - len,"767_App:%d.%d.%d<br>",ver/10000, (ver%10000)/100,ver%100);
  ver  = get_fw_pcbVer();
  len += snprintf(&out[len],outSize - len,"PCB:%d.%d<br>",ver/10,ver%10);
  len += snprintf(&out[len],outSize - len,"MFG:%s<br>",get_mfg_name());
  len += snprintf(&out[len],outSize - len,"AREA:%d<br>",get_fw_area());
  len += snprintf(&out[len],outSize - len,"DATE:%d<br>",get_fw_buildDate());
  len += snprintf(&out[len],outSize - len,"TIME:%d<br>",get_fw_buildTime());

  ver =  SystemRTU.tftAppVer ;
  len += snprintf(&out[len],outSize - len,"767_Boot:%d.%d.%d<br>",ver/10000, (ver%10000)/100,ver%100);
  len += snprintf(&out[len],outSize - len,"TIME:%d<br>", SystemRTU.tftBootBuildTime);

  ver = SystemRTU.subAppVer;
  len += snprintf(&out[len],outSize - len,"471_App:%d.%d.%d<br>",ver/10000, (ver%10000)/100,ver%100);
  len += snprintf(&out[len],outSize - len,"TIME:%d<br>", SystemRTU.subAppBuildTime);
  len += snprintf(&out[len],outSize - len,"471_Boot:%d<br>", SystemRTU.subBootVer);
  len += snprintf(&out[len],outSize - len,"TIME:%d<br>", SystemRTU.subBootBuildTime);



  len += snprintf(&out[len],outSize - len,"</body>\n");
  len += snprintf(&out[len],outSize - len,"</html>\n");

return len;
}



#define GET_UPLOAD_HTML_SIZE 4096
void http_get_update(int conn)
{
  char *get_html = NULL;
  char buff[512];
  
  get_html = EwAlloc(GET_UPLOAD_HTML_SIZE);

  if(get_html)
  {
    make_upload_html_ajax(get_html,GET_UPLOAD_HTML_SIZE);
    make_http_ok_header(buff,sizeof(buff),get_html,"text/html");
  
    write(conn, (const unsigned char*)(buff), (size_t)strlen(buff));
    write(conn, (const unsigned char*)(get_html), (size_t)strlen(get_html));
    EwFree(get_html);
  }
}





