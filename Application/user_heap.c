
#include "user_heap.h"
#include "Lib\tlsf\tlsf.h"



//static char memory_pool[POOL_SIZE];

 char *g_ext_sram = ( char *)0x64100000;

tlsf_t tlsf_handle = NULL;

void asw_tlsf_init(size_t size)
{
  tlsf_handle = tlsf_create_with_pool(g_ext_sram, size);

  if (tlsf_handle == NULL)
  {
        // 초기화 실패 처리
        while (1);
  }
}


void *aws_malloc(size_t size)
{
  return (void *)tlsf_malloc(tlsf_handle,size);
}


void aws_free(void *ptr)
{
    tlsf_free(tlsf_handle,ptr);
}