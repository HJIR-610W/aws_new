
#ifndef OS_DEFINE_H
#define OS_DEFINE_H

#include "cmsis_os2.h"

#define OS_SEM_PEND(sem,timeout) osSemaphoreAcquire(sem, timeout)
#define OS_SEM_POST(sem)         osSemaphoreRelease(sem)

#define OS_GET_TICK()  osKernelGetTickCount()
#endif