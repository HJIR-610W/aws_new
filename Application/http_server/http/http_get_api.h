
#ifndef HTTP_GET_API_H
#define HTTP_GET_API_H

#include <stdint.h>

void send_get_api(uint8_t ip[4],uint32_t port,const char *path,
                     char *outBuff,uint32_t buffSize) ;
void send_post_api(uint8_t ip[4],uint32_t port,char *path,
                      char *data,char *outBuff,uint32_t buffSize);               
#endif