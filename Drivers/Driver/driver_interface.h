
#ifndef DRIVER_INTERFACE_H
#define DRIVER_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct driver_s
{
  const char *name;
  const void* api;        //driver가 제공하는 api 모음
  void* cfg;              //driver가 제공에 필요한 정보구성
  bool opened;            //driver 초기화 여부 
  void* sem;              //공유자원 충돌
  struct driver_s* handle;//종속된 driver
}driver_t;


typedef struct driver_new_s
{
  const char *name;      
  const void *api;        
  void *cfg;              
  bool opened;            
  void *sem;  
  // struct driver_new_s *next;  
}driver_new_t;



#endif
