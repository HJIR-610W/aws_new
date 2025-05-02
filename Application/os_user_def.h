/*
os함수가 길어서 쉬운 용어로 사용
*/
#ifndef OS_USER_DEF_H
#define OS_USER_DEF_H

#include "cmsis_os2.h"

#define CREATE_BINARY_SEM(sem)                 \
  do                                    \
  {                                     \
    if (sem == NULL)                    \
    {                                   \
      sem = osSemaphoreNew(1, 1, NULL); \
    }                                   \
  } while (0)

#define PEND_SEM(sem, timeout)          \
  do                                    \
  {                                     \
    if (sem)                            \
    {                                   \
      osSemaphoreAcquire(sem, timeout); \
    }                                   \
  } while (0)

#define POST_SEM(sem)          \
  do                           \
  {                            \
    if (sem)                   \
    {                          \
      osSemaphoreRelease(sem); \
    }                          \
  } while (0)
#endif

#define GET_TICK() osKernelGetTickCount()