

#ifndef HTTP_UPDATE_H
#define HTTP_UPDATE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
  eFW_UPLOAD_UPDATE, //펌웨어 파일을 업로드하고 업데이트까지 진행
  eFW_UPLOAD,        //펌웨어 파일을 업로드하고 crc체크까지만 진행
  eFW_FILE_UOLOAD    //일반 파일을 업로드함,다양한 목적 활용용
}ePOST_FILE_TYPE_t;

void http_post_uploadFile(int conn,char *post,uint32_t len,ePOST_FILE_TYPE_t fileType);
void http_get_update(int conn);
int update_firmware(uint8_t *firmware,size_t fwLen);

#endif
