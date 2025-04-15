
#ifndef DRIVER_INTERFACE_H
#define DRIVER_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct driver_s
{
  const char *name;       //driver 이름
  const void* api;        //driver api 모음
  void* cfg;              //driver 자체 속성
  bool opened;            //driver 초기화 여부 
  void* sem;              //공유자원 충돌
  struct driver_s* handle;//종속된 driver
}driver_t;


#define DRV_ERR_NONE 0
#define DRV_ERR_HANDLE  1
#define DRV_ERR_TIMEOUT 2
#define DRV_ERR_RECV_DATA    3

#endif
