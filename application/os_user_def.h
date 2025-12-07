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

#if FREE_RTOS_USE
#define OS_SEM_DELETE(sem)          \
  do                           \
  {                            \
    if (sem)                   \
    {                          \
      osSemaphoreDelete(sem); \
      sem = NULL;          \
    }                          \
  } while (0)
#else
#define OS_SEM_DELETE(sem) ((void)0)
#endif




#if FREE_RTOS_USE
#define OS_CREATE_MUTEX(myMutex)                 \
  do                                    \
  {                                     \
    if (myMutex == NULL)                    \
    {                                   \
      myMutex = osMutexNew(NULL); \
    }                                   \
  } while (0)
#else
#define OS_CREATE_MUTEX(myMutex) ((void)0)
#endif





#if FREE_RTOS_USE
#define OS_MUTEX_LOCK(myMutex, timeout)          \
  do                                    \
  {                                     \
    if (myMutex)                            \
    {                                   \
       osMutexAcquire(myMutex, timeout); \
    }                                   \
  } while (0)
#else
#define OS_MUTEX_LOCK(myMutex, timeout) ((void)0)
#endif

#if FREE_RTOS_USE
#define OS_MUTEX_UNLOCK(myMutex)          \
  do                           \
  {                            \
    if (myMutex)                   \
    {                          \
      osMutexRelease(myMutex); \
    }                          \
  } while (0)
#else
#define OS_MUTEX_UNLOCK(myMutex) ((void)0)
#endif

#if FREE_RTOS_USE
#define OS_MUTEX_DELETE(myMutex)          \
  do                           \
  {                            \
    if (myMutex)                   \
    {                          \
      osMutexDelete(myMutex); \
      myMutex = NULL;          \
    }                          \
  } while (0)
#else
#define OS_MUTEX_DELETE(myMutex) ((void)0)
#endif


#define OS_GET_TICK() osKernelGetTickCount()

#define OS_TICK_COUNT 1000

#endif