/*
os함수가 길어서 쉬운 용어로 사용
*/
#ifndef OS_USER_DEF_H
#define OS_USER_DEF_H

#define FREE_RTOS_USE 1

#include "cmsis_os2.h"
#include "FreeRTOS.h"

#if FREE_RTOS_USE
#define OS_CREATE_BINARY_SEM(sem)                 \
  do                                    \
  {                                     \
    if (sem == NULL)                    \
    {                                   \
      sem = osSemaphoreNew(1, 1, NULL); \
    }                                   \
  } while (0)
#else
#define OS_CREATE_BINARY_SEM(sem) ((void)0)
#endif



#if FREE_RTOS_USE
#define OS_PEND_SEM(sem, timeout)          \
  do                                    \
  {                                     \
    if (sem)                            \
    {                                   \
      osSemaphoreAcquire(sem, timeout); \
    }                                   \
  } while (0)
#else
#define OS_PEND_SEM(sem, timeout) ((void)0)
#endif

#if FREE_RTOS_USE
#define OS_POST_SEM(sem)          \
  do                           \
  {                            \
    if (sem)                   \
    {                          \
      osSemaphoreRelease(sem); \
    }                          \
  } while (0)
#else
#define OS_POST_SEM(sem) ((void)0)
#endif


#define OS_GET_TICK() osKernelGetTickCount()
    
    
#endif