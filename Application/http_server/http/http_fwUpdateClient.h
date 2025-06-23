
#ifndef HTTP_FWUPDATE_CLIENT_H
#define HTTP_FWUPDATE_CLIENT_H

#include <stdint.h>

typedef struct fwTarget_s
{
  uint8_t ip[4];
  uint32_t port;
  uint8_t *pData;
  uint32_t dataLen;
  const char *fileName;
  uint32_t waitTimeoutms;
}fwTarget_t;


void create_fwUpdateClientTask(fwTarget_t *pfwTarget);


#endif