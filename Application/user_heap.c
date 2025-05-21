
#include "user_heap.h"
#include "Lib\tlsf\tlsf.h"
#include "os_user_def.h"


static osSemaphoreId_t g_heap_sem;

//static char memory_pool[POOL_SIZE];

 char *g_ext_sram = ( char *)0x64100000;

tlsf_t tlsf_handle = NULL;

void asw_tlsf_init(size_t size)
{

  CREATE_BINARY_SEM(g_heap_sem);

  tlsf_handle = tlsf_create_with_pool(g_ext_sram, size);

  if (tlsf_handle == NULL)
  {
        // 초기화 실패 처리
        while (1);
  }
}


void *aws_malloc(size_t size)
{
  void *mem=0;
  PEND_SEM(g_heap_sem,osWaitForever);

  mem = (void *)tlsf_malloc(tlsf_handle, size);

  POST_SEM(g_heap_sem);
  return mem;
}


void aws_free(void *ptr)
{
  PEND_SEM(g_heap_sem, osWaitForever);
  tlsf_free(tlsf_handle, ptr);
  POST_SEM(g_heap_sem);
}